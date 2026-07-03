#include <stdio.h>
#include <string.h>
#include "motor_fixo.h"
#include "mocks/fake_esp8266.h"
#include "mocks/esp_log.h"

/* ── Estado global ─────────────────────────────────────── */
static volatile gate_state_t state = GATE_STOPPED;
static volatile pulse_state_t pulse_state = PULSE_IDLE;
static volatile unsigned long pulse_start_us = 0;

/* ════════════════════════════════════════════════════════
 * INICIALIZAÇÃO
 * ════════════════════════════════════════════════════════ */

void gate_init(void)
{
    pinMode(BOT_A, 1);
    pinMode(BOT_A_LED, 1);
    pinMode(BOT_B, 1);
    pinMode(BOT_B_LED, 1);
    pinMode(FA, 1);
    pinMode(FA_LED, 1);
    pinMode(FF, 1);
    pinMode(FF_LED, 1);

    digitalWrite(BOT_A, 0);
    digitalWrite(BOT_B, 0);
    digitalWrite(BOT_A_LED, 0);
    digitalWrite(BOT_B_LED, 0);
    digitalWrite(FA, 0);
    digitalWrite(FF, 0);
    digitalWrite(FA_LED, 0);
    digitalWrite(FF_LED, 0);

    state = GATE_STOPPED;
    pulse_state = PULSE_IDLE;
    pulse_start_us = 0;

    ESP_LOGI("GATE", "Gate inicializado");
}

/* ════════════════════════════════════════════════════════
 * CONTROLE DO PORTÃO
 * ════════════════════════════════════════════════════════ */

void gate_stop(void)
{
    digitalWrite(BOT_A, 0);
    digitalWrite(BOT_B, 0);
    digitalWrite(BOT_A_LED, 0);
    digitalWrite(BOT_B_LED, 0);
    state = GATE_STOPPED;
    pulse_state = PULSE_IDLE;
    ESP_LOGI("GATE", "Portao parado");
}

void gate_open(void)
{
    if (state != GATE_STOPPED) {
        ESP_LOGW("GATE", "Portao nao esta parado, nao abrir");
        return;
    }

    digitalWrite(BOT_B, 0);
    digitalWrite(BOT_B_LED, 0);
    digitalWrite(BOT_A, 1);
    digitalWrite(BOT_A_LED, 1);
    digitalWrite(FF, 1);
    digitalWrite(FF_LED, 1);

    pulse_start_us = micros();
    pulse_state = PULSE_OPEN_HIGH;
    state = GATE_OPENING;
    ESP_LOGI("GATE", "Abrindo portao");
}

void gate_close(void)
{
    if (state != GATE_STOPPED) {
        ESP_LOGW("GATE", "Portao nao esta parado, nao fechar");
        return;
    }

    digitalWrite(BOT_A, 0);
    digitalWrite(BOT_A_LED, 0);
    digitalWrite(BOT_B, 1);
    digitalWrite(BOT_B_LED, 1);
    digitalWrite(FA, 1);
    digitalWrite(FA_LED, 1);

    pulse_start_us = micros();
    pulse_state = PULSE_CLOSE_HIGH;
    state = GATE_CLOSING;
    ESP_LOGI("GATE", "Fechando portao");
}

/* ════════════════════════════════════════════════════════
 * ATUALIZAÇÃO DE PULSO (máquina de estados)
 * ════════════════════════════════════════════════════════ */

void gate_update_pulse(void)
{
    if (pulse_state == PULSE_IDLE) {
        return;
    }

    unsigned long now = micros();

    switch (pulse_state) {
        case PULSE_OPEN_HIGH:
            if (now - pulse_start_us >= PULSE_WIDTH_US) {
                digitalWrite(BOT_A, 0);
                pulse_start_us = micros();
                pulse_state = PULSE_OPEN_DONE;
            }
            break;

        case PULSE_OPEN_DONE:
            if (now - pulse_start_us >= PULSE_WIDTH_US) {
                digitalWrite(FF, 0);
                digitalWrite(FF_LED, 0);
                pulse_state = PULSE_IDLE;
                state = GATE_STOPPED;
                ESP_LOGI("GATE", "Portao finalizou abertura");
            }
            break;

        case PULSE_CLOSE_HIGH:
            if (now - pulse_start_us >= PULSE_WIDTH_US) {
                digitalWrite(BOT_B, 0);
                pulse_start_us = micros();
                pulse_state = PULSE_CLOSE_DONE;
            }
            break;

        case PULSE_CLOSE_DONE:
            if (now - pulse_start_us >= PULSE_WIDTH_US) {
                digitalWrite(FA, 0);
                digitalWrite(FA_LED, 0);
                pulse_state = PULSE_IDLE;
                state = GATE_STOPPED;
                ESP_LOGI("GATE", "Portao finalizou fechamento");
            }
            break;

        default:
            break;
    }
}

/* ════════════════════════════════════════════════════════
 * ESTADO
 * ════════════════════════════════════════════════════════ */

gate_state_t gate_get_state(void)
{
    return state;
}

pulse_state_t gate_get_pulse_state(void)
{
    return pulse_state;
}

int gate_get_pin_state(int pin)
{
    return digitalRead(pin);
}
