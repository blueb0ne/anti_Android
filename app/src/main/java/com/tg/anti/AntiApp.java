package com.tg.anti;

import android.app.Application;
import android.content.Context;
import android.util.Log;

public class AntiApp extends Application {

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        Log.d("ANTI-APP", "loader:"+this.getClassLoader());
        MyLoader.inject();
        Log.d("ANTI-APP", "loader after:"+this.getClassLoader());
        xcrash.XCrash.init(this);
    }
}
