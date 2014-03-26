package com.catfish.undercover;

import java.lang.reflect.Method;

import android.content.Context;
import android.util.Log;

import com.catfish.undercover.HookedMethod.HookedCallback;

public class Undercover {
    public final static String TAG = "catfish";

    public void onInject(Context application) {
        Log.e(TAG, "hook starts");
        Method[] hookMethod = null;
        try {
            Class actyclass = Class.forName("com.tencent.mm.ui.contact.profile.ContactInfoUI");
            hookMethod = actyclass.getDeclaredMethods();
        } catch (Exception e) {
            e.printStackTrace();
        }
        HookedCallback h = new HookedCallback() {

            @Override
            public Object invoke(HookedMethod method, Object receiver, Object[] args) {
                for (Object obj : args) {
                    Log.d(TAG, "hooked: " + method.getName() + ", " + obj.getClass());
                }
                new Exception("got " + method.getName()).printStackTrace();
                return method.invoke(receiver, args);
            }
        };
        for (Method m : hookMethod) {
            HookManager.setMethodHooked(m, h);;
        }
    }
}