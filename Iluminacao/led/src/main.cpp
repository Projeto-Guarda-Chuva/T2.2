/*
  Controle de fita WS2812B com 3 segmentos (3 fios de dados) - ESP32
  Projeto PlatformIO - biblioteca FastLED

  Pinagem:
    GPIO18 --[resistor 330-470ohm]--> Segmento 1 (DIN)
    GPIO5  --[resistor 330-470ohm]--> Segmento 2 (DIN)
    GPIO17 --[resistor 330-470ohm]--> Segmento 3 (DIN)
    Fonte 5V (+) -> fio +5V (comum aos 3 segmentos)
    Fonte 5V (-) -> GND comum (fonte + ESP32 + os 3 segmentos)

  Ajuste LEDS_POR_SEGMENTO para a quantidade real de LEDs de CADA segmento
  (os 3 segmentos foram informados como sendo do mesmo tamanho).

  Comandos pelo Monitor Serial (baud 115200):
    r -> Vermelho (todos os segmentos)
    g -> Verde
    b -> Azul
    w -> Branco
    o -> Desliga
    c -> Efeito arco-iris continuo
    Ou digite: R G B  (ex: 255 0 128) -> cor customizada
*/

#include <Arduino.h>
#include <FastLED.h>

// ---------- Configuracao ----------
constexpr int PIN_SEG1 = 25;
constexpr int PIN_SEG2 = 26;
constexpr int PIN_SEG3 = 27;

constexpr int LEDS_POR_SEGMENTO = 300; // ajuste para a quantidade real de cada segmento
constexpr int BRILHO = 100;           // 0-255, comece baixo pra nao estourar a fonte

CRGB seg1[LEDS_POR_SEGMENTO];
CRGB seg2[LEDS_POR_SEGMENTO];
CRGB seg3[LEDS_POR_SEGMENTO];

bool modoRainbow = false;
uint8_t hueInicial = 0;

void setColor(uint8_t r, uint8_t g, uint8_t b) {
  modoRainbow = false;
  fill_solid(seg1, LEDS_POR_SEGMENTO, CRGB(r, g, b));
  fill_solid(seg2, LEDS_POR_SEGMENTO, CRGB(r, g, b));
  fill_solid(seg3, LEDS_POR_SEGMENTO, CRGB(r, g, b));
  FastLED.show();

  Serial.print("Cor: R=");
  Serial.print(r);
  Serial.print(" G=");
  Serial.print(g);
  Serial.print(" B=");
  Serial.println(b);
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, PIN_SEG1, GRB>(seg1, LEDS_POR_SEGMENTO);
  FastLED.addLeds<WS2812B, PIN_SEG2, GRB>(seg2, LEDS_POR_SEGMENTO);
  FastLED.addLeds<WS2812B, PIN_SEG3, GRB>(seg3, LEDS_POR_SEGMENTO);
  FastLED.setBrightness(BRILHO);

  setColor(0, 0, 0);

  Serial.println("Pronto.");
  Serial.println("Comandos: r=vermelho g=verde b=azul w=branco o=off c=rainbow");
  Serial.println("Ou digite: R G B  (ex: 255 0 128)");
}

void loop() {
  if (Serial.available()) {
    String linha = Serial.readStringUntil('\n');
    linha.trim();

    if (linha.length() > 0) {
      if (linha.length() == 1) {
        char c = linha.charAt(0);
        switch (c) {
          case 'r': case 'R': setColor(255, 0, 0); break;
          case 'g': case 'G': setColor(0, 255, 0); break;
          case 'b': case 'B': setColor(0, 0, 255); break;
          case 'w': case 'W': setColor(255, 255, 255); break;
          case 'o': case 'O': setColor(0, 0, 0); break;
          case 'c': case 'C':
            modoRainbow = true;
            Serial.println("Modo rainbow ativado");
            break;
          default:
            Serial.println("Comando invalido.");
            break;
        }
      } else {
        int r, g, b;
        int lidos = sscanf(linha.c_str(), "%d %d %d", &r, &g, &b);
        if (lidos == 3) {
          setColor((uint8_t)constrain(r, 0, 255),
                    (uint8_t)constrain(g, 0, 255),
                    (uint8_t)constrain(b, 0, 255));
        } else {
          Serial.println("Comando invalido. Use r/g/b/w/o/c ou 'R G B' (ex: 255 0 128)");
        }
      }
    }
  }

  if (modoRainbow) {
    uint8_t passoHue = 255 / LEDS_POR_SEGMENTO + 1;
    fill_rainbow(seg1, LEDS_POR_SEGMENTO, hueInicial, passoHue);
    fill_rainbow(seg2, LEDS_POR_SEGMENTO, hueInicial, passoHue);
    fill_rainbow(seg3, LEDS_POR_SEGMENTO, hueInicial, passoHue);
    FastLED.show();
    hueInicial++;
    delay(20);
  }
}
