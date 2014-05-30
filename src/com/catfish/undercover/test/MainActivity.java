package com.catfish.undercover.test;

import android.app.Activity;
import android.os.Bundle;

import com.catfish.undercover.Injector;
import com.catfish.undercover.R;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Injector inj = new Injector(this);
        inj.startInjection("system_server");
//        inj.startInjection("com.marvell.mars");
//        inj.startInjection("com.sina.show");
    }
}
