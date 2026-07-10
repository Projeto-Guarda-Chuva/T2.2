#include "led_movel.h"
#include <FastLED.h>

static CRGB seg1[LED_POR_SEGMENTO];
static CRGB seg2[LED_POR_SEGMENTO];
static CRGB seg3[LED_POR_SEGMENTO];

void led_movel_setup(void)
{
    FastLED.addLeds<WS2812B, LED_PIN_SEG1, GRB>(seg1, LED_POR_SEGMENTO);
    FastLED.addLeds<WS2812B, LED_PIN_SEG2, GRB>(seg2, LED_POR_SEGMENTO);
    FastLED.addLeds<WS2812B, LED_PIN_SEG3, GRB>(seg3, LED_POR_SEGMENTO);
    FastLED.setBrightness(LED_BRILHO);

    led_movel_set_color(0, 0, 0); /* estado inicial: apagado */
}

void led_movel_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    fill_solid(seg1, LED_POR_SEGMENTO, CRGB(r, g, b));
    fill_solid(seg2, LED_POR_SEGMENTO, CRGB(r, g, b));
    fill_solid(seg3, LED_POR_SEGMENTO, CRGB(r, g, b));
    FastLED.show();

    Serial.print("LED: R=");
    Serial.print(r);
    Serial.print(" G=");
    Serial.print(g);
    Serial.print(" B=");
    Serial.println(b);
}

bool led_movel_handle_id(int id, uint8_t r, uint8_t g, uint8_t b)
{
    switch (id) {
        case ID_LED_COR_CUSTOM:
            led_movel_set_color(r, g, b);
            return true;

        case ID_LED_VERMELHO:
            led_movel_set_color(255, 0, 0);
            return true;

        case ID_LED_VERDE:
            led_movel_set_color(0, 255, 0);
            return true;

        case ID_LED_AZUL:
            led_movel_set_color(0, 0, 255);
            return true;

        case ID_LED_PRETO:
            led_movel_set_color(0, 0, 0);
            return true;

        case ID_LED_BRANCO:
            led_movel_set_color(255, 255, 255);
            return true;

        default:
            return false; /* ID desconhecido para esta família */
    }
}
