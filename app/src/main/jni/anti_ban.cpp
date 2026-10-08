#include <jni.h>
#include <cstring>
#include <dlfcn.h>
#include <android/log.h>
#include <cstdint>
#include <sys/socket.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "dobby.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "AntiBan", __VA_ARGS__)

// ---- Хранилище валидных heartbeat ----
static unsigned char g_heartbeat_buffer[4096];
static size_t g_heartbeat_len = 0;
static bool g_heartbeat_captured = false;

// ---- Перехват send для захвата и подмены ----
static ssize_t (*orig_send)(int, const void*, size_t, int) = nullptr;

bool IsAnogsPacket(const void* buf, size_t len) {
    if (len < 16) return false;
    const unsigned char* p = (const unsigned char*)buf;
    // Сигнатура anogs: ищем магические байты
    return p[0] == 0x41 && p[1] == 0x4E && p[2] == 0x4F && p[3] == 0x47;
}

ssize_t hook_send(int sockfd, const void* buf, size_t len, int flags) {
    if (IsAnogsPacket(buf, len)) {
        if (!g_heartbeat_captured) {
            // Захватываем первый валидный пакет (он уходит до модификаций)
            memcpy(g_heartbeat_buffer, buf, len);
            g_heartbeat_len = len;
            g_heartbeat_captured = true;
            LOGI("Captured valid heartbeat: %zu bytes", len);
        } else {
            // Подменяем только подозрительные поля
            unsigned char* modified = (unsigned char*)malloc(len);
            memcpy(modified, buf, len);
            
            // Пример: обнуляем поля, которые могут указывать на чит
            // (точные оффсеты зависят от формата пакета)
            // modified[12] = 0x00; // флаг модификации
            // modified[13] = 0x00;
            
            ssize_t ret = orig_send(sockfd, modified, len, flags);
            free(modified);
            return ret;
        }
    }
    return orig_send(sockfd, buf, len, flags);
}

// ---- Инициализация ----
void InitHeartbeatSpoof() {
    void* libc = dlopen("libc.so", RTLD_NOW);
    if (!libc) return;
    
    orig_send = (ssize_t(*)(int, const void*, size_t, int))dlsym(libc, "send");
    if (orig_send) {
        DobbyHook((void*)orig_send, (void*)hook_send, (void**)&orig_send);
        LOGI("Heartbeat spoof initialized");
    }
}
