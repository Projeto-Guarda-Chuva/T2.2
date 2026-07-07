#include "protocol.h"
#include "display_manager.h"
#include "esp_system.h"
#include "cJSON.h"
#include "esp_log.h"

static const char *TAG = "PROTOCOLO";

esp_err_t process_command_json(const char *json_str) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGE(TAG, "Erro ao ler JSON");
        return ESP_FAIL;
    }

    cJSON *id_item = cJSON_GetObjectItem(root, "id");
    if (cJSON_IsNumber(id_item)) {
        int id = id_item->valueint;
        ESP_LOGI(TAG, "Comando Recebido: %d", id);

        switch (id) {
            case 10: {
                int r = cJSON_GetObjectItem(root, "r") ? cJSON_GetObjectItem(root, "r")->valueint : 0;
                int g = cJSON_GetObjectItem(root, "g") ? cJSON_GetObjectItem(root, "g")->valueint : 0;
                int b = cJSON_GetObjectItem(root, "b") ? cJSON_GetObjectItem(root, "b")->valueint : 0;

                if (r > 255) r = 255; if (r < 0) r = 0;
                if (g > 255) g = 255; if (g < 0) g = 0;
                if (b > 255) b = 255; if (b < 0) b = 0;

                display_set_bg_color(r, g, b);
                break;
            }
            case 11: display_set_bg_color(255, 0, 0); break;
            case 12: display_set_bg_color(0, 255, 0); break;
            case 13: display_set_bg_color(0, 0, 255); break;
            case 14: display_set_bg_color(0, 0, 0); break;
            case 15: display_set_bg_color(255, 255, 255); break;

            default:
                ESP_LOGW(TAG, "ID Desconhecido");
                break;
        }
    }
    cJSON_Delete(root);
    return ESP_OK;
}