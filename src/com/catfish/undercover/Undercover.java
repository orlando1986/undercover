package com.catfish.undercover;

import android.app.Application;
import android.util.Log;

public class Undercover {
    public final static String TAG = "catfish";

    public void onInject(Application application) {
        Log.e(TAG, "hook starts");
    }
}