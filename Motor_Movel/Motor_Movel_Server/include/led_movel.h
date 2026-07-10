#ifndef LED_MOVEL_H
#define LED_MOVEL_H

#include <Arduino.h>

/* ────────────────────────────────────────────────────────────
 * Pinagem (conforme main.cpp de teste manual - testado e OK)
 *   GPIO18 -> Segmento 1 (DIN)
 *   GPIO5  -> Segmento 2 (DIN)
 *   GPIO17 -> Segmento 3 (DIN)
 * ──────────────────────────────────────────────────────────── */
#define LED_PIN_SEG1 18
#define LED_PIN_SEG2 5
#define LED_PIN_SEG3 17

/* AJUSTE para a quantidade real de LEDs de cada segmento */
#define LED_POR_SEGMENTO 300
#define LED_BRILHO       100  /* 0-255 */

/* ────────────────────────────────────────────────────────────
 * Protocolo Jellyfish V3 - Família 10 (Interface Visual / LED)
 * Endpoint: POST http://{IP_WROOM}/led_movel
 * JSON:     { "id": 10, "r": 0-255, "g": 0-255, "b": 0-255 }
 *           { "id": 11 | 12 | 13 | 14 | 15 }
 * ──────────────────────────────────────────────────────────── */
#define ID_LED_COR_CUSTOM 10  /* usa r,g,b do payload */
#define ID_LED_VERMELHO   11
#define ID_LED_VERDE      12
#define ID_LED_AZUL       13
#define ID_LED_PRETO      14  /* limpar / apagar */
#define ID_LED_BRANCO     15

/* ── API pública ───────────────────────────────────────────── */

/* Configura a fita (3 segmentos) e apaga como estado inicial. */
void led_movel_setup(void);

/* Executa um ID da Família 10. r/g/b só são usados quando id==10.
 * Retorna true se o ID era conhecido e foi tratado. */
bool led_movel_handle_id(int id, uint8_t r, uint8_t g, uint8_t b);

/* Controle direto de cor, usado internamente e também exposto. */
void led_movel_set_color(uint8_t r, uint8_t g, uint8_t b);

#endif /* LED_MOVEL_H */
