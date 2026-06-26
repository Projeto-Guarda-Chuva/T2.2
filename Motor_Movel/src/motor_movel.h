#ifndef MOTOR_MOVEL_H
#define MOTOR_MOVEL_H

/* ── Pinos (GPIOs reais do ESP_WROOM-32) ───────────────────── */
#define ENABLE_PIN  12   /* R_EN + L_EN do BTS7960 em paralelo  */
#define RPWM        14   /* PWM sentido SUBIR                    */
#define LPWM        27   /* PWM sentido DESCER                   */
#define FCS_PIN     13   /* Fim de curso Superior (NC)           */
#define FCI_PIN     15   /* Fim de curso Inferior (NC)           */

/* ── Parâmetros ────────────────────────────────────────────── */
#define VELOCIDADE  200  /* 0-25000  (~78 % PWM)                  */

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
