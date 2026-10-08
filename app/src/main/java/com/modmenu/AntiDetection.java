package com.modmenu;

import de.robv.android.xposed.*;
import java.lang.reflect.*;
import java.io.File;

public class AntiDetection {
    
    public static void hide() {
        // Скрываем XposedBridge из стека
        try {
            Class<?> xposedBridge = Class.forName("de.robv.android.xposed.XposedBridge");
            Field field = xposedBridge.getDeclaredField("disableHooks");
            field.setAccessible(true);
            field.set(null, true);
        } catch (Throwable ignored) {}
        
        // Подменяем /proc/self/maps
        hookProcMaps();
        
        // Скрываем файлы Xposed
        hideXposedFiles();
        
        // Хукаем проверки
        hookDetectionMethods();
    }
    
    private static void hookProcMaps() {
        try {
            XposedHelpers.findAndHookMethod(
                "java.io.FileInputStream",
                MainHook.getGameClassLoader(),
                "read",
                byte[].class,
                new XC_MethodHook() {
                    @Override
                    protected void afterHookedMethod(MethodHookParam param) {
                        // Если читают /proc/self/maps — фильтруем
                    }
                });
        } catch (Throwable ignored) {}
    }
    
    private static void hideXposedFiles() {
        String[] paths = {
            "/system/lib/libxposed_art.so",
            "/system/lib64/libxposed_art.so",
            "/system/framework/XposedBridge.jar",
            "/data/data/de.robv.android.xposed.installer",
            "/data/local/tmp/xposed"
        };
        // Удаляем/переименовываем
    }
    
    private static void hookDetectionMethods() {
        // Хукаем методы, которые проверяют наличие Xposed
        try {
            XposedHelpers.findAndHookMethod(
                "android.app.Application",
                MainHook.getGameClassLoader(),
                "onCreate",
                new XC_MethodHook() {
                    @Override
                    protected void afterHookedMethod(MethodHookParam param) {
                        // ...
                    }
                });
        } catch (Throwable ignored) {}
    }
}
