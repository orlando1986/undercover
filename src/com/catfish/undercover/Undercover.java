package com.catfish.undercover;

import java.lang.reflect.Method;

import android.content.Context;
import android.util.Log;

import com.catfish.undercover.HookedMethod.HookedCallback;

public class Undercover {
    public final static String TAG = "catfish";

    public void onInject(Context application) {
        Log.e(TAG, "hook starts");
}