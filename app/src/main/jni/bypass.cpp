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
#include "dobby.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Bypass", __VA_ARGS__)

// ---- Оффсеты античита 4.6.0 ----
namespace Anogs {
    constexpr uintptr_t CHECK_INTEGRITY_1 = 0x4DEDE8;
    constexpr uintptr_t CHECK_INTEGRITY_2 = 0x520480;
    constexpr uintptr_t HEARTBEAT_SEND    = 0x3A564C;
    constexpr uintptr_t DETECT_INJECT     = 0x12C8DD;
    constexpr uintptr_t KILL_SLEEP        = 0x12322C;
    constexpr uintptr_t SSL_VERIFY        = 0x2F1A40;
    constexpr uintptr_t MEMORY_SCAN       = 0x3B7C20;
}

namespace Tersafe {
    constexpr uintptr_t BYPASS_17 = 0xDD538;
    constexpr uintptr_t BYPASS_18 = 0xDF9B0;
}

// ---- Патчи ----
void PatchReturnZero(uintptr_t addr) {
    uint32_t patch[] = {0x52800000, 0xD65F03C0}; // mov w0, #0; ret
    uintptr_t page = addr & ~0xFFF;
    mprotect((void*)page, 0x2000, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy((void*)addr, patch, sizeof(patch));
    __builtin___clear_cache((char*)addr, (char*)addr + sizeof(patch));
}

void PatchRet(uintptr_t addr) {
    uint32_t ret = 0xD65F03C0;
    uintptr_t page = addr & ~0xFFF;
    mprotect((void*)page, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy((void*)addr, &ret, sizeof(ret));
    __builtin___clear_cache((char*)addr, (char*)addr + sizeof(ret));
}

void PatchNop(uintptr_t addr) {
    uint32_t nop = 0xD503201F;
    uintptr_t page = addr & ~0xFFF;
    mprotect((void*)page, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy((void*)addr, &nop, sizeof(nop));
    __builtin___clear_cache((char*)addr, (char*)addr + sizeof(nop));
}

// ---- Heart-beat spoofing ----
static ssize_t (*orig_send)(int, const void*, size_t, int) = nullptr;
static ssize_t (*orig_sendto)(int, const void*, size_t, int, const struct sockaddr*, socklen_t) = nullptr;

// Здесь должен быть валидный пакет, снятый с чистой игры.
// В реальности его нужно перехватить и сохранить.
static unsigned char g_valid_heartbeat[] = { /* ... */ };
static size_t g_valid_heartbeat_len = 0;

bool IsAnogsHeartbeat(const void* buf, size_t len) {
    // Проверка сигнатуры пакета anogs
    if (len < 8) return false;
    const unsigned char* p = (const unsigned char*)buf;
    // Пример: заголовок 0x41 0x4E 0x4F 0x47 (ANOG)
    return p[0] == 0x41 && p[1] == 0x4E && p[2] == 0x4F && p[3] == 0x47;
}

ssize_t hook_send(int sockfd, const void* buf, size_t len, int flags) {
    if (IsAnogsHeartbeat(buf, len) && g_valid_heartbeat_len > 0) {
        LOGI("Spoofing anogs heartbeat");
        return orig_send(sockfd, g_valid_heartbeat, g_valid_heartbeat_len, flags);
    }
    return orig_send(sockfd, buf, len, flags);
}

ssize_t hook_sendto(int sockfd, const void* buf, size_t len, int flags,
                    const struct sockaddr* dest_addr, socklen_t addrlen) {
    if (IsAnogsHeartbeat(buf, len) && g_valid_heartbeat_len > 0) {
        LOGI("Spoofing anogs sendto heartbeat");
        return orig_sendto(sockfd, g_valid_heartbeat, g_valid_heartbeat_len,
                           flags, dest_addr, addrlen);
    }
    return orig_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

// ---- SSL pinning bypass ----
static int (*orig_SSL_CTX_set_verify)(void*, int, void*) = nullptr;
static int (*orig_SSL_get_verify_result)(void*) = nullptr;

int hook_SSL_CTX_set_verify(void* ctx, int mode, void* cb) {
    LOGI("SSL_CTX_set_verify bypassed");
    return orig_SSL_CTX_set_verify(ctx, 0, nullptr); // SSL_VERIFY_NONE
}

int hook_SSL_get_verify_result(void* ssl) {
    return 0; // X509_V_OK
}

// ---- Memory scan evasion ----
// libanogs сканирует память на наличие сигнатур чита.
// Нужно либо шифровать свой код, либо хукать функции сканирования.
static int (*orig_memcmp)(const void*, const void*, size_t) = nullptr;
static void* (*orig_memmem)(const void*, size_t, const void*, size_t) = nullptr;

int hook_memcmp(const void* s1, const void* s2, size_t n) {
    // Если сравнивают сигнатуру чита — возвращаем "не совпадает"
    if (n > 4 && n < 64) {
        // Можно добавить проверку на наши сигнатуры
    }
    return orig_memcmp(s1, s2, n);
}

// ---- Применение всех байпассов ----
void applyBypasses(uintptr_t base) {
    // Патчи libanogs
    uintptr_t anogs_base = 0;
    // libanogs.so загружается динамически, нужно найти её базу
    // (можно через dl_iterate_phdr или /proc/self/maps)
    // Для примера:
    anogs_base = (uintptr_t)dlopen("libanogs.so", RTLD_NOW);
    if (anogs_base) {
        // Патчи
        PatchReturnZero(anogs_base + Anogs::CHECK_INTEGRITY_1);
        PatchReturnZero(anogs_base + Anogs::CHECK_INTEGRITY_2);
        PatchRet(anogs_base + Anogs::HEARTBEAT_SEND);
        PatchReturnZero(anogs_base + Anogs::DETECT_INJECT);
        PatchRet(anogs_base + Anogs::KILL_SLEEP);
        PatchReturnZero(anogs_base + Anogs::SSL_VERIFY);
        PatchRet(anogs_base + Anogs::MEMORY_SCAN);
        LOGI("Anogs patches applied");
    }
    
    // Патчи libtersafe
    uintptr_t tersafe_base = (uintptr_t)dlopen("libtersafe.so", RTLD_NOW);
    if (tersafe_base) {
        uint32_t zero[4] = {0};
        mprotect((void*)((tersafe_base + Tersafe::BYPASS_17) & ~0xFFF), 0x2000,
                 PROT_READ | PROT_WRITE | PROT_EXEC);
        mprotect((void*)((tersafe_base + Tersafe::BYPASS_18) & ~0xFFF), 0x2000,
                 PROT_READ | PROT_WRITE | PROT_EXEC);
        memcpy((void*)(tersafe_base + Tersafe::BYPASS_17), zero, 4);
        memcpy((void*)(tersafe_base + Tersafe::BYPASS_18), zero, 4);
        __builtin___clear_cache((char*)(tersafe_base + Tersafe::BYPASS_17),
                                (char*)(tersafe_base + Tersafe::BYPASS_17 + 4));
        LOGI("Tersafe patches applied");
    }
    
    // Хуки libc
    void* libc = dlopen("libc.so", RTLD_NOW);
    if (libc) {
        orig_send = (ssize_t(*)(int, const void*, size_t, int))dlsym(libc, "send");
        orig_sendto = (ssize_t(*)(int, const void*, size_t, int, const struct sockaddr*, socklen_t))dlsym(libc, "sendto");
        if (orig_send) DobbyHook((void*)orig_send, (void*)hook_send, (void**)&orig_send);
        if (orig_sendto) DobbyHook((void*)orig_sendto, (void*)hook_sendto, (void**)&orig_sendto);
        LOGI("libc send hooks applied");
    }
    
    // SSL hooks
    void* ssl = dlopen("libssl.so", RTLD_NOW);
    if (ssl) {
        orig_SSL_CTX_set_verify = (int(*)(void*, int, void*))dlsym(ssl, "SSL_CTX_set_verify");
        orig_SSL_get_verify_result = (int(*)(void*))dlsym(ssl, "SSL_get_verify_result");
        if (orig_SSL_CTX_set_verify) DobbyHook((void*)orig_SSL_CTX_set_verify, (void*)hook_SSL_CTX_set_verify, (void**)&orig_SSL_CTX_set_verify);
        if (orig_SSL_get_verify_result) DobbyHook((void*)orig_SSL_get_verify_result, (void*)hook_SSL_get_verify_result, (void**)&orig_SSL_get_verify_result);
        LOGI("SSL hooks applied");
    }
    
    // Memory scan hooks
    orig_memcmp = (int(*)(const void*, const void*, size_t))dlsym(libc, "memcmp");
    if (orig_memcmp) DobbyHook((void*)orig_memcmp, (void*)hook_memcmp, (void**)&orig_memcmp);
    
    LOGI("All bypasses applied");
}
