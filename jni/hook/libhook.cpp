/*
 * libmynet.c
 *
 *  Created on: 2013-1-17
 *      Author: d
 */

#include <jni.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include "AndroidRuntime.h"
#include "../inject/utils.h"
#include "Dalvik.h"
#include "Proxy.h"

extern "C" {

static void Hook_hookMethodNative(JNIEnv* env, jclass clazz,
		jobject reflectedMethodIndirect) {

	hookMethod(env, clazz, reflectedMethodIndirect);
}

const JNINativeMethod gMethods[] = { { "hookMethodNative",
		"(Ljava/lang/reflect/Method;)V", (void *) Hook_hookMethodNative } };

static int register_android_jni(JNIEnv *env, jclass clazz) {
	return env->RegisterNatives(clazz, gMethods, sizeof(gMethods));
}

extern jobject findSystemClassLoader(JNIEnv* env);
extern jobject createDexClassLoader(JNIEnv* env, const char* dexPath,
		const char* dexOptDir, const char* libPath, jobject parent);
extern jobject findPathClassLoader(JNIEnv* env, const char *pkgName);
extern jclass loadTargetClass(JNIEnv* env, jobject dexClassLoaderObject,
		const char* className);

static jclass sTargetClass = 0;
#define SIZE 0x100
static const char* PATHS[SIZE];
static jclass CLASS[SIZE];
static int sSize = 0;

int invoke_dex_method(const char* dexPath, const char* className,
		const char* methodName, const char* proxyName, const char *pkgName,
		int argc, char *argv[]) {
	LOGD("Invoke dex E");
	JNIEnv * env = android::AndroidRuntime::getJNIEnv();

	jclass targetClass;

	if (sSize == 0) {
		memset(PATHS, 0, sizeof(PATHS));
		memset(CLASS, 0, sizeof(CLASS));
	}

	int i = 0;
	for (i = 0; i < sSize; i++) {
		if (PATHS[i] && !strcmp(dexPath, PATHS[i])) {
			break;
		}
	}

	if (i == sSize || CLASS[i] == 0) {
		jobject systemClassLoaderObject = 0;
		if (strcmp(pkgName, "system_server") == 0) {
			systemClassLoaderObject = findSystemClassLoader(env);
		} else {
			systemClassLoaderObject = findPathClassLoader(env, pkgName);
		}
		jobject dexClassLoaderObject = createDexClassLoader(env, dexPath, NULL,
				NULL, systemClassLoaderObject);
		targetClass = loadTargetClass(env, dexClassLoaderObject, className);
		jclass proxyClass = loadTargetClass(env, dexClassLoaderObject,
				proxyName);

		if (!targetClass) {
			LOGE("Failed to load target class %s", className);
			return -1;
		} else {
			LOGD("target class %s --> %p", className, targetClass);
		}

		if (!proxyClass) {
			LOGE("Failed to load proxyClass class %s", proxyName);
			return -1;
		}

		register_android_jni(env, proxyClass);
		initMembers(env, proxyClass);
		env->DeleteLocalRef(proxyClass);
		env->DeleteLocalRef(dexClassLoaderObject);
		env->DeleteLocalRef(systemClassLoaderObject);
		CLASS[i] = (jclass) env->NewGlobalRef(targetClass);
		PATHS[i] = dexPath;
		env->DeleteLocalRef(targetClass);
		sSize++;
	} else {
		LOGD("target class %s --> %p, already loaded, i=%d",
				className, CLASS[i], i);
	}
	/* Invoke target method*/
	jmethodID targetMethod = env->GetStaticMethodID(CLASS[i], methodName,
			"([Ljava/lang/String;)V");
	if (!targetMethod) {
		LOGE("Failed to load target method %s", methodName);
		return -1;
	}

	jclass stringClass = (jclass) env->FindClass("java/lang/String");
	jobjectArray stringArray = env->NewObjectArray(argc, stringClass, NULL);
	LOGD("stringArray %p", stringArray);
	for (int i = 0; i < argc; i++) {
		jstring tmpString = env->NewStringUTF(argv[i]);
		env->SetObjectArrayElement(stringArray, i, tmpString);
		env->DeleteLocalRef(tmpString);
	}
	env->CallStaticVoidMethod(CLASS[i], targetMethod, stringArray);
	env->DeleteLocalRef(stringClass);
	env->DeleteLocalRef(stringArray);
	LOGD("Invoke dex X");
	return 0;
}

int hook(char *argv) {
	char *pkgName = argv;
	while (*argv != '#')
		argv++;
	*argv = 0;
	argv++;
	LOGD("loading dex begin %s", argv);
	invoke_dex_method(argv, "com.catfish.undercover.Hook", "main",
			"com.catfish.undercover.HookManager", pkgName, 1, &argv);
	LOGD("loading dex end");
	return -1;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = -1;

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		return -1;
	}
	assert(env != NULL);

	jclass proxyClass = env->FindClass("com.catfish.undercover.HookManager");

	register_android_jni(env, proxyClass);
	initMembers(env, proxyClass);

	result = JNI_VERSION_1_4;

	return result;
}
}
