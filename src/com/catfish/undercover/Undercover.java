package com.catfish.undercover;

import java.lang.reflect.Method;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.ViewGroup;

import com.catfish.undercover.HookedMethod.HookedCallback;

public class Undercover {
    public final static String TAG = "catfish";

    public void onInject(Context application) {
        Log.e(TAG, "hook starts");
        Method hookMethod = null;
        try {
            Class actyclass = Class.forName("com.marvell.mars.ui.privilege.PrivilegeFragment");
            hookMethod = actyclass.getDeclaredMethod("onCreateView", LayoutInflater.class, ViewGroup.class, Bundle.class);
        } catch (Exception e) {
            e.printStackTrace();
        }
        HookManager.setMethodHooked(hookMethod, new HookedCallback() {

            @Override
            public Object invoke(HookedMethod method, Object receiver, Object[] args) {
                Log.d(TAG, "hooked: " + method.getName() + ", args0=" + args[0] + ", args1=" + args[1] + ", args2=" + args[2]);
                return method.invoke(receiver, args);
            }
        });
    }

}