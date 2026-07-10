#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <Arduino.h>

/*
 * Extrai o valor inteiro de um campo qualquer de um JSON simples,
 * ex: json_extrair_campo_int(body, "r", out) acha "r": 128 dentro
 * do corpo e devolve 128 em out.
 *
 * Reutilizavel para "id", "r", "g", "b", "volume", etc, sem
 * depender de uma lib JSON completa (ArduinoJson) para payloads
 * simples de 1 nivel como os do protocolo Jellyfish.
 *
 * Retorna true se o campo foi encontrado e tinha um numero valido.
 */
bool json_extrair_campo_int(const String &body, const char *campo, int &out_valor);

#endif /* JSON_UTILS_H */
