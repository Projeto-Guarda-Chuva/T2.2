/* ============================================================
 * ── API REST (ESP32) ────────────────────────────────────────
 *   GET  /status
 *        Resposta JSON: { rssi, heap, uptime, state, fcs, fci }
 *
 *   POST /command   (Content-Type: text/plain)
 *        Corpo  │ Ação
 *        ───────┼──────────────────────────────────────────────
 *        "31"   │ Subir
 *        "32"   │ Descer
 *        "30"   │ Parar
 *        "34"   │ Fim de curso Superior (enviado pela camada superior)
 *        "35"   │ Fim de curso Inferior (enviado pela camada superior)
 *
 * ── Lógica de segurança ─────────────────────────────────────
 *   • Não sobe  se FCS já estiver acionado.
 *   • Não desce se FCI já estiver acionado.
 *   • Inverte sentido somente após parar (protege o BTS7960).
 *   • Fins de curso verificados a cada loop() — independente
 *     de WiFi, o ESP para o motor localmente.
 * ============================================================ */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "motor_movel.h"

/* ── WiFi ──────────────────────────────────────────────────── */
static const char *SSID = "ESP_F85DED";
/* static const char *PASS = "senha";  <- descomente se necessário */

/* ── Servidor HTTP ─────────────────────────────────────────── */
static WebServer server(80);

/* ── Estado global ─────────────────────────────────────────── */
static volatile estado_t state = PARADO;

// --- Definição de Comandos ---
#define CMD_SUBIR "31"
#define CMD_DESCER "32"
#define CMD_PARAR  "30"
#define CMD_FCS    "34"
#define CMD_FCI    "35"

/* ============================================================
 * SETUP
 * ============================================================ */
void setup(void)
{
    Serial.begin(115200);

    /* Saídas */
    pinMode(ENABLE_PIN, OUTPUT);

    /* Configuração do LEDC (PWM nativo do ESP32) */
    ledcSetup(CANAL_RPWM, PWM_FREQ, PWM_RES);
    ledcSetup(CANAL_LPWM, PWM_FREQ, PWM_RES);
    ledcAttachPin(RPWM, CANAL_RPWM);
    ledcAttachPin(LPWM, CANAL_LPWM);

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
    Serial.println("Servidor HTTP do Motor Movel iniciado no ESP32.");
}

/* ============================================================
 * LOOP
 * ============================================================ */
void loop(void)
{
    if (WiFi.status() != WL_CONNECTED) {
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
            "Corpo vazio. Use 31, 32, 30, 34 ou 35.");
        return;
    }

    String cmd = server.arg("plain");
    cmd.trim();

    Serial.print("Comando recebido: ");
    Serial.println(cmd);

    /* ── 31 : Subir ─────────────────────────────────────────── */
    if (cmd == CMD_SUBIR) {
        if (fim_superior()) {
            server.send(200, "text/plain",
                "Ignorado: fim de curso superior ja acionado.");
            return;
        }
        if (state == SUBINDO) {
            server.send(200, "text/plain", "Ja esta subindo.");
            return;
        }
        if (state == DESCENDO) {
            motor_parar();   /* para antes de inverter */
            delay(400);      /* tempo morto de segurança para desaceleração */
        }
        motor_subir();

    /* ── 32 : Descer ────────────────────────────────────────── */
    } else if (cmd == CMD_DESCER) {
        if (fim_inferior()) {
            server.send(200, "text/plain",
                "Ignorado: fim de curso inferior ja acionado.");
            return;
        }
        if (state == DESCENDO) {
            server.send(200, "text/plain", "Ja esta descendo.");
            return;
        }
        if (state == SUBINDO) {
            motor_parar();   /* para antes de inverter */
            delay(400);      /* tempo morto de segurança para desaceleração */
        }
        motor_descer();

    /* ── 33 : Parar ─────────────────────────────────────────── */
    } else if (cmd == CMD_PARAR) {
        motor_parar();

    /* ── 34 : Fim de curso Superior (software) ──────────────── */
    } else if (cmd == CMD_FCS) {
        if (state == SUBINDO)
            motor_parar();
        Serial.println("FCS recebido por software.");

    /* ── 35 : Fim de curso Inferior (software) ──────────────── */
    } else if (cmd == CMD_FCI) {
        if (state == DESCENDO)
            motor_parar();
        Serial.println("FCI recebido por software.");

    /* ── Comando inválido ───────────────────────────────────── */
    } else {
        server.send(400, "text/plain",
            "Comando invalido. Use 31, 32, 30, 34 ou 35.");
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
    ledcWrite(CANAL_RPWM, 0);
    ledcWrite(CANAL_LPWM, 0);
    state = PARADO;
    Serial.println("Motor parado.");
}

void motor_subir(void)
{
    ledcWrite(CANAL_LPWM, 0);
    ledcWrite(CANAL_RPWM, VELOCIDADE);
    state = SUBINDO;
    Serial.println("Motor subindo.");
}

void motor_descer(void)
{
    ledcWrite(CANAL_RPWM, 0);
    ledcWrite(CANAL_LPWM, VELOCIDADE);
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
    static bool ja_conectado = false;

    if (WiFi.status() == WL_CONNECTED) {
        if (!ja_conectado) {
            Serial.println("\nConectado!");
            Serial.print("IP do Motor Movel: ");
            Serial.println(WiFi.localIP());
            ja_conectado = true;
        }
        return;
    }

    ja_conectado = false;

    static unsigned long last_attempt = 0;
    if (millis() - last_attempt > 10000 || last_attempt == 0) {
        Serial.println("\nTentando conectar ao WiFi...");
        WiFi.begin(SSID);
        /* WiFi.begin(SSID, PASS); */
        last_attempt = millis();
    }
}
