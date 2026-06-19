#include "test_support.h"
#include "led_strip.h"

#include <string.h>

void process_command_json(const char *json_str) {
    g_test_state.process_command_json_call_count++;
    strncpy(
        g_test_state.last_process_command_json,
        json_str,
        sizeof(g_test_state.last_process_command_json) - 1
    );
}

esp_err_t led_strip_new_rmt_device(
    const led_strip_config_t *strip_config,
    const led_strip_rmt_config_t *rmt_config,
    led_strip_handle_t *ret_strip
) {
    (void)strip_config;
    (void)rmt_config;
    g_test_state.led_strip_new_device_call_count++;
    if (ret_strip != NULL) {
        *ret_strip = (led_strip_handle_t)0x1234;
    }
    return ESP_OK;
}

esp_err_t led_strip_set_pixel(
    led_strip_handle_t strip,
    uint32_t index,
    uint32_t red,
    uint32_t green,
    uint32_t blue
) {
    (void)strip;
    g_test_state.led_strip_set_pixel_call_count++;
    g_test_state.last_pixel_index = (int)index;
    g_test_state.last_pixel_r = (int)red;
    g_test_state.last_pixel_g = (int)green;
    g_test_state.last_pixel_b = (int)blue;
    return ESP_OK;
}

esp_err_t led_strip_refresh(led_strip_handle_t strip) {
    (void)strip;
    g_test_state.led_strip_refresh_call_count++;
    return ESP_OK;
}

esp_err_t led_strip_clear(led_strip_handle_t strip) {
    (void)strip;
    g_test_state.led_strip_clear_call_count++;
    return ESP_OK;
}
