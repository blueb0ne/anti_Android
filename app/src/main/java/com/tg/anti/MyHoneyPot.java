package com.tg.anti;

public class MyHoneyPot {
    private ClassLoader cl;
    public MyHoneyPot(ClassLoader cl){
        cl = cl.getParent();
    }
}
