package com.coreunittest;

public class LooperJni
{
    private static String TAG="Bear/LooperJni";
    static
    {
        System.loadLibrary("corelooper");
    }

    public LooperJni(String params)
    {
        init(params);
    }

    protected native int init(String params);

    public native void release();
}
