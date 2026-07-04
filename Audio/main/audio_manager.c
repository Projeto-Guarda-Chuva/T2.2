#include "audio_manager.h"
#include <stdio.h>

#include <gst/gst.h>

static const char *TRACKS[] = {
    "audios/Modo - Água Viva.mp3",
    "audios/Modo - Pôr do Sol.mp3"
};


// Callback interno para detectar o fim da música e alternar:
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    AudioManager *am = (AudioManager *)data;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS: {
            g_print("Fim da faixa alcançado. Alternando playlist...\n");
            
            am->current_track = 1 - am->current_track; 
            gst_element_set_state(am->pipeline, GST_STATE_READY);
            
            gchar *absolute_path = g_canonicalize_filename(TRACKS[am->current_track], NULL);
            gchar *uri = g_filename_to_uri(absolute_path, NULL, NULL);
            
            g_object_set(G_OBJECT(am->pipeline), "uri", uri, NULL);
            
            g_free(absolute_path);
            g_free(uri);
            
            g_object_set(G_OBJECT(am->pipeline), "volume", am->current_volume, NULL);
            
            gst_element_set_state(am->pipeline, GST_STATE_PLAYING);
            break;
        }

        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;
            gst_message_parse_error(msg, &error, &debug);
            g_printerr("Erro no GStreamer: %s!\n", error->message);
            g_error_free(error);
            g_free(debug);
            am->is_playing = false;
            break;
        }

        default:
            break;
    }

    return TRUE;
}


void audio_manager_init(AudioManager *am, GMainLoop *loop) {
    gst_init(NULL, NULL);
    
    am->loop = loop;
    am->is_playing = false;
    am->current_track = 0;
    am->current_volume = 1.0;

    am->pipeline = gst_element_factory_make("playbin", "audio-player");
    am->source = am->pipeline;
    
    // Configura o barramento para capturar o fim da reprodução (EOS):
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(am->pipeline));
    gst_bus_add_watch(bus, bus_call, am);
    gst_object_unref(bus);
}


void audio_manager_start_playlist(AudioManager *am, const char *initial_file) {
    if (am->is_playing) {
        audio_manager_stop(am);
    }

    if (g_strcmp0(initial_file, TRACKS[1]) == 0) {
        am->current_track = 1;
    } 
    
    else {
        am->current_track = 0;
    }

    g_print("Iniciando playlist com: %s\n", TRACKS[am->current_track]);

    gst_element_set_state(am->pipeline, GST_STATE_READY);

    gchar *absolute_path = g_canonicalize_filename(TRACKS[am->current_track], NULL);
    gchar *uri = g_filename_to_uri(absolute_path, NULL, NULL);
    
    g_object_set(G_OBJECT(am->pipeline), "uri", uri, NULL);
    g_object_set(G_OBJECT(am->pipeline), "volume", am->current_volume, NULL);

    // Libera a memória alocada pelas strings da GLib:
    g_free(absolute_path);
    g_free(uri);

    gst_element_set_state(am->pipeline, GST_STATE_PLAYING);
    am->is_playing = true;
}


void audio_manager_stop(AudioManager *am) {
    if (!am->is_playing) {
        return;
    }
    
    g_print("Parando a reprodução.\n");
    gst_element_set_state(am->pipeline, GST_STATE_NULL);
    am->is_playing = false;
}


void audio_manager_set_volume(AudioManager *am, int volume_percent) {
    if (volume_percent < 0) {
        volume_percent = 0;
    } 

    if (volume_percent > 100) {
        volume_percent = 100;
    }

    // GStreamer aceita volume linear onde 1.0 = 100%:
    am->current_volume = (double)volume_percent / 100.0;
    
    g_print("Alterando volume para: %d%%\n", volume_percent);
    g_object_set(G_OBJECT(am->pipeline), "volume", am->current_volume, NULL);
}


void audio_manager_cleanup(AudioManager *am) {
    audio_manager_stop(am);
    gst_object_unref(am->pipeline);
}
