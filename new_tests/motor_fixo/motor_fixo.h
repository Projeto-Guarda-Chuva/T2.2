#ifndef MOTOR_FIXO_H
#define MOTOR_FIXO_H

#include "mocks/fake_esp8266.h"

typedef int gate_state_t;
typedef int pulse_state_t;

/* ── Estados ────────────────────────────────────────── */
#define GATE_STOPPED  0
#define GATE_OPENING  1
#define GATE_CLOSING  2

#define PULSE_IDLE        0
#define PULSE_OPEN_HIGH   1
#define PULSE_OPEN_DONE   2
#define PULSE_CLOSE_HIGH  3
#define PULSE_CLOSE_DONE  4

#define PULSE_WIDTH_US 500

/* ── Protótipos: inicialização ─────────────────────── */
void gate_init(void);

/* ── Protótipos: controle ──────────────────────────── */
void gate_stop(void);
void gate_open(void);
void gate_close(void);
void gate_update_pulse(void);

/* ── Protótipos: estado ────────────────────────────── */
gate_state_t gate_get_state(void);
pulse_state_t gate_get_pulse_state(void);
int gate_get_pin_state(int pin);

#endif /* MOTOR_FIXO_H */
