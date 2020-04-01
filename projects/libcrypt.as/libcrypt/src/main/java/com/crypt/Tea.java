package com.crypt;

public class Tea
{
    static String TAG="Bear/Tea";
    public native void setPassword(String password);
    public native String encodeTextWithBase64(String text);
    public native String decodeTextWithBase64(String cryptText);

    public native byte[] encode(byte[] data);
    public native byte[] decode(byte[] data);

}
