#ifndef MOTOR_MOVEL_H
#define MOTOR_MOVEL_H

/* ── Pinos ─────────────────────────────────────────────────── */
#define ENABLE_PIN  D1   /* R_EN + L_EN do BTS7960 em paralelo  */
#define RPWM        D5   /* PWM sentido SUBIR                    */
#define LPWM        D6   /* PWM sentido DESCER                   */
#define FCS_PIN     D7   /* Fim de curso Superior (NC)           */
#define FCI_PIN     D8   /* Fim de curso Inferior (NC)           */

/* ── Parâmetros ────────────────────────────────────────────── */
#define VELOCIDADE  200  /* 0-255  (~78 % PWM)                  */

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
