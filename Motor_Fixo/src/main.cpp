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

const char *ssid = "ods-pc";
const char *pass = "87654321";

ESP8266WebServer server(80);

// --- Máquina de estados do portão ---
enum gate_state { GATE_STOPPED, GATE_OPENING, GATE_CLOSING };
volatile gate_state state = GATE_STOPPED; // volatile para garantir visibilidade

// --- Controle de pulso não-bloqueante ---
enum pulse_state { PULSE_IDLE, PULSE_OPEN_HIGH, PULSE_OPEN_DONE, PULSE_CLOSE_HIGH, PULSE_CLOSE_DONE, PULSE_STOP_HIGH, PULSE_STOP_DONE };
volatile pulse_state pulseState = PULSE_IDLE; // volatile
volatile unsigned long pulseStartUs = 0;      // volatile
const unsigned long PULSE_WIDTH_US = 200000;

// --- Definição de Comandos (Protocolo Jellyfish V3 - Familia 20) ---
#define ID_MOTOR_FIXO_PARAR  20
#define ID_MOTOR_FIXO_ABRIR  21
#define ID_MOTOR_FIXO_FECHAR 22

// --- Debug periódico (não-bloqueante) ---
const unsigned long DEBUG_INTERVAL_MS = 5000; // a cada 5s
unsigned long lastDebugMs = 0;

// Prototipos
void connect_WiFi();
void handle_command();
void handle_status();
void stop_gate();
void open_gate();
void close_gate();
void update_pulse();
bool extrair_id(const String &body, int &out_id);
void print_debug_status();
const char *gate_state_str(gate_state s);

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=== Motor Fixo - boot ===");

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
    server.on("/motor_fixo", HTTP_POST, handle_command);
    server.onNotFound([]() {
        server.send(404, "text/plain", "Endpoint não encontrado.");
    });
    
    server.begin();
    Serial.println("Servidor HTTP iniciado!");
    Serial.print("Endereço MAC: ");
    Serial.println(WiFi.macAddress());

    print_debug_status();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Sinal WiFi perdido, reconectando...");
        connect_WiFi();
    }
    server.handleClient();
    update_pulse();

    // Debug periódico sem travar o loop (sem delay)
    if (millis() - lastDebugMs >= DEBUG_INTERVAL_MS) {
        lastDebugMs = millis();
        print_debug_status();
    }
}

// --- Debug: status de WiFi/IP/estado, chamado em loop ---
void print_debug_status() {
    Serial.print("[DEBUG] WiFi: ");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("CONECTADO | IP: ");
        Serial.print(WiFi.localIP());
        Serial.print(" | RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.print(" dBm");
    } else {
        Serial.print("DESCONECTADO (status=");
        Serial.print(WiFi.status());
        Serial.print(")");
    }
    Serial.print(" | Portao: ");
    Serial.print(gate_state_str(state));
    Serial.print(" | Heap livre: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
}

const char *gate_state_str(gate_state s) {
    switch (s) {
        case GATE_OPENING: return "ABRINDO";
        case GATE_CLOSING: return "FECHANDO";
        default:           return "PARADO";
    }
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
        case PULSE_STOP_HIGH:
            if (now - pulseStartUs >= PULSE_WIDTH_US) {
                digitalWrite(FA, LOW);
                digitalWrite(FF, LOW);
                digitalWrite(FA_LED, LOW);
                digitalWrite(FF_LED, LOW);
                pulseStartUs = micros();
                pulseState = PULSE_STOP_DONE;
            }
            break;
        case PULSE_STOP_DONE:
            if (now - pulseStartUs >= PULSE_WIDTH_US) {
                pulseState = PULSE_IDLE;
                state = GATE_STOPPED;
                Serial.println("Portão finalizou stop.");
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

    digitalWrite(FA, HIGH);
    digitalWrite(FF, HIGH);
    digitalWrite(FA_LED, HIGH);
    digitalWrite(FF_LED, HIGH);
    
    pulseStartUs = micros();
    pulseState = PULSE_STOP_HIGH;
    state = GATE_STOPPED;
    Serial.println("O portão foi parado.");
}

void open_gate() {
    digitalWrite(BOT_B, LOW);
    digitalWrite(BOT_B_LED, LOW);
    digitalWrite(BOT_A, HIGH);
    digitalWrite(BOT_A_LED, HIGH);
    
    digitalWrite(FF,     HIGH);
    digitalWrite(FF_LED, HIGH);
    
    pulseStartUs = micros();
    pulseState   = PULSE_OPEN_HIGH;
    state        = GATE_OPENING;
    Serial.println("Abrindo portão...");
}

void close_gate() {
    digitalWrite(BOT_A, LOW);
    digitalWrite(BOT_A_LED, LOW);
    digitalWrite(BOT_B, HIGH);
    digitalWrite(BOT_B_LED, HIGH);
    
    digitalWrite(FA, HIGH);
    digitalWrite(FA_LED, HIGH);
    
    pulseStartUs = micros();
    pulseState   = PULSE_CLOSE_HIGH;
    state        = GATE_CLOSING;
    Serial.println("Fechando portão...");
}

// Extrai o valor inteiro do campo "id" de um corpo JSON simples,
// do tipo { "id": 21 }. A familia 20 (Motor Fixo) nao usa parametros
// extras, entao nao precisamos de uma lib JSON completa aqui.
bool extrair_id(const String &body, int &out_id) {
    int pos = body.indexOf("\"id\"");
    if (pos < 0) return false;

    int dois_pontos = body.indexOf(':', pos);
    if (dois_pontos < 0) return false;

    int i = dois_pontos + 1;
    while (i < (int)body.length() && isspace(body[i])) i++;

    bool negativo = false;
    if (i < (int)body.length() && body[i] == '-') {
        negativo = true;
        i++;
    }

    String numero = "";
    while (i < (int)body.length() && isDigit(body[i])) {
        numero += body[i];
        i++;
    }

    if (numero.length() == 0) return false;

    out_id = numero.toInt() * (negativo ? -1 : 1);
    return true;
}

void handle_command() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Corpo vazio. Envie JSON, ex: {\"id\":21}");
        return;
    }
    String body = server.arg("plain");
    Serial.print("[HTTP] Recebido em /motor_fixo: ");
    Serial.println(body);

    int id;
    if (!extrair_id(body, id)) {
        Serial.println("[HTTP] Erro: JSON invalido ou campo \"id\" ausente.");
        server.send(400, "text/plain", "JSON invalido ou campo \"id\" ausente.");
        return;
    }

    Serial.print("[HTTP] ID processado: ");
    Serial.println(id);

    // Garantir atomicidade nas mudanças de estado
    noInterrupts();
    if (id == ID_MOTOR_FIXO_ABRIR) {
        if (state == GATE_STOPPED) open_gate();
    } else if (id == ID_MOTOR_FIXO_FECHAR) {
        if (state == GATE_STOPPED) close_gate();
    } else if (id == ID_MOTOR_FIXO_PARAR) {
        stop_gate();
    } else {
        interrupts();
        Serial.println("[HTTP] Erro: ID desconhecido.");
        server.send(400, "text/plain", "ID desconhecido para Motor Fixo (use 20, 21 ou 22).");
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
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Ja conectado.");
        return;
    }

    Serial.print("Conectando na rede: ");
    Serial.println(ssid);

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi conectado com sucesso!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}