package com.catfish.undercover;

import java.lang.reflect.Method;

import android.util.Log;

import com.catfish.undercover.HookedMethod.HookedCallback;

public class Undercover {
    public final static String TAG = "catfish";

    public void onInject() {
        Log.e(TAG, "hook starts");
        Method hookMethod = null;
        try {
            Class actyclass = Class.forName("android.app.Activity");
            hookMethod = actyclass.getDeclaredMethod("onResume", (Class[]) null);
        } catch (Exception e) {
            e.printStackTrace();
        }
        HookManager.setMethodHooked(hookMethod, new HookedCallback() {

            @Override
            public Object invoke(HookedMethod method, Object receiver, Object[] args) {
                Log.d(TAG, "hooked: " + method.getName());
                return method.invoke(receiver, args);
            }
        });
    }

}