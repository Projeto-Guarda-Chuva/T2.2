#pragma once

#include <stdint.h>

#include "esp_err.h"

typedef const char *esp_event_base_t;
typedef void (*esp_event_handler_t)(void *arg, esp_event_base_t base, int32_t id, void *data);

typedef struct {
    uint8_t addr[4];
} esp_ip4_addr_t;

typedef struct {
    esp_ip4_addr_t ip;
} esp_netif_ip_info_t;

typedef struct {
    esp_netif_ip_info_t ip_info;
} ip_event_got_ip_t;

#define IP_EVENT ((esp_event_base_t)"IP_EVENT")
#define IP_EVENT_STA_GOT_IP 1
#define IPSTR "%d.%d.%d.%d"
#define IP2STR(ip) 0, 0, 0, 0

esp_err_t esp_event_loop_create_default(void);
esp_err_t esp_event_handler_register(
    esp_event_base_t event_base,
    int32_t event_id,
    esp_event_handler_t event_handler,
    void *event_handler_arg
);
