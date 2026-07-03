#include "audio_manager.h"

#include <gst/gst.h>

#include <glib.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GstElement *player = NULL;

static audio_state_t state = AUDIO_IDLE;

static uint8_t current_volume = 100;

static char current_file[PATH_MAX] = "";

static char last_error[256] = "";


static void audio_set_error(const char *msg) {
    snprintf(last_error, sizeof(last_error), "%s", msg);
    state = AUDIO_ERROR;
}


bool audio_init(void) {
    if(player) {
        return true;
    }

    gst_init(NULL, NULL);
    player = gst_element_factory_make("playbin", "audio-player");

    if(player == NULL) {
        audio_set_error("Não foi possível criar o playbin!");
        return false;
    }

    g_object_set(player, "volume", (gdouble)current_volume / 100.0, NULL);
    state = AUDIO_IDLE;

    current_file[0] = '\0';
    last_error[0] = '\0';

    return true;
}


void audio_deinit(void) {
    if(player == NULL) {
        return;
    }

    gst_element_set_state(player, GST_STATE_NULL);
    gst_object_unref(player);
    player = NULL;

    current_file[0] = '\0';
    last_error[0] = '\0';

    state = AUDIO_IDLE;
}


bool audio_play(const char *file) {
    if(player == NULL) {
        audio_set_error("Player não inicializado!");
        return false;
    }

    if(file == NULL || strlen(file) == 0) {
        audio_set_error("Nome do arquivo inválido!");
        return false;
    }

    if(audio_is_playing()) {
        audio_stop();
    }

    char absolute_path[PATH_MAX];
    if(realpath(file, absolute_path) == NULL) {
        audio_set_error("Arquivo não encontrado!");
        return false;
    }

    gchar *uri = gst_filename_to_uri(absolute_path, NULL);
    if(uri == NULL) {
        audio_set_error("Não foi possível criar a URI!");
        return false;
    }

    snprintf(current_file, sizeof(current_file), "%s", absolute_path);
    g_object_set(player, "uri", uri, NULL);
    g_free(uri);

    GstStateChangeReturn ret;
    ret = gst_element_set_state(player, GST_STATE_PLAYING);

    if(ret == GST_STATE_CHANGE_FAILURE) {
        audio_set_error("Falha ao iniciar reprodução!");
        return false;
    }

    state = AUDIO_PLAYING;
    return true;
}


void audio_stop(void) {
    if(player == NULL) {
        return;
    }

    gst_element_set_state(player, GST_STATE_NULL);
    current_file[0] = '\0';
    state = AUDIO_STOPPED;
}


bool audio_set_volume(uint8_t volume) {
    if(player == NULL) {
        audio_set_error("Player não inicializado.");
        return false;
    }

    if(volume > 100) {
        volume = 100;
    }

    current_volume = volume;
    g_object_set(player, "volume", (gdouble)volume / 100.0, NULL);

    return true;
}


audio_state_t audio_get_state(void) {
    return state;
}


bool audio_is_playing(void) {
    if(player == NULL) {
        return false;
    }

    GstState gst_state;
    gst_element_get_state(player, &gst_state, NULL, 0);

    return gst_state == GST_STATE_PLAYING;
}


uint8_t audio_get_volume(void) {
    return current_volume;
}


const char *audio_get_current_file(void) {
    return current_file;
}


const char *audio_get_last_error(void) {
    return last_error;
}