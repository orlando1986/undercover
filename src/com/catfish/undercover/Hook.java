package com.catfish.undercover;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

import android.app.Application;
import android.net.LocalServerSocket;
import android.util.Log;

public class Hook {
    static final String TAG = "catfish";
    private static final boolean DEBUG = true;

    public static void main(String[] args) {
        File source = new File(args[0]);
        int pid = android.os.Process.myPid();
        Application app = findApplication();
        try {
            Class<?> clclz = Class.forName("java.lang.ClassLoader");
            ClassLoader l = Hook.class.getClassLoader();
            Field pf = clclz.getDeclaredField("parent");
            pf.setAccessible(true);
            if (app != null) {
                pf.set(l, app.getClassLoader());
            }
        } catch (Exception e) {
            Log.e(TAG, "replace parent classloader failed!");
        }
        new Undercover().onInject(app);
        try {
            LocalServerSocket server = new LocalServerSocket(pid + ":" + source.lastModified());
            new VersionServer(server).start();
        } catch (IOException e) {
            Log.e(TAG, "create server failed!");
        }
    }

    private static class VersionServer extends Thread {
        private LocalServerSocket mServer = null;

        public VersionServer(LocalServerSocket server) {
            mServer = server;
        }

        @Override
        public void run() {
            while (true) {
                try {
                    mServer.accept();
                } catch (IOException e) {
                    Log.e(TAG, "socket handles message failed!");
                }
            }
        }
    }

    static void LOGD(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    private static Application findApplication() {
        try {
            Class<?> atclz = Class.forName("android.app.ActivityThread");
            Method currentActivityThread = atclz.getDeclaredMethod("currentActivityThread", (Class[]) null);
            currentActivityThread.setAccessible(true);
            Object activitythread = currentActivityThread.invoke(null);
            if (activitythread == null) {  // system_server
                Class<?> activitymanager = Class.forName("com.android.server.am.ActivityManagerService");
                Field f = activitymanager.getDeclaredField("mSystemThread");
                f.setAccessible(true);
                activitythread = f.get(null);
            }

            Field appf = atclz.getDeclaredField("mInitialApplication");
            appf.setAccessible(true);
            Application app = (Application) appf.get(activitythread);
            return app;
        } catch (Exception e) {
            Log.e(TAG, "findApplication failed", e);
        }
        return null;
    }
}
