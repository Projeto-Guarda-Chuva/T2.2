#ifndef MOTOR_MOVEL_H
#define MOTOR_MOVEL_H

#include <Arduino.h>

/* ────────────────────────────────────────────────────────────
 * Pinagem (L298N)
 *   D23 -> ENA (PWM - velocidade)
 *   D22 -> IN1 (direcao)
 *   D21 -> IN2 (direcao)
 * ──────────────────────────────────────────────────────────── */
#define PIN_ENA 23
#define PIN_IN1 22
#define PIN_IN2 21

/* ── PWM (LEDC) ────────────────────────────────────────────── */
#define PWM_FREQ       20000  /* 20 kHz, acima do audível */
#define PWM_RESOLUTION 8      /* 0-255 */
#define CH_ENA         0

/* ── Velocidade padrão (0-255) ────────────────────────────── */
#define VELOCIDADE_PADRAO 200

/* ────────────────────────────────────────────────────────────
 * Protocolo Jellyfish V3 - Família 30 (Motor Móvel)
 * Endpoint: POST http://{IP_WROOM}/motor_movel
 * JSON:     { "id": 30 | 31 | 32 }
 * ──────────────────────────────────────────────────────────── */
#define ID_MOTOR_PARAR   30  /* Ponte H: IN1=0, IN2=0 */
#define ID_MOTOR_SUBIR   31  /* Ponte H: IN1=1, IN2=0 */
#define ID_MOTOR_DESCER  32  /* Ponte H: IN1=0, IN2=1 */

typedef enum {
    MOTOR_PARADO   = 0,
    MOTOR_SUBINDO  = 1,
    MOTOR_DESCENDO = 2
} motor_estado_t;

/* ── API pública ───────────────────────────────────────────── */

/* Configura pinos e PWM. Chamar uma vez no setup(). */
void motor_movel_setup(void);

/* Executa um ID recebido pelo protocolo (30/31/32).
 * Retorna true se o ID era conhecido e foi tratado. */
bool motor_movel_handle_id(int id);

/* Controle direto (usado internamente e também exposto,
 * caso queira chamar fora do fluxo de comando). */
void motor_parar(void);
void motor_subir(void);
void motor_descer(void);

/* Consulta de estado (útil para endpoint de status, se quiser). */
motor_estado_t motor_movel_get_estado(void);
const char *motor_movel_get_estado_str(void);

#endif /* MOTOR_MOVEL_H */