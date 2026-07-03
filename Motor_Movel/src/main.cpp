#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "motor_movel.h"

/* ── WiFi ──────────────────────────────────────────────────── */
static const char *SSID = "SEU_SSID_AQUI";
static const char *PASS = "SUA_SENHA_AQUI";

/* ── Servidor HTTP ─────────────────────────────────────────── */
static WebServer server(80);

/* ── Estado global ─────────────────────────────────────────── */
static volatile estado_t state = PARADO;

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
    delay(2000);
    Serial.println("ESP32 iniciado!");

    pinMode(R_EN_PIN, OUTPUT);
    pinMode(L_EN_PIN, OUTPUT);

    ledcSetup(CANAL_RPWM, PWM_FREQ, PWM_RES);
    ledcSetup(CANAL_LPWM, PWM_FREQ, PWM_RES);
    ledcAttachPin(RPWM, CANAL_RPWM);
    ledcAttachPin(LPWM, CANAL_LPWM);

    pinMode(FCS_PIN, INPUT_PULLUP);
    pinMode(FCI_PIN, INPUT_PULLUP);

    motor_parar();   /* estado inicial seguro */

    WiFi.mode(WIFI_STA);
    wifi_conectar();

    server.on("/status",  HTTP_GET,  handle_status);
    server.on("/command", HTTP_POST, handle_command);
    server.onNotFound([]() {
        server.send(404, "text/plain", "Endpoint nao encontrado. Use /status ou /command.");
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

    verificar_fins_de_curso();
    server.handleClient();
}

/* ============================================================
 * HANDLERS HTTP
 * ============================================================ */
void handle_command(void)
{
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Corpo vazio. Use 31, 32, 30, 34 ou 35.");
        return;
    }

    String cmd = server.arg("plain");
    cmd.trim();

    Serial.print("Bruto recebido: ");
    Serial.println(cmd);

    if (cmd.startsWith("{")) {
        String num_apenas = "";
        for (size_t i = 0; i < cmd.length(); i++) {
            if (isDigit(cmd[i])) num_apenas += cmd[i];
        }
        cmd = num_apenas;
    }

    Serial.print("Comando processado: ");
    Serial.println(cmd);

    if (cmd == CMD_SUBIR) {
        if (fim_superior()) {
            server.send(200, "text/plain", "Ignorado: fim de curso superior ja acionado.");
            return;
        }
        if (state == SUBINDO) {
            server.send(200, "text/plain", "Ja esta subindo.");
            return;
        }
        if (state == DESCENDO) {
            motor_parar();
            delay(400);
        }
        motor_subir();

    } else if (cmd == CMD_DESCER) {
        if (fim_inferior()) {
            server.send(200, "text/plain", "Ignorado: fim de curso inferior ja acionado.");
            return;
        }
        if (state == DESCENDO) {
            server.send(200, "text/plain", "Ja esta descendo.");
            return;
        }
        if (state == SUBINDO) {
            motor_parar();
            delay(400);
        }
        motor_descer();

    } else if (cmd == CMD_PARAR) {
        motor_parar();

    } else if (cmd == CMD_FCS) {
        if (state == SUBINDO) motor_parar();
        Serial.println("FCS recebido por software.");

    } else if (cmd == CMD_FCI) {
        if (state == DESCENDO) motor_parar();
        Serial.println("FCI recebido por software.");

    } else {
        server.send(400, "text/plain", "Comando invalido. Use 31, 32, 30, 34 ou 35.");
        return;
    }

    server.send(200, "text/plain", "Comando executado.");
}

void handle_status(void)
{
    server.sendHeader("Access-Control-Allow-Origin", "*");

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
        (int)WiFi.RSSI(), (unsigned)ESP.getFreeHeap(), (unsigned long)millis(),
        estado_str, fim_superior(), fim_inferior()
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
    digitalWrite(R_EN_PIN, LOW);
    digitalWrite(L_EN_PIN, LOW);
    state = PARADO;
    Serial.println("Motor parado.");
}

void motor_subir(void)
{
    digitalWrite(R_EN_PIN, HIGH);
    digitalWrite(L_EN_PIN, HIGH);
    ledcWrite(CANAL_LPWM, 0);
    ledcWrite(CANAL_RPWM, VELOCIDADE);
    state = SUBINDO;
    Serial.println("Motor subindo.");
}

void motor_descer(void)
{
    digitalWrite(R_EN_PIN, HIGH);
    digitalWrite(L_EN_PIN, HIGH);
    ledcWrite(CANAL_RPWM, 0);
    ledcWrite(CANAL_LPWM, VELOCIDADE);
    state = DESCENDO;
    Serial.println("Motor descendo.");
}

/* ============================================================
 * FINS DE CURSO
 * ============================================================ */
int fim_superior(void) { return digitalRead(FCS_PIN) == LOW; }
int fim_inferior(void) { return digitalRead(FCI_PIN) == LOW; }

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
    if (WiFi.status() == WL_CONNECTED) return;

    Serial.println("\nTentando conectar ao WiFi...");
    WiFi.begin(SSID, PASS);

    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
        delay(500);
        Serial.print(".");
        tentativas++;
        verificar_fins_de_curso();
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado com sucesso!");
        Serial.print("IP do Motor Movel: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFalha ao conectar. Tentando novamente em segundo plano...");
    }
}