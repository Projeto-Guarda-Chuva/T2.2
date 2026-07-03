#ifndef MOTOR_MOVEL_H
#define MOTOR_MOVEL_H

/* ── Pinos (GPIOs reais do ESP_WROOM-32) ───────────────────── */
#define R_EN_PIN    12   /* Enable direito (lado RPWM/subir)     */
#define L_EN_PIN    26   /* Enable esquerdo (lado LPWM/descer)   */
#define RPWM        14
#define LPWM        27
#define FCS_PIN     13
#define FCI_PIN     15

/* ── Parâmetros ────────────────────────────────────────────── */
#define VELOCIDADE  200  /* 0-255  (~78 % PWM) */

/* ── Configuração do LEDC (PWM nativo do ESP32) ────────────── */
#define PWM_FREQ      5000  /* Frequência do PWM: 5 kHz */
#define PWM_RES       8     /* Resolução de 8 bits (0-255) */
#define CANAL_RPWM    0     /* Canal LEDC 0 */
#define CANAL_LPWM    1     /* Canal LEDC 1 */

/* ── Estado do atuador ─────────────────────────────────────── */
typedef enum {
    PARADO  = 0,
    SUBINDO = 1,
    DESCENDO = 2
} estado_t;

/* ── Protótipos: controle do motor ─────────────────────────── */
void motor_parar(void);
void motor_subir(void);
void motor_descer(void);

/* ── Protótipos: fins de curso ─────────────────────────────── */
int  fim_superior(void);
int  fim_inferior(void);
void verificar_fins_de_curso(void);

/* ── Protótipos: rede ──────────────────────────────────────── */
void wifi_conectar(void);

/* ── Protótipos: handlers HTTP ─────────────────────────────── */
void handle_status(void);
void handle_command(void);

#endif /* MOTOR_MOVEL_H */