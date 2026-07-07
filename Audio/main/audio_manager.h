#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#ifndef TEST_ENV
#include <glib.h>
#include <gst/gst.h>
#endif

typedef struct {
#ifndef TEST_ENV
    GstElement *pipeline;
    GstElement *source;
    GstElement *volume_element;
    GMainLoop *loop;
#else
    void *pipeline;
    void *source;
    void *volume_element;
    void *loop;
#endif
    bool is_playing;
    int current_track; 
    double current_volume; 
} AudioManager;

void audio_manager_init(AudioManager *am, void *loop);

void audio_manager_start_playlist(AudioManager *am, const char *initial_file);

void audio_manager_stop(AudioManager *am);

void audio_manager_set_volume(AudioManager *am, int volume_percent);

void audio_manager_cleanup(AudioManager *am);

void audio_init(void);

void audio_play(const char *filename);

void audio_play_default(void);

void audio_stop(void);

bool audio_is_playing(void);

void audio_set_volume(int8_t volume);

#endif
