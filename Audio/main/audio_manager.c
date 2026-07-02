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


void audio_deinit(void) {
    if(player) {
        gst_element_set_state(player, GST_STATE_NULL);
        gst_object_unref(player);
        player = NULL;
    }
}


bool audio_play(const char *file) {
    if(player == NULL) {
        return false;
    }

    if(file == NULL || strlen(file) == 0) {
        return false;
    }

    char absolute_path[PATH_MAX];
    if(realpath(file, absolute_path) == NULL) {
        state = AUDIO_ERROR;
        return false;
    }

    snprintf(current_file, sizeof(current_file), "%s", absolute_path);
    gchar *uri = gst_filename_to_uri(absolute_path, NULL);

    g_object_set(player, "uri", uri, NULL);
    g_free(uri);

    GstStateChangeReturn ret = gst_element_set_state(player, GST_STATE_PLAYING);
    if(ret == GST_STATE_CHANGE_FAILURE) {
        state = AUDIO_ERROR;
        return false;
    }

    state = AUDIO_PLAYING;
    return true;
}


void audio_stop(void) {
    if(player == NULL) {
        return;
    }

    gst_element_set_state(player, GST_STATE_READY);
    state = AUDIO_STOPPED;
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