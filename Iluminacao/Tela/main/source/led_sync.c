#include "led_sync.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_http_client.h"
#include "esp_netif.h"
#include "mdns.h"
#include "esp_log.h"

static const char *TAG = "LED_SYNC";

/*
 * Hostname mDNS da Wroom (sem ".local" - a query aqui e so o nome
 * puro). A Wroom precisa estar anunciando esse mesmo nome via
 * ESPmDNS (MDNS.begin("motormovel")) do lado dela.
 */
#define WROOM_MDNS_HOSTNAME "motormovel"
#define WROOM_LED_PATH       "/led_movel"

#define MDNS_QUERY_TIMEOUT_MS 2000
#define HTTP_TIMEOUT_MS       2000
#define LED_SYNC_TASK_STACK   4096
#define LED_SYNC_TASK_PRIO    4

typedef struct {
    int     id;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_cmd_t;

static QueueHandle_t   s_queue         = NULL;
static bool            s_started       = false;
static esp_ip4_addr_t  s_wroom_ip_cache;
static bool            s_wroom_ip_valida = false;

/* Resolve "motormovel.local" -> IPv4 via mDNS.
 * Retorna true e preenche out_ip se achou. */
static bool resolver_wroom_ip(esp_ip4_addr_t *out_ip)
{
    esp_ip4_addr_t addr = { 0 };
    esp_err_t err = mdns_query_a(WROOM_MDNS_HOSTNAME, MDNS_QUERY_TIMEOUT_MS, &addr);

    if (err != ESP_OK) {
        if (err == ESP_ERR_NOT_FOUND) {
            ESP_LOGW(TAG, "Host %s.local nao respondeu ao mDNS (Wroom desligada ou fora da rede?)",
                      WROOM_MDNS_HOSTNAME);
        } else {
            ESP_LOGW(TAG, "Erro na query mDNS: %s", esp_err_to_name(err));
        }
        return false;
    }

    *out_ip = addr;
    return true;
}

/* Faz o POST de fato. Roda dentro da task, nunca chamado direto
 * pelo handler HTTP do /tela. */
static void enviar_http(const color_cmd_t *cmd)
{
    esp_ip4_addr_t ip;
    bool resolvido = resolver_wroom_ip(&ip);

    if (resolvido) {
        s_wroom_ip_cache   = ip;
        s_wroom_ip_valida  = true;
    } else if (s_wroom_ip_valida) {
        ESP_LOGW(TAG, "mDNS falhou nessa tentativa, usando ultimo IP conhecido em cache.");
        ip = s_wroom_ip_cache;
    } else {
        ESP_LOGE(TAG, "Sem IP valido da Wroom (mDNS falhou e sem cache). Comando descartado.");
        return;
    }

    char ip_str[16];
    snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&ip));

    char url[96];
    snprintf(url, sizeof(url), "http://%s%s", ip_str, WROOM_LED_PATH);

    char json[96];
    if (cmd->id == 10) {
        snprintf(json, sizeof(json), "{\"id\":10,\"r\":%d,\"g\":%d,\"b\":%d}",
                 cmd->r, cmd->g, cmd->b);
    } else {
        snprintf(json, sizeof(json), "{\"id\":%d}", cmd->id);
    }

    esp_http_client_config_t config = {
        .url        = url,
        .method     = HTTP_METHOD_POST,
        .timeout_ms = HTTP_TIMEOUT_MS,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Falha ao criar cliente HTTP");
        return;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json, strlen(json));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "LED sincronizado via %s (%s) -> status %d", url, json, status);
    } else {
        ESP_LOGW(TAG, "Falha ao sincronizar LED via %s (%s): %s", url, json, esp_err_to_name(err));
        /* Se a requisicao falhou de verdade (nao so a query mDNS),
         * o IP em cache pode estar desatualizado - descarta pra
         * forcar nova resolucao mDNS na proxima tentativa. */
        s_wroom_ip_valida = false;
    }

    esp_http_client_cleanup(client);
}

/* Task consumidora: fica bloqueada esperando um comando novo na fila
 * e so entao faz o POST. Enquanto o POST estiver rodando, comandos
 * novos podem sobrescrever a fila (xQueueOverwrite no lado do
 * produtor) - ao terminar, a task pega a versao mais recente. */
static void led_sync_task(void *arg)
{
    color_cmd_t cmd;

    for (;;) {
        if (xQueueReceive(s_queue, &cmd, portMAX_DELAY) == pdTRUE) {
            enviar_http(&cmd);
        }
    }
}

void led_sync_init(void)
{
    if (s_started) {
        ESP_LOGW(TAG, "led_sync_init chamado mais de uma vez, ignorando.");
        return;
    }

    esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao iniciar mDNS (%s). led_sync nao vai funcionar.",
                  esp_err_to_name(err));
        return;
    }

    s_queue = xQueueCreate(1, sizeof(color_cmd_t));
    if (s_queue == NULL) {
        ESP_LOGE(TAG, "Falha ao criar fila do led_sync");
        return;
    }

    BaseType_t ok = xTaskCreate(led_sync_task, "led_sync_task",
                                 LED_SYNC_TASK_STACK, NULL,
                                 LED_SYNC_TASK_PRIO, NULL);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "Falha ao criar task do led_sync");
        return;
    }

    s_started = true;
    ESP_LOGI(TAG, "led_sync pronto (mDNS + task + fila). Alvo: %s.local", WROOM_MDNS_HOSTNAME);
}

void led_sync_send_color(int id, uint8_t r, uint8_t g, uint8_t b)
{
    if (!s_started) {
        ESP_LOGW(TAG, "led_sync_send_color chamado antes de led_sync_init! Ignorando.");
        return;
    }

    color_cmd_t cmd = { .id = id, .r = r, .g = g, .b = b };

    /* xQueueOverwrite: nunca bloqueia, nunca falha por fila cheia.
     * Se a task ainda nao consumiu o comando anterior, ele e
     * substituido - so a cor mais recente importa. */
    xQueueOverwrite(s_queue, &cmd);
}
