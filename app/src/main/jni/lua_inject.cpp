#include <jni.h>
#include <cstring>
#include <dlfcn.h>
#include <android/log.h>
#include <cstdint>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "LuaInject", __VA_ARGS__)

// Дополнительные Lua-функции, если нужны
// Например, для регистрации C-функций в Lua
extern "C" {
    // ...
}
