#include <cstdlib>
#include <cmath>
#include <ctime>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Behavior", __VA_ARGS__)

// Случайный разброс прицела
float RandomJitter(float base, float range) {
    float r = (float)rand() / RAND_MAX;
    return base + (r - 0.5f) * range;
}

// Плавное наведение с задержкой
struct AimState {
    float currentX, currentY;
    float targetX, targetY;
    float smooth;
    int delayMs;
};

void UpdateAim(AimState* state) {
    // Задержка перед началом наведения (имитация реакции человека)
    if (state->delayMs > 0) {
        state->delayMs -= 16;
        return;
    }
    
    // Плавное движение с ускорением/замедлением
    float dx = state->targetX - state->currentX;
    float dy = state->targetY - state->currentY;
    
    // Добавляем шум
    dx = RandomJitter(dx, 2.0f);
    dy = RandomJitter(dy, 2.0f);
    
    state->currentX += dx * state->smooth;
    state->currentY += dy * state->smooth;
    
    // Микро-дрожание (человеческий тремор)
    state->currentX += RandomJitter(0, 0.5f);
    state->currentY += RandomJitter(0, 0.5f);
}

// Случайная задержка между выстрелами
int GetRandomFireDelay() {
    return 50 + rand() % 150; // 50-200 мс
}
