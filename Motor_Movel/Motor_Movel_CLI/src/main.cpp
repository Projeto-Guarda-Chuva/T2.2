/*
  Controle de motor linear via ponte H L298N
  ESP32 Devkit - Projeto PlatformIO

  Pinagem (L298N):
    D23 -> ENA (PWM - velocidade)
    D22 -> IN1 (direcao)
    D21 -> IN2 (direcao)
    GND -> GND
    VCC (motor) -> alimentacao do motor (ex: 12V)
    +5V do L298N -> pode alimentar a logica (ou usar jumper interno)

  Comandos pelo Monitor Serial (baud 115200):
    a -> ABRIR   (motor gira em um sentido)
    f -> FECHAR  (motor gira no sentido contrario)
    s -> PARAR
    + -> aumenta velocidade
    - -> diminui velocidade
*/

#include <Arduino.h>

// ---------- Pinos ----------
constexpr int PIN_ENA = 23; // PWM - velocidade
constexpr int PIN_IN1 = 22; // direcao
constexpr int PIN_IN2 = 21; // direcao

// ---------- PWM (LEDC) ----------
constexpr int PWM_FREQ       = 20000;  // 20kHz, acima do audivel
constexpr int PWM_RESOLUTION = 8;      // 0-255
constexpr int CH_ENA = 0;

int velocidade = 255; // 0 a 255, valor inicial

void parar() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  ledcWrite(CH_ENA, 0);
}

void abrir() {
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  ledcWrite(CH_ENA, velocidade);
}

void fechar() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, HIGH);
  ledcWrite(CH_ENA, velocidade);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);

  ledcSetup(CH_ENA, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PIN_ENA, CH_ENA);

  parar();

  Serial.println("Pronto. Comandos: a=abrir, f=fechar, s=parar, +=+veloc, -=-veloc");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    switch (c) {
      case 'a':
      case 'A':
        abrir();
        Serial.println("ABRINDO");
        break;

      case 'f':
      case 'F':
        fechar();
        Serial.println("FECHANDO");
        break;

      case 's':
      case 'S':
        parar();
        Serial.println("PARADO");
        break;

      case '+':
        velocidade = min(255, velocidade + 15);
        Serial.print("Velocidade: ");
        Serial.println(velocidade);
        // se ja estiver girando, atualiza o PWM em tempo real
        ledcWrite(CH_ENA, velocidade);
        break;

      case '-':
        velocidade = max(0, velocidade - 15);
        Serial.print("Velocidade: ");
        Serial.println(velocidade);
        ledcWrite(CH_ENA, velocidade);
        break;

      default:
        break;
    }
  }
}