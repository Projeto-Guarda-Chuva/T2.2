/*
  Motor Móvel - ESP32 WROOM
  Servidor HTTP seguindo o Protocolo de IDs V3.0 - Jellyfish (Família 30 + 10)

  Endpoints:
    POST http://{IP_WROOM}/motor_movel
      { "id": 30 }  -> PARAR
      { "id": 31 }  -> SUBIR
      { "id": 32 }  -> DESCER

    POST http://{IP_WROOM}/led_movel
      { "id": 10, "r":0-255, "g":0-255, "b":0-255 }  -> cor customizada
      { "id": 11 } vermelho | 12 verde | 13 azul | 14 preto | 15 branco
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include "../include/motor_movel.h"
#include "../include/led_movel.h"
#include "../include/json_utils.h"

/* ── WiFi ──────────────────────────────────────────────────── */
static const char *SSID = "ods-pc";
static const char *PASS = "87654321";

/* ── mDNS: a placa fica acessivel em http://motormovel.local ── */
static const char *MDNS_HOSTNAME = "motormovel";
static bool mdnsIniciado = false;

/* ── Servidor HTTP ─────────────────────────────────────────── */
static WebServer server(80);

/* ============================================================
 * Grava o IP atual em /ip.json no LittleFS (memoria flash da
 * propria placa). Assim da pra conferir o IP sem precisar abrir
 * o Monitor Serial (que estava derrubando o WiFi).
 * ============================================================ */
static void salvar_ip_json(void)
{
    if (!LittleFS.begin(true)) { /* true = formata se nao existir FS valido */
        Serial.println("Erro: falha ao montar LittleFS.");
        return;
    }

    String json = "{\"ip\":\"" + WiFi.localIP().toString() +
                  "\",\"rssi\":" + String(WiFi.RSSI()) +
                  ",\"uptime_ms\":" + String(millis()) + "}";

    File f = LittleFS.open("/ip.json", "w");
    if (!f) {
        Serial.println("Erro: falha ao abrir /ip.json para escrita.");
        return;
    }
    f.print(json);
    f.close();

    Serial.print("IP salvo em /ip.json: ");
    Serial.println(json);
}

/* ============================================================
 * HANDLER: GET /ip -> devolve o conteudo de /ip.json
 * ============================================================ */
static void handle_ip(void)
{
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (!LittleFS.exists("/ip.json")) {
        server.send(404, "application/json", "{\"erro\":\"ip.json ainda nao foi gravado\"}");
        return;
    }

    File f = LittleFS.open("/ip.json", "r");
    if (!f) {
        server.send(500, "application/json", "{\"erro\":\"falha ao ler ip.json\"}");
        return;
    }

    server.streamFile(f, "application/json");
    f.close();
}

/* ============================================================
 * WiFi
 * ============================================================ */
static void wifi_conectar(void)
{
    if (WiFi.status() == WL_CONNECTED) return;

    Serial.println("\nTentando conectar ao WiFi...");
    WiFi.begin(SSID, PASS);

    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
        delay(500);
        Serial.print(".");
        tentativas++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado com sucesso!");
        Serial.print("IP_WROOM (Motor Movel / LED Movel): ");
        Serial.println(WiFi.localIP());
        salvar_ip_json();

        if (!mdnsIniciado) {
            if (MDNS.begin(MDNS_HOSTNAME)) {
                MDNS.addService("http", "tcp", 80);
                mdnsIniciado = true;
                Serial.print("mDNS ativo: http://");
                Serial.print(MDNS_HOSTNAME);
                Serial.println(".local");
            } else {
                Serial.println("Erro: falha ao iniciar mDNS.");
            }
        }
    } else {
        Serial.println("\nFalha ao conectar. Tentando novamente em segundo plano...");
    }
}

/* ============================================================
 * HANDLER: POST /motor_movel
 * ============================================================ */
static void handle_motor_movel(void)
{
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Corpo vazio. Envie JSON, ex: {\"id\":31}");
        return;
    }

    String body = server.arg("plain");
    Serial.print("Recebido em /motor_movel: ");
    Serial.println(body);

    int id;
    if (!json_extrair_campo_int(body, "id", id)) {
        server.send(400, "text/plain", "JSON invalido ou campo \"id\" ausente.");
        return;
    }

    Serial.print("ID processado: ");
    Serial.println(id);

    if (!motor_movel_handle_id(id)) {
        server.send(400, "text/plain", "ID desconhecido para Motor Movel (use 30, 31 ou 32).");
        return;
    }

    server.send(200, "text/plain", "Comando executado.");
}

/* ============================================================
 * HANDLER: POST /led_movel
 * ============================================================ */
static void handle_led_movel(void)
{
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Corpo vazio. Envie JSON, ex: {\"id\":11}");
        return;
    }

    String body = server.arg("plain");
    Serial.print("Recebido em /led_movel: ");
    Serial.println(body);

    int id;
    if (!json_extrair_campo_int(body, "id", id)) {
        server.send(400, "text/plain", "JSON invalido ou campo \"id\" ausente.");
        return;
    }

    int r = 0, g = 0, b = 0;
    if (id == ID_LED_COR_CUSTOM) {
        /* r/g/b sao opcionais no parser; se ausentes, ficam 0 */
        json_extrair_campo_int(body, "r", r);
        json_extrair_campo_int(body, "g", g);
        json_extrair_campo_int(body, "b", b);
    }

    Serial.print("ID processado: ");
    Serial.print(id);
    if (id == ID_LED_COR_CUSTOM) {
        Serial.print(" | R="); Serial.print(r);
        Serial.print(" G=");   Serial.print(g);
        Serial.print(" B=");   Serial.println(b);
    } else {
        Serial.println();
    }

    if (!led_movel_handle_id(id, (uint8_t)r, (uint8_t)g, (uint8_t)b)) {
        server.send(400, "text/plain", "ID desconhecido para LED Movel (use 10-15).");
        return;
    }

    server.send(200, "text/plain", "Comando executado.");
}

/* ============================================================
 * SETUP
 * ============================================================ */
void setup(void)
{
    Serial.begin(115200);
    delay(2000);
    Serial.println("ESP32 iniciado - Motor Movel + LED Movel (Jellyfish V3)");

    motor_movel_setup();
    led_movel_setup();

    WiFi.mode(WIFI_STA);
    wifi_conectar();

    server.on("/motor_movel", HTTP_POST, handle_motor_movel);
    server.on("/led_movel",   HTTP_POST, handle_led_movel);
    server.on("/ip",          HTTP_GET,  handle_ip);
    server.onNotFound([]() {
        server.send(404, "text/plain", "Endpoint nao encontrado. Use POST /motor_movel ou /led_movel.");
    });

    server.begin();
    Serial.println("Servidor HTTP (Motor Movel + LED Movel) iniciado.");
}

/* ============================================================
 * LOOP
 * ============================================================ */
void loop(void)
{
    if (WiFi.status() != WL_CONNECTED) {
        wifi_conectar();
    }

    server.handleClient();
}