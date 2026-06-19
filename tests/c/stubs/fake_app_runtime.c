#include "test_support.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include <stdint.h>
#include <string.h>

esp_err_t esp_netif_init(void) {
    g_test_state.esp_netif_init_call_count++;
    return ESP_OK;
}

void *esp_netif_create_default_wifi_sta(void) {
    g_test_state.esp_netif_create_default_wifi_sta_call_count++;
    return (void *)0x22;
}

esp_err_t esp_event_loop_create_default(void) {
    g_test_state.esp_event_loop_create_default_call_count++;
    return ESP_OK;
}

esp_err_t esp_event_handler_register(
    esp_event_base_t event_base,
    int32_t event_id,
    esp_event_handler_t event_handler,
    void *event_handler_arg
) {
    g_test_state.esp_event_handler_register_call_count++;
    g_test_state.last_event_base = event_base;
    g_test_state.last_event_id = event_id;
    g_test_state.last_event_handler = event_handler;
    g_test_state.last_event_handler_arg = event_handler_arg;
    return ESP_OK;
}

esp_err_t esp_wifi_init(const wifi_init_config_t *config) {
    (void)config;
    g_test_state.esp_wifi_init_call_count++;
    return ESP_OK;
}

esp_err_t esp_wifi_set_mode(int mode) {
    g_test_state.esp_wifi_set_mode_call_count++;
    g_test_state.last_wifi_mode = mode;
    return ESP_OK;
}

esp_err_t esp_wifi_set_config(int interface_id, const wifi_config_t *config) {
    g_test_state.esp_wifi_set_config_call_count++;
    g_test_state.last_wifi_interface_id = interface_id;
    if (config != NULL) {
        memcpy(&g_test_state.last_wifi_config, config, sizeof(wifi_config_t));
    }
    return ESP_OK;
}

esp_err_t esp_wifi_start(void) {
    g_test_state.esp_wifi_start_call_count++;
    return ESP_OK;
}

esp_err_t esp_wifi_connect(void) {
    g_test_state.esp_wifi_connect_call_count++;
    return ESP_OK;
}

esp_err_t nvs_flash_init(void) {
    int index = g_test_state.next_nvs_flash_init_result_index;
    if (index < g_test_state.next_nvs_flash_init_result_count) {
        g_test_state.next_nvs_flash_init_result_index++;
    } else if (g_test_state.next_nvs_flash_init_result_count > 0) {
        index = g_test_state.next_nvs_flash_init_result_count - 1;
    } else {
        index = 0;
    }

    g_test_state.nvs_flash_init_call_count++;
    return g_test_state.next_nvs_flash_init_results[index];
}

esp_err_t nvs_flash_erase(void) {
    g_test_state.nvs_flash_erase_call_count++;
    return ESP_OK;
}
