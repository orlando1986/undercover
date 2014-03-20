package com.catfish.undercover;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.Map;

import android.util.Log;

/**
 * Welcome to use Hook tool coded by catfish, hope you can enjoy the java hook
 * journey. You can use any supported methods here in this class to implement a
 * lightweight hook function. We support all methods replacement except those
 * with the modifier of private, constructor, native and static.
 * 
 * @author catfish
 * 
 */

public class HookManager {
    public final static String TAG = "catfish";
    private final static boolean DEBUG = true;
    private final static Map<String, HookedCallback> sMethodCache = new HashMap<String, HookedCallback>();

    public final static void setMethodHooked(Method hookMethod, HookedCallback callback, boolean force) {
        String key = generateKey(hookMethod);
        if (sMethodCache.containsKey(key)) {
            sMethodCache.put(key, callback);
            return;
        }
        if (Modifier.isNative(hookMethod.getModifiers()) && !force) {
            Log.e(TAG, "this is a native method, we did not force to hook it");
            return;
        }

        Class<?> declaringClass = hookMethod.getDeclaringClass();
        int slot = (int) getSlotField(hookMethod);

        Class<?>[] parameterTypes = ((Method) hookMethod).getParameterTypes();
        Class<?> returnType = ((Method) hookMethod).getReturnType();

        AdditionalHookInfo additionalInfo = new AdditionalHookInfo(parameterTypes, returnType);

        hookMethodNative(hookMethod, declaringClass, slot, additionalInfo);
        synchronized (sMethodCache) {
            sMethodCache.put(key, callback);
        }
    }

    private final static String generateKey(Method m) {
        StringBuffer sb = new StringBuffer();
        sb.append(m.getDeclaringClass().getName());
        sb.append(":");
        sb.append(m.getName());
        sb.append(":");
        Type[] params = m.getGenericParameterTypes();
        for (Type t : params) {
            sb.append(t.getClass().getName());
            sb.append(":");
        }
        sb.append(m.getGenericReturnType().getClass().getName());
        return sb.toString();
    }

    public static HookedCallback getHookedCallback(Method m) {
        String key = generateKey(m);
        synchronized (sMethodCache) {
            return sMethodCache.get(key);
        }
    }

    private static int getSlotField(Method m) {
        try {
            Class<?> clazz = m.getClass();
            Field f = clazz.getDeclaredField("slot");
            f.setAccessible(true);
            return f.getInt(m);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError(e.getMessage());
        } catch (Exception e1) {
            Log.e(TAG, e1.getMessage(), e1);
            return -1;
        }
    }

    static void LOGD(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    @SuppressWarnings("unused")
    private static class AdditionalHookInfo {
        private Class<?>[] parameterTypes;
        private Class<?> returnType;

        private AdditionalHookInfo(Class<?>[] parameterTypes, Class<?> returnType) {
            this.parameterTypes = parameterTypes;
            this.returnType = returnType;
        }
    }

    public interface HookedCallback {

        public Object preCalled();

        public boolean needInvokeOriginal();

        public Object postCalled();

    }

    private static Object handleHookedMethod(Method method, Object thisObject, Object[] args)
            throws Throwable {
        Log.e(TAG, "version4.0 success! --- " + method.getName());
        invokeOriginalMethod(method, thisObject, args, method.getParameterTypes(), method.getReturnType());
//        method.invoke(thisObject, args);
        /*
         * AdditionalHookInfo additionalInfo = (AdditionalHookInfo)
         * additionalInfoObj;
         * 
         * synchronized (sMethodCache) { HookedCallback callback =
         * sMethodCache.get(generateKey(method)); if (callback == null) { return
         * invokeOriginalMethodNative(method, originalMethodId,
         * additionalInfo.parameterTypes, additionalInfo.returnType, thisObject,
         * args); } }
         */
        return null;

    }

    private native synchronized static void hookMethodNative(Method method, Class<?> declaringClass, int slot, Object additionalInfo);
    private native synchronized static Object invokeOriginalMethod(Method method, Object obj, Object[] args, Object[] param, Object returnType);
}
