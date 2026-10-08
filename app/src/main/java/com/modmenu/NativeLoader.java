package com.modmenu;

public class NativeLoader {
    static {
        System.loadLibrary("modmenu");
    }
    
    public static native void init();
    public static native long getLibBase(String libName);
    public static native void loadAndRunLua(long L, byte[] code, String name);
    public static native void setLuaState(long L);
    public static native long getLuaState();
    public static native void applyBypasses(long base);
    public static native void hookLuaPcall(long base);
}
