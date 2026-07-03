#pragma once

#include <stdint.h>

/* ── GPIO/Pin modes ─────────────────────────────────────── */
#define INPUT       0
#define OUTPUT      1
#define INPUT_PULLUP 2

/* ── Digital levels ─────────────────────────────────────── */
#define LOW         0
#define HIGH        1

/* ── Estrutura de espionagem do LEDC ──────────────────── */
typedef struct {
    uint8_t rpwm_value;
    uint8_t lpwm_value;
    int pin_states[40];
    int setup_calls;
} motor_spy_t;

extern motor_spy_t g_motor_spy;

/* ── Protótipos do Arduino ─────────────────────────────── */
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
int digitalRead(int pin);

void ledcSetup(int channel, int freq, int resolution);
void ledcAttachPin(int pin, int channel);
void ledcWrite(int channel, int value);

/* ── Funções de espionagem ─────────────────────────────── */
void motor_spy_reset(void);
