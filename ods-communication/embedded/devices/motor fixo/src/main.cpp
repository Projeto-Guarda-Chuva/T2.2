#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Definições de pinos (WeMos D1 Mini utiliza nomenclatura DX)
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
enum gate_state { GATE_STOPPED, GATE_OPENING, GATE_CLOSING };
volatile gate_state state = GATE_STOPPED; // volatile para garantir visibilidade

// --- Controle de pulso não-bloqueante ---
enum pulse_state { PULSE_IDLE, PULSE_OPEN_HIGH, PULSE_OPEN_DONE, PULSE_CLOSE_HIGH, PULSE_CLOSE_DONE };
volatile pulse_state pulseState = PULSE_IDLE; // volatile
volatile unsigned long pulseStartUs = 0;      // volatile
const unsigned long PULSE_WIDTH_US = 500;

// --- Definição de Comandos (Mudar para números futuramente) ---
const String CMD_OPEN  = "A";
const String CMD_CLOSE = "F";
const String CMD_STOP  = "STOP";

// Prototipos
void connect_WiFi();
void handle_command();
void handle_status();
void stop_gate();
void open_gate();
void close_gate();
void update_pulse();

void setup() {
    Serial.begin(115200);

    // Configuração de pinos
    pinMode(BOT_A, OUTPUT); pinMode(BOT_B, OUTPUT);
    pinMode(BOT_A_LED, OUTPUT); pinMode(BOT_B_LED, OUTPUT);
    pinMode(FA, OUTPUT); pinMode(FF, OUTPUT);
    pinMode(FA_LED, OUTPUT); pinMode(FF_LED, OUTPUT);

    digitalWrite(BOT_A, LOW); digitalWrite(BOT_B, LOW);
    digitalWrite(FA, LOW);    digitalWrite(FF, LOW);
    digitalWrite(FA_LED, LOW); digitalWrite(FF_LED, LOW);
    digitalWrite(BOT_A_LED, LOW); digitalWrite(BOT_B_LED, LOW);

    WiFi.mode(WIFI_STA);
    connect_WiFi();

    server.on("/status", HTTP_GET, handle_status);
    server.on("/command", HTTP_POST, handle_command);
    server.onNotFound([]() {
        server.send(404, "text/plain", "Endpoint não encontrado.");
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
    update_pulse();
}

void update_pulse() {
    if (pulseState == PULSE_IDLE) return;

    unsigned long now = micros();

    // Transições de pulso e máquina de estado
    switch (pulseState) {
        case PULSE_OPEN_HIGH:
            if (now - pulseStartUs >= PULSE_WIDTH_US) {
                digitalWrite(BOT_A, LOW);
                pulseStartUs = micros();
                pulseState = PULSE_OPEN_DONE;
            }
            break;
        case PULSE_OPEN_DONE:
            if (now - pulseStartUs >= PULSE_WIDTH_US) {
                digitalWrite(FF, LOW);
                digitalWrite(FF_LED, LOW);
                pulseState = PULSE_IDLE;
                state = GATE_STOPPED; // Transição para parado
                Serial.println("Portão finalizou abertura.");
            }
            break;
        case PULSE_CLOSE_HIGH:
            if (now - pulseStartUs >= PULSE_WIDTH_US) {
                digitalWrite(BOT_B, LOW);
                pulseStartUs = micros();
                pulseState = PULSE_CLOSE_DONE;
            }
            break;
        case PULSE_CLOSE_DONE:
            if (now - pulseStartUs >= PULSE_WIDTH_US) {
                digitalWrite(FA, LOW);
                digitalWrite(FA_LED, LOW);
                pulseState = PULSE_IDLE;
                state = GATE_STOPPED; // Transição para parado
                Serial.println("Portão finalizou fechamento.");
            }
            break;
        default: break;
    }
}

void stop_gate() {
    digitalWrite(BOT_A, LOW);
    digitalWrite(BOT_B, LOW);
    digitalWrite(BOT_A_LED, LOW);
    digitalWrite(BOT_B_LED, LOW);
    state = GATE_STOPPED;
    pulseState = PULSE_IDLE;
    Serial.println("O portão foi parado.");
}

void open_gate() {
    digitalWrite(BOT_B,     LOW);
    digitalWrite(BOT_B_LED, LOW);
    digitalWrite(BOT_A,     HIGH);
    digitalWrite(BOT_A_LED, HIGH);
    
    digitalWrite(FF,     HIGH);
    digitalWrite(FF_LED, HIGH);
    
    pulseStartUs = micros();
    pulseState   = PULSE_OPEN_HIGH;
    state        = GATE_OPENING;
    Serial.println("Abrindo portão...");
}

void close_gate() {
    digitalWrite(BOT_A,     LOW);
    digitalWrite(BOT_A_LED, LOW);
    digitalWrite(BOT_B,     HIGH);
    digitalWrite(BOT_B_LED, HIGH);
    
    digitalWrite(FA,     HIGH);
    digitalWrite(FA_LED, HIGH);
    
    pulseStartUs = micros();
    pulseState   = PULSE_CLOSE_HIGH;
    state        = GATE_CLOSING;
    Serial.println("Fechando portão...");
}

void handle_command() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Corpo vazio.");
        return;
    }
    String cmd = server.arg("plain");

    // Garantir atomicidade nas mudanças de estado
    noInterrupts();
    if (cmd == CMD_OPEN) {
        if (state == GATE_STOPPED) open_gate();
    } else if (cmd == CMD_CLOSE) {
        if (state == GATE_STOPPED) close_gate();
    } else if (cmd == CMD_STOP) {
        stop_gate();
    } else {
        interrupts();
        server.send(400, "text/plain", "Comando inválido.");
        return;
    }
    interrupts();

    server.send(200, "text/plain", "Comando aceito.");
}

void handle_status() {
    String json = "{";
    json += "\"rssi\":"   + String(WiFi.RSSI())      + ",";
    json += "\"heap\":"   + String(ESP.getFreeHeap()) + ",";
    json += "\"uptime\":" + String(millis()) + ",";
    json += "\"state\":"  + String(state);
    json += "}";
    server.send(200, "application/json", json);
}

void connect_WiFi() {
    if (WiFi.status() == WL_CONNECTED) return;
    WiFi.begin(ssid);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}
