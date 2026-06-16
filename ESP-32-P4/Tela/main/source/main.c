#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "wifi_manager.h"
#include "http_server.h"
#include "display_manager.h"

static const char *TAG = "MAIN";

static void on_got_ip(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
    ESP_LOGI(TAG, "Wi-Fi Conectado! IP: " IPSTR, IP2STR(&event->ip_info.ip));
}

static void display_task(void *arg)
{
    display_init();
    display_set_bg_color(255, 255, 255);
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando Sistema ESP32-P4");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    xTaskCreate(display_task, "display_init", 8192, NULL, 5, NULL);
    wifi_init_simple();
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, on_got_ip, NULL));
    ESP_LOGI(TAG, "Aguardando Wi-Fi...");
    vTaskDelay(pdMS_TO_TICKS(15000));
    start_web_server();
    ESP_LOGI(TAG, "Sistema pronto!");
}
