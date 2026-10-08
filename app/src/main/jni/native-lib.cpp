#include <jni.h>
#include <string>
#include <cstring>
#include <dlfcn.h>
#include <android/log.h>
#include <cstdint>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <cstdlib>
#include <cstdio>
#include "dobby.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ModMenu", __VA_ARGS__)

static uintptr_t g_base = 0;
static void* g_lua_state = nullptr;

// ---- Lua функции из оффсетов ----
using lua_loadbufferx_t = int(*)(void*, const char*, size_t, const char*, const char*);
using lua_pcall_t = int(*)(void*, int, int, int);

static lua_loadbufferx_t orig_lua_loadbufferx = nullptr;
static lua_pcall_t orig_lua_pcall = nullptr;

// ---- Хук lua_pcall для получения L ----
int hook_lua_pcall(void* L, int nargs, int nresults, int errfunc) {
    if (!g_lua_state) {
        g_lua_state = L;
        LOGI("lua_State captured: %p", L);
    }
    return orig_lua_pcall(L, nargs, nresults, errfunc);
}

// ---- Получение базы libUE4.so ----
uintptr_t GetLibBase(const char* libName) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;
    char line[512];
    uintptr_t base = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, libName)) {
            base = strtoul(line, nullptr, 16);
            break;
        }
    }
    fclose(fp);
    return base;
}

// ---- JNI методы ----
extern "C" JNIEXPORT void JNICALL
Java_com_modmenu_NativeLoader_init(JNIEnv* env, jclass clazz) {
    g_base = GetLibBase("libUE4.so");
    if (!g_base) {
        LOGI("libUE4.so not found");
        return;
    }
    LOGI("libUE4.so base: 0x%lx", g_base);
    
    // Хукаем lua_pcall, чтобы получить lua_State
    uintptr_t lua_pcall_addr = g_base + 0xB761700;
    DobbyHook((void*)lua_pcall_addr, (void*)hook_lua_pcall, (void**)&orig_lua_pcall);
    LOGI("Hooked lua_pcall at 0x%lx", lua_pcall_addr);
    
    // Применяем байпассы
    applyBypasses(g_base);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_modmenu_NativeLoader_getLibBase(JNIEnv* env, jclass clazz, jstring libName) {
    const char* name = env->GetStringUTFChars(libName, nullptr);
    uintptr_t base = GetLibBase(name);
    env->ReleaseStringUTFChars(libName, name);
    return base;
}

extern "C" JNIEXPORT void JNICALL
Java_com_modmenu_NativeLoader_loadAndRunLua(JNIEnv* env, jclass clazz, jlong L, jbyteArray code, jstring name) {
    if (!L || !g_base) return;
    
    jsize len = env->GetArrayLength(code);
    jbyte* buf = env->GetByteArrayElements(code, nullptr);
    const char* scriptName = env->GetStringUTFChars(name, nullptr);
    
    uintptr_t loadbufferx_addr = g_base + 0xB785B3C;
    uintptr_t pcall_addr = g_base + 0xB761700;
    
    lua_loadbufferx_t lua_loadbufferx = (lua_loadbufferx_t)loadbufferx_addr;
    lua_pcall_t lua_pcall = (lua_pcall_t)pcall_addr;
    
    int status = lua_loadbufferx((void*)L, (const char*)buf, len, scriptName, "bt");
    if (status == 0) {
        lua_pcall((void*)L, 0, 0, 0);
        LOGI("Lua script %s executed", scriptName);
    } else {
        LOGI("Lua load error for %s: %d", scriptName, status);
    }
    
    env->ReleaseByteArrayElements(code, buf, JNI_ABORT);
    env->ReleaseStringUTFChars(name, scriptName);
}

extern "C" JNIEXPORT void JNICALL
Java_com_modmenu_NativeLoader_setLuaState(JNIEnv* env, jclass clazz, jlong L) {
    g_lua_state = (void*)L;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_modmenu_NativeLoader_getLuaState(JNIEnv* env, jclass clazz) {
    return (jlong)g_lua_state;
}

extern "C" JNIEXPORT void JNICALL
Java_com_modmenu_NativeLoader_applyBypasses(JNIEnv* env, jclass clazz, jlong base) {
    applyBypasses((uintptr_t)base);
}

extern "C" JNIEXPORT void JNICALL
Java_com_modmenu_NativeLoader_hookLuaPcall(JNIEnv* env, jclass clazz, jlong base) {
    uintptr_t addr = (uintptr_t)base + 0xB761700;
    DobbyHook((void*)addr, (void*)hook_lua_pcall, (void**)&orig_lua_pcall);
}
