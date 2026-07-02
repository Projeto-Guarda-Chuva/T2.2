#include "audio_manager.h"

#include <gst/gst.h>
#include <glib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static GstElement *player = NULL;

static audio_state_t state = AUDIO_IDLE;

static uint8_t current_volume = 100;

static char current_file[PATH_MAX];


bool audio_init(void) {
    gst_init(NULL, NULL);

    player = gst_element_factory_make("playbin", "audio-player")

    if(player == NULL) {
        state = AUDIO_ERROR;
        return false;
    }

    g_object_set(player, "volume", current_volume / 100.0, NULL);
    state = AUDIO_IDLE;
    return true;
}


bool audio_is_playing(void) {
    if(current_state == AUDIO_PLAYING) {
        return true;
    }

    return false;
}


void audio_play_default(void) {
    ESP_LOGI(TAG, "Reproduzindo áudio padrão!");
    audio_play("default.mp3");
}

void audio_play(const char *file) {
    if(file == NULL || strlen(file) == 0) {
        ESP_LOGE(TAG, "Nome do arquivo inválido!");
        return;
    }

    strncpy(current_file, file, sizeof(current_file)-1);
    current_file[sizeof(current_file)-1] = '\0';

    current_state = AUDIO_PLAYING;
    ESP_LOGI(TAG, 
        "Reproduzindo arquivo \"%s\" (Volume: %d%%)", 
        current_file, 
        current_volume);
}


void audio_stop(void) {
    if(AUDIO_STOPPED) {
        ESP_LOGW(TAG, "Nenhum áudio está sendo reproduzido.");
        return;
    }

    current_state = AUDIO_STOPPED;
    ESP_LOGI(TAG, "Parando áudio!");
}


void audio_set_volume(uint8_t volume) {
    if(volume > 100) {
        ESP_LOGW(TAG, "Volume %d inválido! Ajustando para 100.", volume);
        volume = 100;
    }

    ESP_LOGI(TAG, "Alterando o volume atual de %d para %d%%", current_volume, volume);
    current_volume = volume;
}