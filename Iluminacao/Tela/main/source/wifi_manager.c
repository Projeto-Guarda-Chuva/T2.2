#include "wifi_manager.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#define WIFI_SSID "hotspot"
//#define WIFI_PASS ""

#define STATIC_IP       "192.168.76.43"
#define STATIC_GATEWAY  "192.168.76.1"
#define STATIC_NETMASK  "255.255.254.0"

void wifi_init_simple(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Cria a interface STA e ja configura IP estatico antes do wifi_init */
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();

    /* Para o cliente DHCP e aplica IP fixo */
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));

    esp_netif_ip_info_t ip_info = {};
    ip_info.ip.addr      = esp_ip4addr_aton(STATIC_IP);
    ip_info.gw.addr      = esp_ip4addr_aton(STATIC_GATEWAY);
    ip_info.netmask.addr = esp_ip4addr_aton(STATIC_NETMASK);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));

    ESP_LOGI("WIFI", "IP estatico configurado: %s", STATIC_IP);

    /* Inicializa e conecta normalmente */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = WIFI_SSID,
            //.password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_LOGI("WIFI", "Conectando ao Wi-Fi...");
}