#include "Dalvik.h"
#include <jni.h>
#include "ScopedPthreadMutexLock.h"
#include "Globals.h"
#define offset_type_DvmJitGlobals_codeCacheFull bool

#define MEMBER_OFFSET_ARRAY(type,member) offsets_array_ ## type ## _ ## member
#define MEMBER_OFFSET_VAR(type,member) offset_ ## type ## _ ## member
#define MEMBER_TYPE(type,member) offset_type_ ## type ## _ ## member
#define MEMBER_PTR(obj,type,member) \
    ( (MEMBER_TYPE(type,member)*)  ( (char*)(obj) + MEMBER_OFFSET_VAR(type,member) ) )
#define MEMBER_VAL(obj,type,member) *MEMBER_PTR(obj,type,member)

struct XposedHookInfo {
	struct {
		Method originalMethod;
		// copy a few bytes more than defined for Method in AOSP
		// to accomodate for (rare) extensions by the target ROM
		int dummyForRomExtensions[4];
	} originalMethodStruct;

	Object* reflectedMethod;
	Object* additionalInfo;
};

void hookMethod(JNIEnv* env, jclass clazz, jobject reflectedMethodIndirect,
		jobject declaredClassIndirect, jint slot,
		jobject additionalInfoIndirect);
void initMembers(JNIEnv* env, jclass xposedClass);
