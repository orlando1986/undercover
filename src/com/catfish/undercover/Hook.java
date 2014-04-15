package com.catfish.undercover;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

import android.app.Application;
import android.net.LocalServerSocket;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

public class Hook {
    static final String TAG = "catfish";
    private static final boolean DEBUG = true;
    private static HandlerThread sThread = null;
    private static Handler sHandler = null;
    private static final int MSG_FINDAPP = 0;
    private static final int MSG_SOCKET = 1;
    private static Object sReflectActivityThread = null;

    private static class HookHandler extends Handler {
        HookHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_FINDAPP:
                Application app = findApplication(sReflectActivityThread);
                if (app == null) {
                    Log.e(TAG, "can not find application, delay 500 millisec");
                    sHandler.sendMessageDelayed(Message.obtain(msg), 500);
                } else {
                    new Hook().replaceParentClassLoader(app);
                    sHandler.sendMessage(Message.obtain(sHandler, MSG_SOCKET, msg.obj));
                }
                break;
            case MSG_SOCKET:
                try {
                    ((LocalServerSocket) msg.obj).accept();
                    sHandler.sendMessage(msg);
                } catch (IOException e) {
                    Log.e(TAG, "socket handles message failed!");
                }
                break;
            }
        }
    }

    public static void main(String[] args) {
        File source = new File(args[0]);
        int pid = android.os.Process.myPid();
        sReflectActivityThread = findActivityThread();
        LocalServerSocket server = null;
        try {
            server = new LocalServerSocket(pid + ":" + source.lastModified());
        } catch (IOException e) {
            Log.e(TAG, "create server failed!");
        }
        if (sThread == null) {
            sThread = new HandlerThread("catfish");
            sThread.start();
        }
        if (sHandler == null) {
            sHandler = new HookHandler(sThread.getLooper());
        }
        sHandler.sendMessage(Message.obtain(sHandler, MSG_FINDAPP, server));
    }

    private void replaceParentClassLoader(Application app) {
        try {
            Class<?> clclz = Class.forName("java.lang.ClassLoader");
            ClassLoader l = Hook.class.getClassLoader();
            Field pf = clclz.getDeclaredField("parent");
            pf.setAccessible(true);
            pf.set(l, app.getClassLoader());
        } catch (Exception e) {
            Log.e(TAG, "replace parent classloader failed: " + e.getMessage());
        }
        new Undercover().onInject(app);
    }

    static void LOGD(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    private final static Object findActivityThread() {
        try {
            Class<?> atclz = Class.forName("android.app.ActivityThread");
            Method currentActivityThread = atclz.getDeclaredMethod("currentActivityThread", (Class[]) null);
            currentActivityThread.setAccessible(true);
            Object activitythread = currentActivityThread.invoke(null);
            if (activitythread == null) { // system_server
                Class<?> activitymanager = Class.forName("com.android.server.am.ActivityManagerService");
                Field f = activitymanager.getDeclaredField("mSystemThread");
                f.setAccessible(true);
                activitythread = f.get(null);
            }
            return activitythread;
        } catch (Exception e) {
            Log.e(TAG, "findApplication failed", e);
        }
        return null;
    }

    private final static Application findApplication(Object activitythread) {
        try {
            Field appf = activitythread.getClass().getDeclaredField("mInitialApplication");
            appf.setAccessible(true);
            Application app = (Application) appf.get(activitythread);
            return app;
        } catch (Exception e) {
            Log.e(TAG, "findApplication failed", e);
        }
        return null;
    }
}
