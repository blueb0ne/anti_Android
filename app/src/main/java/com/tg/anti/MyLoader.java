package com.tg.anti;

import android.util.Log;

import java.lang.reflect.Field;

import dalvik.system.PathClassLoader;

public class MyLoader extends PathClassLoader{
    public static int mClassLoaded = 0;
    public MyLoader(String dexPath, ClassLoader parent){
        super(dexPath, parent);
    }

    @Override
    protected Class<?> findClass(String name) throws ClassNotFoundException {
        Log.i("ANTI-Classloader", "index:"+(mClassLoaded++) + " try find class:"+name);
        if(name.contains("MyHoneyPot")){
            Log.e("ANTI-Classloader", "trigger honeypot,find unpacker!!");
        }
        return super.findClass(name);
    }

    public static void inject(){
        ClassLoader pathClassLoader = AntiApp.class.getClassLoader();
        MyLoader cl = new MyLoader("", pathClassLoader.getParent());
        setParent(pathClassLoader, cl);
    }

    private static void setParent(ClassLoader cl, ClassLoader newParent){
        try{
            Field f = ClassLoader.class.getDeclaredField("parent");
            f.setAccessible(true);
            f.set(cl, newParent);
        }catch (Throwable e){
            throw new RuntimeException(e);
        }
    }
}
