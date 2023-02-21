package com.tg.anti;

import static com.tg.android.anti.NativeLib.*;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.net.URL;

public class MainActivity extends Activity {
    static private String TAG = "MUTI-MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        refreshUI();

        Button btn = findViewById(R.id.button);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                TextView xposed = findViewById(R.id.xposed);
                xposed.setText(AntiXposed());
            }
        });

        Toast.makeText(this,getApkPath(),Toast.LENGTH_LONG).show();
    }

    private void refreshUI(){
        TextView frida = findViewById(R.id.frida);
        frida.setText(AntiFrida());

        TextView xposed = findViewById(R.id.xposed);
        xposed.setText(AntiXposed());

        TextView root = findViewById(R.id.root);
        root.setText(AntiRoot());

        TextView debug = findViewById(R.id.debug);
        debug.setText(AntiDebug());

        TextView memDump = findViewById(R.id.memDump);
        memDump.setText(AntiMemDump());

        TextView emulator = findViewById(R.id.emulator);
        emulator.setText(AntiEmulator());

        TextView dualApp = findViewById(R.id.dualApp);
        dualApp.setText(AntiDualApp());
    }

    private static synchronized String getApkPath() {
        synchronized (MainActivity.class) {
            String str = null;
            long currentTimeMillis = System.currentTimeMillis();
            URL resource = MainActivity.class.getResource("/AndroidManifest.xml");
            if (resource == null) {
                Log.i(TAG, "Cannot load resource!");
                return null;
            }

            String valueOf = String.valueOf(resource);
            Log.i(TAG, "Resource URL is " + valueOf);
            String url = resource.toString();
            String substring = url.substring(9, url.lastIndexOf(33));
            Log.i(TAG, String.format("Found APK path %s after %d ms.", substring, Long.valueOf(System.currentTimeMillis() - currentTimeMillis)));
            return substring;
        }
    }

}