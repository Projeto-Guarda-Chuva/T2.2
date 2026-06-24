#include "audio_manager.h"

#include "esp_log.h"

static const char *TAG = "AUDIO";

static uint8_t current_volume = 100;

void audio_init(void) {
    ESP_LOGI(TAG, "Inicializando atuador de áudio...");
    ESP_LOGI(TAG, "Volume inicial: %d%%", current_volume);
    ESP_LOGI(TAG, "Atuador de áudio inicializado!");
}


void audio_play(void) {
    ESP_LOGI(TAG, "Reproduzindo áudio!");
}


void audio_stop(void) {
    ESP_LOGI(TAG, "Parando áudio!");
}


void audio_set_volume(uint8_t volume) {
    if(volume > 100) {
        ESP_LOGW(TAG, "Volume %d inválido! Ajustando para 100.", volume);
        current_volume = 100;
    }

    ESP_LOGI(TAG, "Alterando o volume atual de %d para %d%%", current_volume, volume);
}
