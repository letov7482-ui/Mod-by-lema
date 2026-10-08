package com.modmenu;

import de.robv.android.xposed.*;
import de.robv.android.xposed.callbacks.XC_LoadPackage;
import android.app.Application;
import android.content.Context;
import android.content.res.AssetManager;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;

public class MainHook implements IXposedHookLoadPackage {
    
    public static final String PUBG_PACKAGE = "com.tencent.ig";
    public static final String PUBGM_PACKAGE = "com.pubg.imobile";
    public static final String BGMI_PACKAGE = "com.pubg.krmobile";
    
    private static Context gameContext;
    private static ClassLoader gameClassLoader;
    
    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) {
        if (!lpparam.packageName.equals(PUBG_PACKAGE) 
            && !lpparam.packageName.equals(PUBGM_PACKAGE)
            && !lpparam.packageName.equals(BGMI_PACKAGE)) return;
        
        XposedBridge.log("[ModMenu] Hooked: " + lpparam.packageName);
        
        gameClassLoader = lpparam.classLoader;
        gameContext = (Context) AndroidAppHelper.currentApplication();
        
        // Инициализация нативных хуков (libanogs, libtersafe, Lua)
        NativeLoader.init();
        
        // Внедряем вкладку в настройки игры
        SettingsInjector.inject(lpparam);
        
        // Фикс puffer_temp и pak
        PufferFix.apply(lpparam.classLoader);
        
        // Ждём появления lua_State и инжектим скрипты
        new Thread(() -> {
            while (true) {
                long L = NativeLoader.getLuaState();
                if (L != 0) {
                    injectLuaScripts(L);
                    break;
                }
                try { Thread.sleep(500); } catch (InterruptedException ignored) {}
            }
        }).start();
    }
    
    private static void injectLuaScripts(long L) {
        try {
            AssetManager am = gameContext.getAssets();
            String[] scripts = {
                "lua/config.lua",
                "lua/recoil.lua",
                "lua/spread.lua",
                "lua/damage.lua",
                "lua/speed.lua",
                "lua/aimbot.lua"
            };
            
            for (String script : scripts) {
                InputStream is = am.open(script);
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                byte[] buf = new byte[4096];
                int n;
                while ((n = is.read(buf)) != -1) baos.write(buf, 0, n);
                is.close();
                byte[] code = baos.toByteArray();
                
                NativeLoader.loadAndRunLua(L, code, script);
                XposedBridge.log("[ModMenu] Loaded: " + script);
            }
        } catch (Throwable t) {
            XposedBridge.log("[ModMenu] injectLuaScripts: " + t);
        }
    }
    
    public static ClassLoader getGameClassLoader() { return gameClassLoader; }
    public static Context getGameContext() { return gameContext; }
}
