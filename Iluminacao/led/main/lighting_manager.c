#include "lighting_manager.h"
#include "led_strip.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

#define LED_STRIP_BLINK_GPIO  2 // GPIO assignment
#define LED_STRIP_LED_NUMBERS 24 // Numbers of the LED in the strip
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

static const char *TAG = "LIGHTING";
static led_strip_handle_t led_strip = NULL;

static uint8_t current_r = 0;
static uint8_t current_g = 0;
static uint8_t current_b = 0;
static uint8_t current_intensity = 255;


void lighting_init(void) {
    if (led_strip != NULL) {
        ESP_LOGW(TAG, "Lighting já inicializado!");
        return;
    }

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_BLINK_GPIO,
        .max_leds = LED_STRIP_LED_NUMBERS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RMT_RES_HZ,
        .flags.with_dma = false,
    };

    ESP_LOGI(TAG, "Inicializando fita LED...");
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Lighting inicializado!");
}


void lighting_set_color(uint8_t r, uint8_t g, uint8_t b) {
    if (led_strip == NULL) {
        ESP_LOGE(TAG, "LED Strip não inicializado!");
        return;
    }

    current_r = r;
    current_g = g;
    current_b = b;

    uint8_t scaled_r = (r * current_intensity) / 255;
    uint8_t scaled_g = (g * current_intensity) / 255;
    uint8_t scaled_b = (b * current_intensity) / 255;

    ESP_LOGI(TAG, "Nova cor recebida: R=%d G=%d B=%d!", r, g, b);

    for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, scaled_r, scaled_g, scaled_b));
    }

    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}


void lighting_set_intensity(uint8_t intensity) {
    if (led_strip == NULL) {
        return;
    }

    ESP_LOGI(TAG, "Alterando intensidade de %d para %d!", current_intensity, intensity);
    current_intensity = intensity;
    lighting_set_color(current_r, current_g, current_b);
}


void lighting_off(void) {
    if (led_strip == NULL) {
        ESP_LOGE(TAG, "LED Strip não inicializado!");
        return;
    }

    ESP_ERROR_CHECK(led_strip_clear(led_strip));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));

    current_r = 0;
    current_g = 0;
    current_b = 0;

    ESP_LOGI(TAG, "Iluminacao desligada: %d LEDs apagados", LED_STRIP_LED_NUMBERS);
}