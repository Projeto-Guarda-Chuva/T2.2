#pragma once

#include "esp_err.h"
#include "esp_netif.h"

typedef struct {
    int unused;
} wifi_init_config_t;

typedef struct {
    struct {
        char ssid[32];
        char password[64];
    } sta;
} wifi_config_t;

#define WIFI_INIT_CONFIG_DEFAULT() ((wifi_init_config_t){0})
#define WIFI_MODE_STA 1
#define WIFI_IF_STA 2

esp_err_t esp_wifi_init(const wifi_init_config_t *config);
esp_err_t esp_wifi_set_mode(int mode);
esp_err_t esp_wifi_set_config(int interface_id, const wifi_config_t *config);
esp_err_t esp_wifi_start(void);
esp_err_t esp_wifi_connect(void);
