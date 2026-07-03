#pragma once

#include <stdint.h>
#include <stddef.h>

/* ── Estados do Portão ──────────────────────────────── */
#define GATE_STOPPED  0
#define GATE_OPENING  1
#define GATE_CLOSING  2

/* ── Estados de Pulso ──────────────────────────────── */
#define PULSE_IDLE        0
#define PULSE_OPEN_HIGH   1
#define PULSE_OPEN_DONE   2
#define PULSE_CLOSE_HIGH  3
#define PULSE_CLOSE_DONE  4

/* ── Pinos (WeMos D1 Mini - ESP8266) ────────────────*/
#define D1  5
#define D2  4
#define D3  0
#define D4  2
#define D5  14
#define D6  12
#define D7  13
#define D8  15

#define BOT_A     D1
#define BOT_A_LED D2
#define BOT_B     D3
#define BOT_B_LED D4
#define FA        D5
#define FA_LED    D6
#define FF        D7
#define FF_LED    D8

/* ── Constantes ────────────────────────────────────── */
#define PULSE_WIDTH_US 500

/* ── Estrutura de espionagem ────────────────────── */
typedef struct {
    int pin_states[20];
    int micros_calls;
    unsigned long last_micros;
} gpio_spy_t;

extern gpio_spy_t g_gpio_spy;

/* ── Protótipos do Arduino ──────────────────────── */
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
int digitalRead(int pin);

void noInterrupts(void);
void interrupts(void);

unsigned long micros(void);
void delay(unsigned int ms);

/* ── Funções de espionagem ──────────────────────── */
void gpio_spy_reset(void);
int gpio_read_pin(int pin);
void gpio_set_pin(int pin, int value);
