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


bool audio_set_volume(uint8_t volume) {
    if(player == NULL) {
        return false;
    }

    if(volume > 100) {
        volume = 100;
    }

    current_volume = volume;
    g_object_set(player, "volume", volume / 100.0, NULL);
    return true;
}


audio_state_t audio_get_state(void) {
    return state;
}


bool audio_is_playing(void) {
    return state == AUDIO_PLAYING;
}


uint8_t audio_get_volume(void) {
    return current_volume;
}


const char *audio_get_current_file(void) {
    return current_file;
}
