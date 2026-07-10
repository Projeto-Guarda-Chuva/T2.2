#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * led_sync: cliente HTTP que replica o comando de cor (Familia 10)
 * para o endpoint /led_movel na placa Wroom, onde o LED fisico
 * esta conectado (a placa da tela nao tem GPIO livre pra isso).
 *
 * Nao-bloqueante: led_sync_send_color() so enfileira o comando e
 * retorna na hora. Uma task dedicada consome a fila e faz o POST
 * em segundo plano, sem travar quem chamou (ex: o handler HTTP
 * do /tela).
 *
 * A fila tem tamanho 1 com xQueueOverwrite: se chegar uma cor nova
 * enquanto a task ainda esta enviando a anterior, so a mais recente
 * e mandada em seguida (nao acumula comandos antigos/obsoletos).
 */

/* Cria a fila e sobe a task consumidora. Chamar UMA VEZ no boot,
 * depois que o Wi-Fi conectar (ex: em app_main, apos wifi_init_simple). */
void led_sync_init(void);

/* Enfileira o comando de cor para ser enviado a Wroom.
 * Retorna imediatamente (nao bloqueia). */
void led_sync_send_color(int id, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif
