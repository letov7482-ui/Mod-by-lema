#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Obf", __VA_ARGS__)

// XOR-шифрование секции кода
static const uint8_t XOR_KEY[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};

void EncryptSection(void* addr, size_t size) {
    uint8_t* p = (uint8_t*)addr;
    for (size_t i = 0; i < size; i++) {
        p[i] ^= XOR_KEY[i % sizeof(XOR_KEY)];
    }
}

void DecryptSection(void* addr, size_t size) {
    // XOR симметричен
    EncryptSection(addr, size);
}

// Пример: зашифрованная функция
__attribute__((section(".encrypted"))) 
void SensitiveFunction() {
    // код чита
}

// Расшифровка перед вызовом
void CallSensitive() {
    void* addr = (void*)SensitiveFunction;
    size_t size = 0x100; // примерный размер
    uintptr_t page = (uintptr_t)addr & ~0xFFF;
    mprotect((void*)page, 0x2000, PROT_READ | PROT_WRITE | PROT_EXEC);
    DecryptSection(addr, size);
    SensitiveFunction();
    EncryptSection(addr, size); // обратно
}
