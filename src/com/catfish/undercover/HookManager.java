package com.catfish.undercover;

import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.Map;

import android.util.Log;

import com.catfish.undercover.HookedMethod.HookedCallback;

/**
 * Welcome to use Hook tool coded by catfish, hope you can enjoy the java hook
 * journey. You can hook any non-native method with a HookedCallback here.
 * 
 * @author catfish
 * 
 */

public class HookManager {
    private final static String TAG = Hook.TAG;
    private final static Map<String, HookedMethod> sMethodCache = new HashMap<String, HookedMethod>();

    /**
     * setMethodHooked() can give you a chance to hook a non-native method with
     * a HookedCallback. Each time a target method is invoked, the callback is
     * called, and you can decide how to perform this method. Warning: there is
     * only one callback registered for a specific method, so you need to
     * consider the callback registered before when you are going to set a
     * callback for a method.
     * 
     * @param hookMethod
     *            The method you want to hooked
     * @param callback
     *            The callback is called when the target method is invoked
     */
    public final static void setMethodHooked(Method hookMethod, HookedCallback callback) {
        if (callback == null) {
            Log.e(TAG, "null HookedCallback is not allowed");
            return;
        }
        String key = generateKey(hookMethod);
        HookedMethod fake = sMethodCache.get(key);
        if (fake != null) {
            fake.mCallback = callback;
            return;
        }

        hookMethodNative(hookMethod);
        synchronized (sMethodCache) {
            sMethodCache.put(key, new HookedMethod(callback));
        }
    }

    public static HookedCallback getHookedCallback(Method m) {
        String key = generateKey(m);
        HookedMethod hm = sMethodCache.get(key);
        if (hm != null) {
            return hm.mCallback;
        }
        return null;
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

    private static Object handleHookedMethod(Method method, Object thisObject, Object[] args) throws Throwable {

        HookedMethod hookkedmethod = sMethodCache.get(generateKey(method));
        if (hookkedmethod != null) {
            hookkedmethod.mMethod = method;
            if (hookkedmethod.mCallback != null) {
                try {
                    return hookkedmethod.mCallback.invoke(hookkedmethod, thisObject, args);
                } catch (Exception e) {
                    Log.e(TAG, e.getMessage(), e);
                }
            }
        }

        return invokeOriginalMethod(method, thisObject, args, method.getParameterTypes(), method.getReturnType());

    }

    private native synchronized static void hookMethodNative(Method method);

    native synchronized static Object invokeOriginalMethod(Method method, Object obj, Object[] args, Object[] param,
            Object returnType);
}
