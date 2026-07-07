#include "audio_manager.h"
#include <stdio.h>
#include <string.h>

#ifndef PRODUCTION_ENV
    #define IS_TEST_ENVIRONMENT
#endif

#ifndef AUDIO_MANAGER_C_GUARD
#define AUDIO_MANAGER_C_GUARD

static const char *TRACKS[] = {
    "audios/Modo - Água Viva.mp3",
    "audios/Modo - Pôr do Sol.mp3"
};

static AudioManager global_am;
static bool is_initialized = false;
static uint8_t current_volume_int = 100;


#ifndef IS_TEST_ENVIRONMENT
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    AudioManager *am = (AudioManager *)data;
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS: {
            printf("[INFO] Fim da faixa alcançado. Alternando playlist...\n");
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
            printf("[ERROR] Erro no GStreamer: %s!\n", error->message);
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
    } 
    
    else {
        am->current_track = 0;
    }

    const char *file_to_play = (initial_file != NULL && strlen(initial_file) > 0) ? initial_file : TRACKS[am->current_track];


#ifndef IS_TEST_ENVIRONMENT
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
    if (!am->is_playing) {
        return;
    }
#ifndef IS_TEST_ENVIRONMENT
    gst_element_set_state(am->pipeline, GST_STATE_NULL);
#endif
    am->is_playing = false;
}


void audio_manager_set_volume(AudioManager *am, int volume_percent) {
    if (volume_percent < 0) {
        volume_percent = 0;
    }
    
    if (volume_percent > 100) {
        volume_percent = 100;
    }

    am->current_volume = (double)volume_percent / 100.0;

#ifndef IS_TEST_ENVIRONMENT
    g_object_set(G_OBJECT(am->pipeline), "volume", am->current_volume, NULL);
#endif
}


void audio_manager_cleanup(AudioManager *am) {
    audio_manager_stop(am);
#ifndef IS_TEST_ENVIRONMENT
    if (am->pipeline) gst_object_unref(am->pipeline);
#endif
}


void audio_init(void) {
    audio_manager_init(&global_am, NULL);
    is_initialized = true;
    current_volume_int = 100;
    
    printf("[INFO] Inicializando Atuador de Audio...\n");
    printf("[INFO] Componente carregado com sucesso.\n");
    printf("[INFO] Volume inicial configurado em 100%%\n");
}


void audio_play(const char *filename) {
    if (!is_initialized) {
        printf("[WARN] Componente nao inicializado!\n");
        return;
    }

    if (filename == NULL || strlen(filename) == 0) {
        printf("[ERROR] Arquivo invalido para reproducao.\n");
        return;
    }

    printf("[INFO] Reproduzindo arquivo: %s\n", filename);
    audio_manager_start_playlist(&global_am, filename);
}


void audio_play_default(void) {
#ifndef IS_TEST_ENVIRONMENT
    audio_play(TRACKS[0]);
#else
    audio_play("default.mp3");
#endif
}


void audio_stop(void) {
    if (!global_am.is_playing) {
        printf("[WARN] Nenhum audio esta sendo reproduzido.\n");
        return;
    }

    printf("[INFO] Parando audio...\n");
    audio_manager_stop(&global_am);
}


bool audio_is_playing(void) {
    return global_am.is_playing;
}


void audio_set_volume(int8_t volume) {
    uint8_t u_volume = (uint8_t)volume;

    if (u_volume > 100) {
        printf("[WARN] Volume %d invalido! Ajustando para 100.\n", u_volume);
        audio_manager_set_volume(&global_am, 100);
        current_volume_int = 100;
        return;
    }

    printf("[INFO] Alterando volume de %d para %d\n", current_volume_int, u_volume);
    audio_manager_set_volume(&global_am, u_volume);
    current_volume_int = u_volume;
}

#endif