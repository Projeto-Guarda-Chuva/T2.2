#include "protocol.h"
#include "display_manager.h"
#include "esp_system.h"
#include "cJSON.h"
#include "esp_log.h"

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

        switch (id) {
            // GRUPO 10: TELA(COR)
            case 10: {
                int r = cJSON_GetObjectItem(root, "r") ? cJSON_GetObjectItem(root, "r")->valueint : 0;
                int g = cJSON_GetObjectItem(root, "g") ? cJSON_GetObjectItem(root, "g")->valueint : 0;
                int b = cJSON_GetObjectItem(root, "b") ? cJSON_GetObjectItem(root, "b")->valueint : 0;
                display_set_bg_color(r, g, b);
                break;
            }
            case 11: display_set_bg_color(255, 0, 0); break;   // Vermelho
            case 12: display_set_bg_color(0, 255, 0); break;   // Verde
            case 13: display_set_bg_color(0, 0, 255); break;   // Azul
            case 14: display_set_bg_color(0, 0, 0); break;     // Preto
            case 15: display_set_bg_color(255, 255, 255); break; // Branco

            default:
                ESP_LOGW(TAG, "ID Desconhecido");
                break;
        }
    }
    cJSON_Delete(root);
}