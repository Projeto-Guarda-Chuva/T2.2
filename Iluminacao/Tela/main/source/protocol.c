#include "protocol.h"
#include "display_manager.h"
#include "led_sync.h"
#include "esp_system.h"
#include "cJSON.h"
#include "esp_log.h"
#include <stdbool.h>

static const char *TAG = "PROTOCOLO";

void process_command_json(const char *json_str) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGE(TAG, "Erro ao ler JSON");
        return;
    }

    cJSON *id_item = cJSON_GetObjectItem(root, "id");
    if (cJSON_IsNumber(id_item)) {
        int id = id_item->valueint;
        ESP_LOGI(TAG, "Comando Recebido: %d", id);

        int r = 0, g = 0, b = 0;
        bool cor_valida = true;

        switch (id) {
            // GRUPO 10: TELA (COR)
            case 10: {
                r = cJSON_GetObjectItem(root, "r") ? cJSON_GetObjectItem(root, "r")->valueint : 0;
                g = cJSON_GetObjectItem(root, "g") ? cJSON_GetObjectItem(root, "g")->valueint : 0;
                b = cJSON_GetObjectItem(root, "b") ? cJSON_GetObjectItem(root, "b")->valueint : 0;
                break;
            }
            case 11: r = 255; g = 0;   b = 0;   break; // Vermelho
            case 12: r = 0;   g = 255; b = 0;   break; // Verde
            case 13: r = 0;   g = 0;   b = 255; break; // Azul
            case 14: r = 0;   g = 0;   b = 0;   break; // Preto
            case 15: r = 255; g = 255; b = 255; break; // Branco

            default:
                ESP_LOGW(TAG, "ID Desconhecido");
                cor_valida = false;
                break;
        }

        if (cor_valida) {
            // Atualiza a tela local...
            display_set_bg_color(r, g, b);
            // ...e replica a mesma cor para o LED fisico na Wroom
            led_sync_send_color(id, r, g, b);
        }
    }
    cJSON_Delete(root);
}