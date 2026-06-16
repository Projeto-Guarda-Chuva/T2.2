#include "core_esp8266_features.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define BOT_A   D1
#define BOT_A_LED D2
#define BOT_B   D3
#define BOT_B_LED D4
#define FA      D5
#define FA_LED  D6
#define FF      D7
#define FF_LED  D8

const char *ssid = "ESP_F85DED";

ESP8266WebServer server(80);

// --- Máquina de estados do portão ---
enum gate_state {
    GATE_STOPPED,
    GATE_OPENING,
    GATE_CLOSING
};
gate_state state = GATE_STOPPED;

// --- Controle de pulso não-bloqueante ---
enum pulse_state {
    PULSE_IDLE,
    PULSE_OPEN_HIGH,   // BOT_A em HIGH, aguardando LOW
    PULSE_OPEN_DONE,   // BOT_A baixou, aguardando limpar FF
    PULSE_CLOSE_HIGH,  // BOT_B em HIGH, aguardando LOW
    PULSE_CLOSE_DONE   // BOT_B baixou, aguardando limpar FA
};
pulse_state pulseState = PULSE_IDLE;
unsigned long pulseStartUs = 0;
const unsigned long PULSE_WIDTH_US = 500;

void connect_WiFi();
void handle_command();
void handle_status();
void stop_gate();
void open_gate();
void close_gate();
void update_pulse(); // <-- novo: chamado no loop()

// -------------------------------------------------------
void setup() {
    Serial.begin(115200);

    pinMode(BOT_A,     OUTPUT);
    pinMode(BOT_B,     OUTPUT);
    pinMode(BOT_A_LED, OUTPUT);
    pinMode(BOT_B_LED, OUTPUT);
    pinMode(FA,        OUTPUT);
    pinMode(FF,        OUTPUT);
    pinMode(FA_LED,    OUTPUT);
    pinMode(FF_LED,    OUTPUT);

    digitalWrite(BOT_A, LOW);
    digitalWrite(BOT_B, LOW);

    WiFi.mode(WIFI_STA);
    connect_WiFi();

    server.on("/status",  HTTP_GET,  handle_status);
    server.on("/command", HTTP_POST, handle_command);
    server.onNotFound([]() {
        server.send(404, "text/plain", "Endpoint não encontrado. Use /status ou /command.");
    });

    server.begin();
    Serial.println("Servidor HTTP iniciado!");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Sinal WiFi perdido, reconectando...");
        connect_WiFi();
    }

    server.handleClient();
    update_pulse(); // verifica se algum pulso precisa ser finalizado
}

// -------------------------------------------------------
// Gerencia os pulsos sem bloquear
void update_pulse() {
    if (pulseState == PULSE_IDLE) return;

    unsigned long now = micros();

    switch (pulseState) {

        case PULSE_OPEN_HIGH:
            if (now - pulseStartUs >= PULSE_WIDTH_US) {
                digitalWrite(BOT_A, LOW);          // termina o pulso
                pulseStartUs = micros();
                pulseState = PULSE_OPEN_DONE;
            }
            break;

        case PULSE_OPEN_DONE:
            // pequena pausa antes de limpar FF (equivale ao delay comentado)
            if (now - pulseStartUs >= PULSE_WIDTH_US) {
                digitalWrite(FF,     LOW);
                digitalWrite(FF_LED, LOW);
                pulseState = PULSE_IDLE;
                Serial.println("O portão está abrindo.");
            }
            break;

        case PULSE_CLOSE_HIGH:
            if (now - pulseStartUs >= PULSE_WIDTH_US) {
                digitalWrite(BOT_B, LOW);          // termina o pulso
                pulseStartUs = micros();
                pulseState = PULSE_CLOSE_DONE;
            }
            break;

        case PULSE_CLOSE_DONE:
            if (now - pulseStartUs >= PULSE_WIDTH_US) {
                digitalWrite(FA,     LOW);
                digitalWrite(FA_LED, LOW);
                pulseState = PULSE_IDLE;
                Serial.println("O portão está fechando.");
            }
            break;

        default:
            break;
    }
}

// -------------------------------------------------------
void handle_command() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Corpo vazio: Use A, F, FA ou FF.");
        return;
    }

    String cmd = server.arg("plain");
    Serial.println("Comando recebido: " + cmd);

    if (cmd == "A") {
        if (state == GATE_STOPPED && digitalRead(FA) == LOW)
            open_gate();
        else if (state == GATE_CLOSING)
            stop_gate();

    } else if (cmd == "F") {
        if (state == GATE_STOPPED && digitalRead(FF) == LOW)
            close_gate();
        else if (state == GATE_OPENING)
            stop_gate();

    } else if (cmd == "FA") {
        if (state == GATE_OPENING || state == GATE_STOPPED) {
            if (state == GATE_OPENING) stop_gate();
            digitalWrite(FA,     HIGH);
            digitalWrite(FA_LED, HIGH);
        }

    } else if (cmd == "FF") {
        if (state == GATE_CLOSING || state == GATE_STOPPED) {
            if (state == GATE_CLOSING) stop_gate();
            digitalWrite(FF,     HIGH);
            digitalWrite(FF_LED, HIGH);
        }

    } else {
        server.send(400, "text/plain", "Comando inválido: Use A, F, FA ou FF.");
        return;
    }

    server.send(200, "text/plain", "Comando recebido.");
}

// -------------------------------------------------------
void handle_status() {
    String json = "{";
    json += "\"rssi\":"   + String(WiFi.RSSI())      + ",";
    json += "\"heap\":"   + String(ESP.getFreeHeap()) + ",";
    json += "\"uptime\":" + String(millis());
    json += "}";
    server.send(200, "application/json", json);
    Serial.println(json);
}

// -------------------------------------------------------
void connect_WiFi() {
    if (WiFi.status() == WL_CONNECTED) return;
    WiFi.begin(ssid);
    Serial.print("Conectando");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500); // aqui é aceitável: só ocorre na inicialização
    }
    Serial.println("\nConectado! IP: " + WiFi.localIP().toString());
}

// -------------------------------------------------------
void stop_gate() {
    digitalWrite(BOT_A, LOW);
    digitalWrite(BOT_B, LOW);
    state = GATE_STOPPED;
    Serial.println("O portão foi parado.");
}

void open_gate() {
    digitalWrite(BOT_B,     LOW);
    digitalWrite(BOT_B_LED, LOW);
    digitalWrite(BOT_A,     HIGH);
    digitalWrite(BOT_A_LED, HIGH);
    // inicia pulso não-bloqueante
    pulseStartUs = micros();
    pulseState   = PULSE_OPEN_HIGH;
    state        = GATE_OPENING;
}

void close_gate() {
    digitalWrite(BOT_A,     LOW);
    digitalWrite(BOT_A_LED, LOW);
    digitalWrite(BOT_B,     HIGH);
    digitalWrite(BOT_B_LED, HIGH);
    // inicia pulso não-bloqueante
    pulseStartUs = micros();
    pulseState   = PULSE_CLOSE_HIGH;
    state        = GATE_CLOSING;
}