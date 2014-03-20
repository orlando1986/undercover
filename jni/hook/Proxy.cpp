#include "Dalvik.h"
#include <jni.h>

#include <stdlib.h>
#include "Proxy.h"

void* PTR_gDvmJit = NULL;
Method* xposedHandleHookedMethod = NULL;
Method* invokeOriginalMethodNative = NULL;
ClassObject* objectArrayClass = NULL;

static ArrayObject* boxMethodArgs(const Method* method, const u4* args) {
	const char* desc = &method->shorty[1]; // [0] is the return type.

	/* count args */
	size_t argCount = dexProtoGetParameterCount(&method->prototype);

	/* allocate storage */
	ArrayObject* argArray = dvmAllocArrayByClass(objectArrayClass, argCount,
			ALLOC_DEFAULT);
	if (argArray == NULL)
		return NULL;
	Object** argObjects = (Object**) (void*) argArray->contents;

	/*
	 * Fill in the array.
	 */

	size_t srcIndex = 0;
	size_t dstIndex = 0;
	while (*desc != '\0') {
		char descChar = *(desc++);
		JValue value;

		switch (descChar) {
		case 'Z':
		case 'C':
		case 'F':
		case 'B':
		case 'S':
		case 'I':
			value.i = args[srcIndex++];
			argObjects[dstIndex] = (Object*) dvmBoxPrimitive(value,
					dvmFindPrimitiveClass(descChar));
			/* argObjects is tracked, don't need to hold this too */
			dvmReleaseTrackedAlloc(argObjects[dstIndex], NULL);
			dstIndex++;
			break;
		case 'D':
		case 'J':
			value.j = dvmGetArgLong(args, srcIndex);
			srcIndex += 2;
			argObjects[dstIndex] = (Object*) dvmBoxPrimitive(value,
					dvmFindPrimitiveClass(descChar));
			dvmReleaseTrackedAlloc(argObjects[dstIndex], NULL);
			dstIndex++;
			break;
		case '[':
		case 'L':
			argObjects[dstIndex++] = (Object*) args[srcIndex++];
			break;
		}
	}

	return argArray;
}

void realInvokeOriginalMethodNative(const u4* args, JValue* pResult,
		const Method* method, ::Thread* self) {
    Method* meth = (Method*) args[1];
    if (meth == NULL) {
        meth = dvmGetMethodFromReflectObj((Object*) args[0]);
    }
    Object* thisObject = (Object*) args[2]; // null for static methods
    ArrayObject* argList = (ArrayObject*) args[3];
    ArrayObject* params = (ArrayObject*) args[4];
    ClassObject* returnType = (ClassObject*) args[5];

    Method* hookInfo = (Method*) calloc(1, sizeof(Method));
    memcpy(hookInfo, meth, sizeof(Method));
	CLEAR_METHOD_FLAG(hookInfo, ACC_NATIVE);
    // invoke the method
    pResult->l = dvmInvokeMethod(thisObject, hookInfo, argList, params, returnType, true);
}

static void xposedCallHandler(const u4* args, JValue* pResult,
		const Method* method, ::Thread* self) {

//	Method* originalReflected = (Method*) method->insns;
//	CLEAR_METHOD_FLAG(originalReflected, ACC_NATIVE);

	// convert/box arguments
	Object* thisObject = NULL;
	int argsindex = 0;

	// for non-static methods determine the "this" pointer
	if (!dvmIsStaticMethod(method)) {
		thisObject = (Object*) args[0];
		argsindex++;
	}

	ArrayObject* argsArray = boxMethodArgs(method, args + argsindex);
	Object* methodObj = dvmCreateReflectMethodObject(method);

	// call the Java handler function
	JValue result;
	dvmCallMethod(self, xposedHandleHookedMethod, NULL, &result, methodObj,
			thisObject, argsArray);

	dvmReleaseTrackedAlloc(argsArray, self);

	// exceptions are thrown to the caller
	if (dvmCheckException(self)) {
		return;
	}

	// return result with proper type
	ClassObject* returnType = dvmGetBoxedReturnType(method);
	if (returnType->primitiveType == PRIM_VOID) {
		// ignored
	} else if (result.l == NULL) {
		if (dvmIsPrimitiveClass(returnType)) {
			dvmThrowNullPointerException("null result when primitive expected");
		}
		pResult->l = NULL;
	} else {
		if (!dvmUnboxPrimitive(result.l, returnType, pResult)) {
			dvmThrowClassCastException(result.l->clazz, returnType);
		}
	}
}

void initMembers(JNIEnv* env, jclass xposedClass) {
	objectArrayClass = dvmFindArrayClass("[Ljava/lang/Object;", NULL);
	xposedHandleHookedMethod =
			(Method*) env->GetStaticMethodID(xposedClass, "handleHookedMethod",
					"(Ljava/lang/reflect/Method;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
	invokeOriginalMethodNative =
			(Method*) env->GetStaticMethodID(xposedClass, "invokeOriginalMethod",
					"(Ljava/lang/reflect/Method;Ljava/lang/Object;[Ljava/lang/Object;[Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	dvmSetNativeFunc(invokeOriginalMethodNative, realInvokeOriginalMethodNative, NULL);
}

void hookMethod(JNIEnv* env, jclass clazz, jobject reflectedMethodIndirect,
		jobject declaredClassIndirect, jint slot,
		jobject additionalInfoIndirect) {
	// Usage errors?
	if (declaredClassIndirect == NULL || reflectedMethodIndirect == NULL) {
		dvmThrowIllegalArgumentException(
				"method and declaredClass must not be null");
		return;
	}

	// Find the internal representation of the method
	Method* method = (Method*) env->FromReflectedMethod(
			reflectedMethodIndirect);
	if (method == NULL) {
		dvmThrowNoSuchMethodError(
				"could not get internal representation for method");
		return;
	}
	// Save a copy of the original method and other hook info
//	Method* hookInfo = (Method*) calloc(1, sizeof(Method));
//	memcpy(hookInfo, method, sizeof(Method));
	// Replace method with our own code
	method->nativeFunc = xposedCallHandler;
//	method->insns = (const u2*) hookInfo;
//	method->registersSize = method->insSize;
	int argsSize = dvmComputeMethodArgsSize(method) + 1;
	method->registersSize = method->insSize = argsSize;
//	method->outsSize = 0;
	SET_METHOD_FLAG(method, ACC_NATIVE);
}