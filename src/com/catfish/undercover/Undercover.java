package com.catfish.undercover;

import android.content.Context;
import android.util.Log;

public class Undercover {
    public final static String TAG = "catfish";

    public void onInject(Context application) {
        Log.e(TAG, "hook starts");
    }
}