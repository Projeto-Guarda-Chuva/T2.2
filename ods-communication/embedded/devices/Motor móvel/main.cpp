/* ============================================================
 * ── API REST ────────────────────────────────────────────────
 *   GET  /status
 *        Resposta JSON: { rssi, heap, uptime, state, fcs, fci }
 *
 *   POST /command   (Content-Type: text/plain)
 *        Corpo  │ Ação
 *        ───────┼──────────────────────────────────────────────
 *        "S"    │ Subir
 *        "D"    │ Descer
 *        "P"    │ Parar
 *        "FCS"  │ Fim de curso Superior (enviado pela camada superior)
 *        "FCI"  │ Fim de curso Inferior (enviado pela camada superior)
 *
 * ── Lógica de segurança ─────────────────────────────────────
 *   • Não sobe  se FCS já estiver acionado.
 *   • Não desce se FCI já estiver acionado.
 *   • Inverte sentido somente após parar (protege o BTS7960).
 *   • Fins de curso verificados a cada loop() — independente
 *     de WiFi, o ESP para o motor localmente.
 * ============================================================ */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "motor_movel.h"

/* ── WiFi ──────────────────────────────────────────────────── */
static const char *SSID = "ESP_F85DED";
/* static const char *PASS = "senha";  <- descomente se necessário */

/* ── Servidor HTTP ─────────────────────────────────────────── */
static ESP8266WebServer server(80);

/* ── Estado global ─────────────────────────────────────────── */
static estado_t state = PARADO;

/* ============================================================
 * SETUP
 * ============================================================ */
void setup(void)
{
    Serial.begin(115200);

    /* Saídas */
    pinMode(ENABLE_PIN, OUTPUT);
    pinMode(RPWM,       OUTPUT);
    pinMode(LPWM,       OUTPUT);

    /* Entradas com pull-up (fins de curso NC) */
    pinMode(FCS_PIN, INPUT_PULLUP);
    pinMode(FCI_PIN, INPUT_PULLUP);

    /* Estado inicial seguro */
    motor_parar();
    digitalWrite(ENABLE_PIN, HIGH);   /* habilita o BTS7960 */

    WiFi.mode(WIFI_STA);
    wifi_conectar();

    server.on("/status",  HTTP_GET,  handle_status);
    server.on("/command", HTTP_POST, handle_command);
    server.onNotFound([]() {
        server.send(404, "text/plain",
            "Endpoint nao encontrado. Use /status ou /command.");
    });

    server.begin();
    Serial.println("Servidor HTTP do Motor Movel iniciado.");
}

/* ============================================================
 * LOOP
 * ============================================================ */
void loop(void)
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi perdido, reconectando...");
        wifi_conectar();
    }

    verificar_fins_de_curso();   /* proteção por hardware */
    server.handleClient();
}

/* ============================================================
 * HANDLERS HTTP
 * ============================================================ */

void handle_command(void)
{
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain",
            "Corpo vazio. Use S, D, P, FCS ou FCI.");
        return;
    }

    String cmd = server.arg("plain");
    cmd.trim();

    Serial.print("Comando recebido: ");
    Serial.println(cmd);

    /* ── S : Subir ──────────────────────────────────────────── */
    if (cmd == "S") {
        if (fim_superior()) {
            server.send(200, "text/plain",
                "Ignorado: fim de curso superior ja acionado.");
            return;
        }
        if (state == SUBINDO) {
            server.send(200, "text/plain", "Ja esta subindo.");
            return;
        }
        if (state == DESCENDO)
            motor_parar();   /* para antes de inverter */
        motor_subir();

    /* ── D : Descer ─────────────────────────────────────────── */
    } else if (cmd == "D") {
        if (fim_inferior()) {
            server.send(200, "text/plain",
                "Ignorado: fim de curso inferior ja acionado.");
            return;
        }
        if (state == DESCENDO) {
            server.send(200, "text/plain", "Ja esta descendo.");
            return;
        }
        if (state == SUBINDO)
            motor_parar();   /* para antes de inverter */
        motor_descer();

    /* ── P : Parar ──────────────────────────────────────────── */
    } else if (cmd == "P") {
        motor_parar();

    /* ── FCS : Fim de curso Superior (software) ─────────────── */
    } else if (cmd == "FCS") {
        if (state == SUBINDO)
            motor_parar();
        Serial.println("FCS recebido por software.");

    /* ── FCI : Fim de curso Inferior (software) ─────────────── */
    } else if (cmd == "FCI") {
        if (state == DESCENDO)
            motor_parar();
        Serial.println("FCI recebido por software.");

    /* ── Comando inválido ───────────────────────────────────── */
    } else {
        server.send(400, "text/plain",
            "Comando invalido. Use S, D, P, FCS ou FCI.");
        return;
    }

    server.send(200, "text/plain", "Comando executado.");
}

void handle_status(void)
{
    char json[160];
    const char *estado_str;

    switch (state) {
        case SUBINDO:  estado_str = "subindo";  break;
        case DESCENDO: estado_str = "descendo"; break;
        default:       estado_str = "parado";   break;
    }

    snprintf(json, sizeof(json),
        "{\"rssi\":%d,\"heap\":%u,\"uptime\":%lu,"
        "\"state\":\"%s\",\"fcs\":%d,\"fci\":%d}",
        (int)WiFi.RSSI(),
        (unsigned)ESP.getFreeHeap(),
        (unsigned long)millis(),
        estado_str,
        fim_superior(),
        fim_inferior()
    );

    server.send(200, "application/json", json);
    Serial.println(json);
}

/* ============================================================
 * CONTROLE DO MOTOR
 * ============================================================ */

void motor_parar(void)
{
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
    state = PARADO;
    Serial.println("Motor parado.");
}

void motor_subir(void)
{
    analogWrite(LPWM, 0);
    analogWrite(RPWM, VELOCIDADE);
    state = SUBINDO;
    Serial.println("Motor subindo.");
}

void motor_descer(void)
{
    analogWrite(RPWM, 0);
    analogWrite(LPWM, VELOCIDADE);
    state = DESCENDO;
    Serial.println("Motor descendo.");
}

/* ============================================================
 * FINS DE CURSO
 * ============================================================ */

int fim_superior(void)
{
    return digitalRead(FCS_PIN) == LOW;   /* NC: LOW = acionado */
}

int fim_inferior(void)
{
    return digitalRead(FCI_PIN) == LOW;
}

/* Chamado a cada loop() — para o motor por hardware,
   independente do estado da comunicação WiFi.            */
void verificar_fins_de_curso(void)
{
    if (state == SUBINDO && fim_superior()) {
        motor_parar();
        Serial.println("FCS acionado por hardware.");
    }
    if (state == DESCENDO && fim_inferior()) {
        motor_parar();
        Serial.println("FCI acionado por hardware.");
    }
}

/* ============================================================
 * WiFi
 * ============================================================ */

void wifi_conectar(void)
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    WiFi.begin(SSID);
    /* WiFi.begin(SSID, PASS); */

    Serial.print("Conectando ao WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nConectado!");
    Serial.print("IP do Motor Movel: ");
    Serial.println(WiFi.localIP());
}
