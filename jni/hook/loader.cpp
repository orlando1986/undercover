#include <stdio.h>
#include <string.h>
#include <jni.h>

#define LOG_TAG "LOADER"

#ifdef ANDROID_NDK
#include <android/log.h>
#include <sys/system_properties.h>
#define LOGV(fmt, args...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, fmt, ##args)
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args)
#define LOGW(fmt, args...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, fmt, ##args)
#else
#include <cutils/log.h>
#include <cutils/properties.h>
#endif

static int DEBUG_LOG_ENABLED = 0;
#define DEBUG_LOG_ENABLED_KEY "persist.loader.log.debug"
#define LOADER_LOGD(format,args...) do { if (DEBUG_LOG_ENABLED) printf(format "\n", ##args); LOGD(format, ##args); } while(0)
#define LOADER_LOGE(format,args...) do { printf(format "\n", ##args); LOGE(format, ##args); } while(0)

#define PRINT_VAR(a) LOADER_LOGD("    "#a"=%x", (unsigned int)a)

#ifdef ANDROID_NDK
#include <android/log.h>
namespace android {
    class AndroidRuntime {public: static JNIEnv* getJNIEnv(); };
}
extern "C" void jniLogException(JNIEnv* env, int priority, const char* tag, jthrowable exception);
#else
#include <android_runtime/AndroidRuntime.h>
#include <JNIHelp.h>
#endif

static const char* const CLASS_NAME_CLASSLOADERPROXY = "android.os.ClassLoaderProxy";

static int catchException(JNIEnv* env);
static jobject getClassLoader(JNIEnv* env, jclass cls);
static jclass getClass(JNIEnv* env, jobject obj);
static jobject getClassLoader(JNIEnv* env, jclass cls);

