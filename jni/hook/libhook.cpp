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
		"(Ljava/lang/reflect/Method;)V",
		(void *) Hook_hookMethodNative } };

static int register_android_jni(JNIEnv *env, jclass clazz) {
	return env->RegisterNatives(clazz, gMethods, sizeof(gMethods));
}

extern jobject findSystemClassLoader(JNIEnv* env);
extern jobject createDexClassLoader(JNIEnv* env, const char* dexPath, const char* dexOptDir,
        const char* libPath, jobject parent);
extern jobject findPathClassLoader(JNIEnv* env, const char *pkgName);
extern jclass loadTargetClass(JNIEnv* env, jobject dexClassLoaderObject, const char* className);

static jclass sTargetClass = 0;
int invoke_dex_method(const char* dexPath, const char* className, const char* methodName,
		const char* proxyName, const char *pkgName, int argc, char *argv[]) {
	LOGD("Invoke dex E");
	JNIEnv * env = android::AndroidRuntime::getJNIEnv();

	jclass targetClass;

	if (sTargetClass == 0) {
		jobject systemClassLoaderObject = findPathClassLoader(env, pkgName);
		jobject dexClassLoaderObject = createDexClassLoader(env, dexPath, NULL, NULL,
				systemClassLoaderObject);
		targetClass = loadTargetClass(env, dexClassLoaderObject, className);
		jclass proxyClass = loadTargetClass(env, dexClassLoaderObject, proxyName);

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
		sTargetClass = (jclass)env->NewGlobalRef(targetClass);
		env->DeleteLocalRef(targetClass);
	} else {
		LOGD("target class %s --> %p, already loaded", className, sTargetClass);
	}
	/* Invoke target method*/
	jmethodID targetMethod = env->GetStaticMethodID(sTargetClass, methodName,
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
	env->CallStaticVoidMethod(sTargetClass, targetMethod, stringArray);
	env->DeleteLocalRef(stringClass);
	env->DeleteLocalRef(stringArray);
	LOGD("Invoke dex X");
	return 0;
}

int hook(char *argv) {
	char *pkgName = argv;
	while (*argv !='#')argv++;
	*argv = 0;
	argv++;
	LOGD("loading dex begin %s", argv);
	invoke_dex_method(argv, "com.catfish.undercover.Hook", "main",
			"com.catfish.undercover.HookManager", pkgName, 1, &argv);
	LOGD("loading dex end");
	return -1;
}
}
