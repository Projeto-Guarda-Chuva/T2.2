#include "led_strip.h"

#include <string.h>

struct led_strip_s {
    int dummy;
};

static struct led_strip_s fake_strip;

led_spy_t g_led_spy;

void led_spy_reset(void) {
    memset(&g_led_spy, 0, sizeof(g_led_spy));
}

esp_err_t led_strip_new_rmt_device(const led_strip_config_t *config,
                                   const led_strip_rmt_config_t *rmt_config,
                                   led_strip_handle_t *ret_strip) {
    (void)config;
    (void)rmt_config;
    g_led_spy.new_device_count++;
    *ret_strip = &fake_strip;
    return ESP_OK;
}

esp_err_t led_strip_set_pixel(led_strip_handle_t strip, uint32_t index,
                              uint32_t red, uint32_t green, uint32_t blue) {
    (void)strip;
    g_led_spy.set_pixel_count++;
    g_led_spy.last_index = index;
    g_led_spy.last_r = (uint8_t)red;
    g_led_spy.last_g = (uint8_t)green;
    g_led_spy.last_b = (uint8_t)blue;
    return ESP_OK;
}

esp_err_t led_strip_refresh(led_strip_handle_t strip) {
    (void)strip;
    g_led_spy.refresh_count++;
    return ESP_OK;
}

esp_err_t led_strip_clear(led_strip_handle_t strip) {
    (void)strip;
    g_led_spy.clear_count++;
    return ESP_OK;
}
