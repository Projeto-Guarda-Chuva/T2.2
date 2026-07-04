#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <glib.h>
#include <gst/gst.h>
#include <stdbool.h>

typedef struct {
    GstElement *pipeline;
    GstElement *source;
    GstElement *volume_element;
    GMainLoop *loop;
    
    bool is_playing;
    int current_track; // 0 para Água Viva, 1 para Pôr do Sol
    double current_volume; // 0.0 a 1.0
} AudioManager;

void audio_manager_init(AudioManager *am, GMainLoop *loop);

void audio_manager_start_playlist(AudioManager *am, const char *initial_file);

void audio_manager_stop(AudioManager *am);

void audio_manager_set_volume(AudioManager *am, int volume_percent);

void audio_manager_cleanup(AudioManager *am);

#endif
