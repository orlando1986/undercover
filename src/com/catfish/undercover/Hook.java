package com.catfish.undercover;

import java.io.File;
import java.io.IOException;

import android.net.LocalServerSocket;
import android.util.Log;

public class Hook {
    static final String TAG = "catfish";
    private static final boolean DEBUG = true;

    public static void main(String[] args) {
        File source = new File(args[0]);
        int pid = android.os.Process.myPid();
        new Undercover().onInject();
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
                    e.printStackTrace();
                }
            }
        }
    }

    static void LOGD(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }
}
