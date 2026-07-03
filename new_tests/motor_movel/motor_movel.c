/* ============================================================
 * Motor Movel - Implementação testável
 * ============================================================ */

#include <stdio.h>
#include <string.h>
#include "motor_movel.h"
#include "mocks/fake_arduino.h"
#include "mocks/esp_log.h"

/* ── Estado global ─────────────────────────────────────── */
static volatile estado_t state = PARADO;

/* ── Protótipos internos ────────────────────────────── */
static void init_motor_pins(void);

/* ============================================================
 * INICIALIZAÇÃO
 * ============================================================ */

void motor_init(void)
{
    init_motor_pins();
    motor_parar();
    digitalWrite(ENABLE_PIN, HIGH);
    ESP_LOGI("MOTOR", "Motor inicializado");
}

static void init_motor_pins(void)
{
    pinMode(ENABLE_PIN, OUTPUT);
    ledcSetup(CANAL_RPWM, PWM_FREQ, PWM_RES);
    ledcSetup(CANAL_LPWM, PWM_FREQ, PWM_RES);
    ledcAttachPin(RPWM, CANAL_RPWM);
    ledcAttachPin(LPWM, CANAL_LPWM);
    pinMode(FCS_PIN, INPUT_PULLUP);
    pinMode(FCI_PIN, INPUT_PULLUP);
}

/* ============================================================
 * CONTROLE DO MOTOR
 * ============================================================ */

void motor_parar(void)
{
    ledcWrite(CANAL_RPWM, 0);
    ledcWrite(CANAL_LPWM, 0);
    state = PARADO;
    ESP_LOGI("MOTOR", "Motor parado");
}

void motor_subir(void)
{
    if (fim_superior()) {
        ESP_LOGE("MOTOR", "Fim de curso superior ja acionado");
        return;
    }

    if (state == SUBINDO) {
        ESP_LOGW("MOTOR", "Motor ja esta subindo");
        return;
    }

    if (state == DESCENDO) {
        motor_parar();
    }

    ledcWrite(CANAL_LPWM, 0);
    ledcWrite(CANAL_RPWM, VELOCIDADE);
    state = SUBINDO;
    ESP_LOGI("MOTOR", "Motor subindo");
}

void motor_descer(void)
{
    if (fim_inferior()) {
        ESP_LOGE("MOTOR", "Fim de curso inferior ja acionado");
        return;
    }

    if (state == DESCENDO) {
        ESP_LOGW("MOTOR", "Motor ja esta descendo");
        return;
    }

    if (state == SUBINDO) {
        motor_parar();
    }

    ledcWrite(CANAL_RPWM, 0);
    ledcWrite(CANAL_LPWM, VELOCIDADE);
    state = DESCENDO;
    ESP_LOGI("MOTOR", "Motor descendo");
}

/* ============================================================
 * FINS DE CURSO
 * ============================================================ */

int fim_superior(void)
{
    return digitalRead(FCS_PIN) == LOW;
}

int fim_inferior(void)
{
    return digitalRead(FCI_PIN) == LOW;
}

void verificar_fins_de_curso(void)
{
    if (state == SUBINDO && fim_superior()) {
        motor_parar();
        ESP_LOGI("MOTOR", "FCS acionado por hardware");
    }
    if (state == DESCENDO && fim_inferior()) {
        motor_parar();
        ESP_LOGI("MOTOR", "FCI acionado por hardware");
    }
}

/* ============================================================
 * ESTADO
 * ============================================================ */

estado_t motor_get_state(void)
{
    return state;
}

void motor_set_fcs(int value)
{
    if (value) {
        digitalWrite(FCS_PIN, LOW);
    } else {
        digitalWrite(FCS_PIN, HIGH);
    }
}

void motor_set_fci(int value)
{
    if (value) {
        digitalWrite(FCI_PIN, LOW);
    } else {
        digitalWrite(FCI_PIN, HIGH);
    }
}