static jobject getClassLoaderByApplication(JNIEnv* env, jobject obj) {
	jclass cls = env->FindClass("android/app/Application");
    jmethodID getClassLoaderMethodID = env->GetMethodID(cls, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject classLoaderObject = env->CallObjectMethod(obj, getClassLoaderMethodID);
    env->DeleteLocalRef(cls);
    return classLoaderObject;
}

extern "C"
jobject findPathClassLoader(JNIEnv* env, const char *pkgName) {
    LOADER_LOGD("findPathClassLoader(%p, %s) E", env, pkgName);
    jclass atClass = env->FindClass("android/app/ActivityThread");
    PRINT_VAR(atClass);

    jobject atObject = 0;
    jmethodID catMethod = env->GetStaticMethodID(atClass, "currentActivityThread", "()Landroid/app/ActivityThread;");
    PRINT_VAR(catMethod);
    atObject = env->CallStaticObjectMethod(atClass, catMethod);
    PRINT_VAR(atObject);

    if (!atObject) {
        LOADER_LOGE("BAD!!! Cannot to get ActivityThread. Suppose you should not see me.");
    }

    jobject mClassLoaderObject = 0;
    jmethodID getApplicationMethod = env->GetMethodID(atClass, "getApplication",
                    "()Landroid/app/Application;");
    PRINT_VAR(getApplicationMethod);
    jobject appObject = env->CallObjectMethod(atObject, getApplicationMethod);
    PRINT_VAR(appObject);
    mClassLoaderObject = getClassLoaderByApplication(env, appObject);
    env->DeleteLocalRef(appObject);

    PRINT_VAR(mClassLoaderObject);
    LOADER_LOGD("findPathClassLoader(%p, %s) X, %p", env, pkgName, mClassLoaderObject);
    env->DeleteLocalRef(atClass);
    env->DeleteLocalRef(atObject);
    return mClassLoaderObject;
}

extern "C"
jobject findSystemClassLoader(JNIEnv* env) {
    LOADER_LOGD("findSystemClassLoader(%p) E", env);
    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
    PRINT_VAR(classLoaderClass);
    jmethodID getSystemClassLoaderMethodID = env->GetStaticMethodID(classLoaderClass,
            "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    PRINT_VAR(getSystemClassLoaderMethodID);
    jobject systemClassLoaderObject = env->CallStaticObjectMethod(classLoaderClass,
            getSystemClassLoaderMethodID);
    PRINT_VAR(systemClassLoaderObject);
    LOADER_LOGD("findSystemClassLoader(%p) X, %p", env, systemClassLoaderObject);
    env->DeleteLocalRef(classLoaderClass);
    return systemClassLoaderObject;
}

extern "C"
jobject createDexClassLoader(JNIEnv* env, const char* dexPath, const char* dexOptDir,
        const char* libPath, jobject parent) {
    LOADER_LOGD("createDexClassLoader(%p, %s, %s, %s, %p) E", env, dexPath, dexOptDir, libPath, parent);
    int usePathClassLoader = (dexOptDir == NULL || strcmp("", dexOptDir) == 0 || strcmp(".", dexOptDir) == 0);
    jclass dexClassLoaderClass = 0;
    if (usePathClassLoader) {
        dexClassLoaderClass = env->FindClass("dalvik/system/PathClassLoader");
    } else {
        dexClassLoaderClass = env->FindClass("dalvik/system/DexClassLoader");
    }
    PRINT_VAR(dexClassLoaderClass);
    jmethodID dexClassLoaderContructorID = 0;
    if (usePathClassLoader) {
        dexClassLoaderContructorID = env->GetMethodID(dexClassLoaderClass, "<init>",
                "(Ljava/lang/String;Ljava/lang/ClassLoader;)V");
    } else {
        dexClassLoaderContructorID = env->GetMethodID(dexClassLoaderClass, "<init>",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
    }
    PRINT_VAR(dexClassLoaderContructorID);
    jstring dexPathString = env->NewStringUTF(dexPath);
    PRINT_VAR(dexPathString);
    jstring dexOptDirString = env->NewStringUTF((dexOptDir != NULL && strlen(dexOptDir) == 0) ? NULL : dexOptDir);
    PRINT_VAR(dexOptDirString);
    jstring libPathString = env->NewStringUTF(libPath);
    PRINT_VAR(libPathString);
    jobject dexClassLoaderObject = 0;
    if (usePathClassLoader) {
        dexClassLoaderObject = env->NewObject(dexClassLoaderClass, dexClassLoaderContructorID,
                dexPathString, parent);
    } else {
        dexClassLoaderObject = env->NewObject(dexClassLoaderClass, dexClassLoaderContructorID,
                dexPathString, dexOptDirString, libPathString, parent);
    }

    PRINT_VAR(dexClassLoaderObject);
    LOADER_LOGD("createDexClassLoader(%p, %s, %s, %s, %p) X, %p", env, dexPath, dexOptDir,
            libPath, parent, dexClassLoaderObject);
    env->DeleteLocalRef(dexClassLoaderClass);
    env->DeleteLocalRef(dexPathString);
    env->DeleteLocalRef(dexOptDirString);
    env->DeleteLocalRef(libPathString);
    return dexClassLoaderObject;
}

extern "C"
jclass loadTargetClass(JNIEnv* env, jobject dexClassLoaderObject, const char* className) {
    LOADER_LOGD("loadTargetClass(%p, %p, %s) E", env, dexClassLoaderObject, className);
    jclass dexClassLoaderClass = env->GetObjectClass(dexClassLoaderObject);
    PRINT_VAR(dexClassLoaderClass);
    jmethodID loadClassMethodID = env->GetMethodID(dexClassLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    PRINT_VAR(loadClassMethodID);
    jstring classNameString = env->NewStringUTF(className);
    PRINT_VAR(classNameString);
    jclass targetClass = (jclass)env->CallObjectMethod(dexClassLoaderObject, loadClassMethodID, classNameString);
    PRINT_VAR(targetClass);
    LOADER_LOGD("loadTargetClass(%p, %p, %s) X, %p", env, dexClassLoaderObject, className, targetClass);
    env->DeleteLocalRef(dexClassLoaderClass);
    env->DeleteLocalRef(classNameString);
    return targetClass;
}

jclass findClass(JNIEnv* env, const char* className) {
    LOADER_LOGD("findClass(%p, %s) E", env, className);
    char *slashName = (char *)malloc(strlen(className) + 1);
    strcpy(slashName, className);
    int len  = strlen(className);
    int i = 0;
    for (i = 0; i < len; i++) {
        if (slashName[i] == '.') {
            slashName[i] = '/';
        }
    }
    jclass ret = env->FindClass(slashName);
    free(slashName);
    LOADER_LOGD("findClass(%p, %s) X, %p", env, className, ret);
    return ret;
}

static int catchException(JNIEnv* env) {
    jthrowable ex = env->ExceptionOccurred();
    if (ex) {
        jniLogException(env, ANDROID_LOG_ERROR, LOG_TAG, ex);
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(ex);
    }
    return ex == 0;
}

// ClassLoader.getParent()
static jobject getParent(JNIEnv* env, jobject classLoaderObject) {
    jclass classLoaderClass = env->GetObjectClass(classLoaderObject);
    jmethodID getParentMethodID = env->GetMethodID(classLoaderClass, "getParent", "()Ljava/lang/ClassLoader;");
    jobject parentClassLoaderObject = env->CallObjectMethod(classLoaderObject, getParentMethodID);
    LOADER_LOGD("getParent(%p, %p) --> %p", env, classLoaderObject, parentClassLoaderObject);
    env->DeleteLocalRef(classLoaderClass);
    return parentClassLoaderObject;
}

// Object.getClass()
static jclass getClass(JNIEnv* env, jobject obj) {
    jclass cls = env->GetObjectClass(obj);
    LOADER_LOGD("getClass(%p, %p) --> %p", env, obj , cls);
    return cls;
}

// Class.getName()
static jstring getName(JNIEnv* env, jclass classObject) {
    jclass classClass = env->FindClass("java/lang/Class");
    jmethodID getNameMethodID = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
    jstring className = (jstring)env->CallObjectMethod(classObject, getNameMethodID);
    LOADER_LOGD("getName(%p, %p) --> %p", env, classObject, className);
    env->DeleteLocalRef(classClass);
    return className;
}

// Class.getClassLoader()
static jobject getClassLoader(JNIEnv* env, jclass cls) {
    jclass classClass = env->FindClass("java/lang/Class");
    jmethodID getClassLoaderMethodID = env->GetMethodID(classClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject classLoaderObject = env->CallObjectMethod(cls, getClassLoaderMethodID);
    LOADER_LOGD("getClassLoader(%p, %p) --> %p", env, cls, classLoaderObject);
    env->DeleteLocalRef(classClass);
    return classLoaderObject;
}

static int changeClassLoaderParent(JNIEnv* env, jobject classLoaderObject, jobject newParent) {
    jclass classLoaderClass = getClass(env, classLoaderObject);
    jfieldID parentFieldID = env->GetFieldID(classLoaderClass, "parent", "Ljava/lang/ClassLoader;");
    env->SetObjectField(classLoaderObject, parentFieldID, newParent);
    env->DeleteLocalRef(classLoaderClass);
    LOADER_LOGD("succeeded to change ClassLoader's parent");
    return 0;
}

// return 0 success, otherwise failure.
static int insertClassLoaderProxy(JNIEnv* env, jobject dexClassLoaderObject, jobject classLoaderObject) {
    jclass classLoaderProxyClass = loadTargetClass(env, dexClassLoaderObject, CLASS_NAME_CLASSLOADERPROXY);
    if (catchException(env) && !classLoaderProxyClass) {
        LOADER_LOGE("failed to load %s, is it in your dex/jar?", CLASS_NAME_CLASSLOADERPROXY);
        return -1;
    }
    PRINT_VAR(classLoaderProxyClass);
    jmethodID classLoaderProxyContructorID = env->GetMethodID(classLoaderProxyClass, "<init>", "(Ljava/lang/ClassLoader;)V");
    PRINT_VAR(classLoaderProxyContructorID);
    if (catchException(env) && !classLoaderProxyContructorID) {
        LOADER_LOGE("failed to find constructor of %s", CLASS_NAME_CLASSLOADERPROXY);
        env->DeleteLocalRef(classLoaderProxyClass);
        return -1;
    }
    jobject parentClassLoaderObject = getParent(env, classLoaderObject);
    jobject classLoaderProxyObject = env->NewObject(classLoaderProxyClass, classLoaderProxyContructorID, parentClassLoaderObject);
    env->DeleteLocalRef(parentClassLoaderObject);
    if (catchException(env) && !classLoaderProxyObject) {
        LOADER_LOGE("failed to create %s object", CLASS_NAME_CLASSLOADERPROXY);
        env->DeleteLocalRef(classLoaderProxyClass);
        return -1;
    }
    if (changeClassLoaderParent(env, classLoaderObject, classLoaderProxyObject)) {
        LOADER_LOGE("failed to change ClassLoader %p 's parent with %p", classLoaderObject, classLoaderProxyObject);
        env->DeleteLocalRef(classLoaderProxyClass);
        return -1;
    }
    env->DeleteLocalRef(classLoaderProxyClass);
    return 0;
}

static int invoke(const char* dexPath, const char* dexOptDir, const char* pkgName, const char* className, const char* methodName, int argc, char *argv[]) {
    LOADER_LOGD("invoke(%s, %s, %s, %s, %s)", dexPath, dexOptDir, pkgName, className, methodName);
    JNIEnv* env = android::AndroidRuntime::getJNIEnv();

    // find PathClassLoader or SystemClassLoader
    jobject parentClassLoaderObject = 0;
    if (strcmp(pkgName, "android") == 0) {
         parentClassLoaderObject = findSystemClassLoader(env);
    } else {
         parentClassLoaderObject = findPathClassLoader(env, pkgName);
    }
    if (!parentClassLoaderObject) {
        LOADER_LOGE("failed to find parent ClassLoader");
        return -1;
    }

    // get PathClassLoader or SystemClassLoader's parent
    jobject parentParentClassLoaderObject = getParent(env, parentClassLoaderObject);
    if (!parentParentClassLoaderObject) {
        LOADER_LOGE("failed to find parent's parent ClassLoader");
        env->DeleteLocalRef(parentClassLoaderObject);
        return -1;
    }

    // get PathClassLoader or SystemClassLoader's parent's class name
    jclass parentParentClassLoaderClass = getClass(env, parentParentClassLoaderObject);
    jstring parentParentClassName = getName(env, parentParentClassLoaderClass);
    const char *tmpName = env->GetStringUTFChars(parentParentClassName, 0);
    LOADER_LOGD("Path/System ClassLoader's parent name is %s", tmpName);

    // judge whether ClassLoaderProxy has been inserted into ClassLoader chain.
    int loaded = (strcmp(CLASS_NAME_CLASSLOADERPROXY, tmpName) == 0);
    jobject dexClassLoaderObject = 0;
    if (loaded) {
        dexClassLoaderObject = getClassLoader(env, parentParentClassLoaderClass);
    } else {
        dexClassLoaderObject = createDexClassLoader(env, dexPath, dexOptDir, NULL, parentClassLoaderObject);
        if (dexClassLoaderObject) {
            // insert ClassLoaderProxy into ClassLoader chain.
            if (insertClassLoaderProxy(env, dexClassLoaderObject, parentClassLoaderObject)) {
                LOADER_LOGE("failed to insert ClassLoaderProxy");
                env->DeleteLocalRef(parentClassLoaderObject);
                env->DeleteLocalRef(parentParentClassLoaderObject);
                env->DeleteLocalRef(parentParentClassLoaderClass);
                env->DeleteLocalRef(parentParentClassName);
                return -1;
            } else {
                LOADER_LOGD("succeeded to insert ClassLoaderProxy");
            }
        }
    }
    env->DeleteLocalRef(parentClassLoaderObject);
    env->DeleteLocalRef(parentParentClassLoaderObject);
    env->DeleteLocalRef(parentParentClassLoaderClass);
    env->DeleteLocalRef(parentParentClassName);
    if (dexClassLoaderObject) {
        LOADER_LOGD("find DexClassLoader with loaded = %d", loaded);
    } else {
        LOADER_LOGE("failed to find DexClassLoader, loaded = %d", loaded);
        return -1;
    }

    jclass targetClass = loadTargetClass(env, dexClassLoaderObject, className);
    if (!targetClass) {
        LOADER_LOGE("failed to load target class %s", className);
        env->DeleteLocalRef(dexClassLoaderObject);
        return -1;
    }
    env->DeleteLocalRef(dexClassLoaderObject);

    /* Invoke target method */
    jmethodID targetMethod = env->GetStaticMethodID(targetClass, methodName, "([Ljava/lang/String;)I");
    if (!targetMethod) {
        LOADER_LOGE("failed to find target method %s([Ljava/lang/String;)I", methodName);
        env->DeleteLocalRef(targetClass);
        return -1;
    }

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray stringArray = env->NewObjectArray(argc, stringClass, NULL);
    for (int i = 0; i < argc; i++) {
        jstring tmpString = env->NewStringUTF(argv[i]);
        env->SetObjectArrayElement(stringArray, i, tmpString);
        env->DeleteLocalRef(tmpString);
    }
    jint result = env->CallStaticIntMethod(targetClass, targetMethod, stringArray);
    LOADER_LOGE("invoke %s.%s() returns %d", className, methodName, result);
    env->DeleteLocalRef(targetClass);
    env->DeleteLocalRef(stringArray);
    env->DeleteLocalRef(stringClass);
    return result;
}

extern "C"
int dlmain(int argc, char* argv[]) {
    if (argc < 5) {
        LOADER_LOGE("invalid parameters for int dlmain(int argc, char *argv[])");
        LOADER_LOGE("argv[0], dex/jar/apk path");
        LOADER_LOGE("argv[1], dex opt dir, NULL or \"\" for PathClassLoader, otherwise DexClassLoader");
        LOADER_LOGE("argv[2], package name, used to find PathClassLoader, \"android\" means to find SystemClassLoader");
        LOADER_LOGE("argv[3], target class name, such as com.android.inject.Main");
        LOADER_LOGE("argv[4], target method name, it's prototype must be ([Ljava/lang/String;)I");
        return -1;
    }
#ifdef ANDROID_NDK
    static char value[PROP_VALUE_MAX];
    memset(value, 0, sizeof (value));
    __system_property_get(DEBUG_LOG_ENABLED_KEY, value);
#else
    static char value[PROPERTY_VALUE_MAX];
    memset(value, 0, sizeof (value));
    property_get(DEBUG_LOG_ENABLED_KEY, value, "0");
#endif

    char *dexPath = *argv++;
    LOADER_LOGD("dexPath=%s", dexPath);
    char *dexOptDir = *argv++;
    LOADER_LOGD("dexOptDir=%s", dexOptDir);
    char *pkgName = *argv++;
    LOADER_LOGD("pkgName=%s", pkgName);
    char *className = *argv++;
    LOADER_LOGD("className=%s", className);
    char *methodName = *argv++;
    LOADER_LOGD("methodName=%s", methodName);
    argc -= 5;
    if (argc == 0) {
        argv = NULL;
    }
    int i = 0;
    for (i = 0; i < argc; i++) {
        LOADER_LOGD("argv[%d]=%s", i, argv[i]);
    }
    return invoke(dexPath, dexOptDir, pkgName, className, methodName, argc, argv);
}
