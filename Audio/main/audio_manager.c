#ifndef AUDIO_MANAGER_C_GUARD
#define AUDIO_MANAGER_C_GUARD

#include "audio_manager.h"
#include <stdio.h>
#include <string.h>

// Macros para emular o comportamento do ESP_LOG no Linux da Jetson:
#define ESP_LOGI(tag, fmt, ...) printf("[%s] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGE(tag, fmt, ...) fprintf(stderr, "[%s] ERR: " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) printf("[%s] WARN: " fmt "\n", tag, ##__VA_ARGS__)

#ifndef PRODUCTION_ENV
    #define IS_TEST_ENVIRONMENT
#endif

static const char *TAG = "AUDIO_MANAGER";

static const char *TRACKS[] = {
    "audios/Modo - Água Viva.mp3",
    "audios/Modo - Pôr do Sol.mp3"
};

static AudioManager global_am;
static bool is_initialized = false;
static int current_volume_int = 100;


#ifndef IS_TEST_ENVIRONMENT
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    AudioManager *am = (AudioManager *)data;
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS: {
            ESP_LOGI(TAG, "Fim da faixa alcançado. Alternando playlist...");
            am->current_track = 1 - am->current_track; 
            gst_element_set_state(am->pipeline, GST_STATE_READY);
            gchar *absolute_path = g_canonicalize_filename(TRACKS[am->current_track], NULL);
            gchar *uri = g_filename_to_uri(absolute_path, NULL, NULL);
            g_object_set(G_OBJECT(am->pipeline), "uri", uri, NULL);
            g_free(absolute_path); g_free(uri);
            g_object_set(G_OBJECT(am->pipeline), "volume", am->current_volume, NULL);
            gst_element_set_state(am->pipeline, GST_STATE_PLAYING);
            break;
        }
        case GST_MESSAGE_ERROR: {
            gchar *debug; GError *error;
            gst_message_parse_error(msg, &error, &debug);
            ESP_LOGE(TAG, "Erro no GStreamer: %s!", error->message);
            g_error_free(error); g_free(debug);
            am->is_playing = false;
            break;
        }
        default: break;
    }
    return TRUE;
}
#endif


void audio_manager_init(AudioManager *am, void *loop) {
#ifndef IS_TEST_ENVIRONMENT
    gst_init(NULL, NULL);
    am->loop = (GMainLoop *)loop;
    am->pipeline = gst_element_factory_make("playbin", "audio-player");
    am->source = am->pipeline;
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(am->pipeline));
    gst_bus_add_watch(bus, bus_call, am);
    gst_object_unref(bus);
#endif
    (void)loop;
    am->is_playing = false;
    am->current_track = 0;
    am->current_volume = 1.0;
}


void audio_manager_start_playlist(AudioManager *am, const char *initial_file) {
    if (am->is_playing) {
        audio_manager_stop(am);
    }
    
    if (initial_file && strcmp(initial_file, TRACKS[1]) == 0) {
        am->current_track = 1;
    } else {
        am->current_track = 0;
    }


#ifndef IS_TEST_ENVIRONMENT
    const char *file_to_play = (initial_file != NULL && strlen(initial_file) > 0) ? initial_file : TRACKS[am->current_track];
    gst_element_set_state(am->pipeline, GST_STATE_READY);
    gchar *absolute_path = g_canonicalize_filename(file_to_play, NULL);
    gchar *uri = g_filename_to_uri(absolute_path, NULL, NULL);
    g_object_set(G_OBJECT(am->pipeline), "uri", uri, NULL);
    g_object_set(G_OBJECT(am->pipeline), "volume", am->current_volume, NULL);
    g_free(absolute_path); g_free(uri);
    gst_element_set_state(am->pipeline, GST_STATE_PLAYING);
#endif
    am->is_playing = true;
}


void audio_manager_stop(AudioManager *am) {
#ifndef IS_TEST_ENVIRONMENT
    gst_element_set_state(am->pipeline, GST_STATE_NULL);
#endif
    am->is_playing = false;
}


void audio_manager_set_volume(AudioManager *am, int volume_percent) {
    if (volume_percent < 0) volume_percent = 0;
    if (volume_percent > 100) volume_percent = 100;
    am->current_volume = (double)volume_percent / 100.0;
#ifndef IS_TEST_ENVIRONMENT
    g_object_set(G_OBJECT(am->pipeline), "volume", am->current_volume, NULL);
#endif
}


void audio_init(void) {
    audio_manager_init(&global_am, NULL);
    is_initialized = true;
    current_volume_int = 100;
    
    ESP_LOGI(TAG, "Inicializando Atuador de Audio...");
    ESP_LOGI(TAG, "Componente carregado com sucesso.");
    ESP_LOGI(TAG, "Volume inicial configurado em 100%%");
}


void audio_play(const char *filename) {
    if (!is_initialized) {
        ESP_LOGW(TAG, "Componente nao inicializado!");
        return;
    }

    if (filename == NULL || strlen(filename) == 0) {
        ESP_LOGE(TAG, "Arquivo invalido para reproducao.");
        return;
    }

    ESP_LOGI(TAG, "Reproduzindo arquivo: %s", filename);
    audio_manager_start_playlist(&global_am, filename);
}


void audio_play_default(void) {
    if (!is_initialized) {
        ESP_LOGW(TAG, "Componente nao inicializado!");
        return;
    }

    audio_play("default.mp3");
}


void audio_stop(void) {
    if (!is_initialized) {
        ESP_LOGW(TAG, "Componente nao inicializado!");
        return;
    }

    if (!global_am.is_playing) {
        ESP_LOGW(TAG, "Nenhum audio esta sendo reproduzido.");
        return;
    }

    ESP_LOGI(TAG, "Parando audio...");
    audio_manager_stop(&global_am);
}


bool audio_is_playing(void) {
    if (!is_initialized) {
        ESP_LOGW(TAG, "Componente nao inicializado!");
        return false;
    }

    return global_am.is_playing;
}


void audio_set_volume(int volume) {
    if (!is_initialized) {
        ESP_LOGW(TAG, "Componente nao inicializado!");
        return;
    }

    uint8_t volume_u8 = (uint8_t)volume;

    if (volume_u8 > 100) {
        ESP_LOGW(TAG, "Volume %d invalido! Ajustando para 100.", volume_u8);
        audio_manager_set_volume(&global_am, 100);
        current_volume_int = 100;
        return;
    }

    ESP_LOGI(TAG, "Alterando volume de %d para %d", current_volume_int, volume_u8);
    audio_manager_set_volume(&global_am, volume_u8);
    current_volume_int = volume_u8;
}

#endif