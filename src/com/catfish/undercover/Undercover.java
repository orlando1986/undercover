package com.catfish.undercover;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.catfish.undercover.HookedMethod.HookedCallback;

public class Undercover {
    public final static String TAG = "catfish";

    public void onInject(Context application) {
        Log.e(TAG, "hook starts");
        Method addView = null;
        try {
            Class CustomGridView = Class.forName("android.view.ViewGroup");
            addView = CustomGridView.getDeclaredMethod("addView", View.class, android.view.ViewGroup.LayoutParams.class);
        } catch (Exception e) {
            Log.e(TAG, e.getMessage(), e);
        }
        HookedCallback h = new HookedCallback() {

            @Override
            public Object invoke(HookedMethod method, Object receiver, Object[] args) {
                Object result = method.invoke(receiver, args);
                try {
                    Class CustomGridView = Class.forName("com.tencent.mm.plugin.brandservice.ui.base.CustomGridView");
                    if (receiver.getClass().getName().equals(CustomGridView.getName())) {
                        View content = (View) args[0];
                        Object obj = content.getTag();
                        Field usernamef = obj.getClass().getDeclaredField("username");
                        usernamef.setAccessible(true);
                        Log.d(TAG, "username=" + usernamef.get(obj));
                    }
                } catch (Exception e) {
                    Log.e(TAG, e.getMessage(), e);
                }
                return result;
            }
        };
        HookManager.setMethodHooked(addView, h);
    }
}