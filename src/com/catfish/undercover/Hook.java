package com.catfish.undercover;

import java.lang.reflect.Method;

import android.app.Activity;
import android.util.Log;

import com.catfish.undercover.HookManager.HookedCallback;

public class Hook {
    public final static String TAG = "catfish";

    public static void main(String[] args) {
        Log.e(TAG, "hook starts");
        Method hookMethod = null;
        try {
            Class actyclass = Class.forName("android.app.Activity");
            hookMethod = actyclass.getDeclaredMethod("onContentChanged", (Class[]) null);
        } catch (Exception e) {
            e.printStackTrace();
        }
        HookManager.setMethodHooked(hookMethod, new HookedCallback() {
            @Override
            public Object preCalled() {
                return null;
            }

            @Override
            public Object postCalled() {
                return null;
            }

            @Override
            public boolean needInvokeOriginal() {
                return false;
            }
        }, true);
        new Activity().onContentChanged();
    }

}