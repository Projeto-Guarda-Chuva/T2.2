#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <stdbool.h>
#include <stdint.h>


typedef enum {
    AUDIO_STOPPED,
    AUDIO_PLAYING,
    AUDIO_ERROR
} audio_state_t;


void audio_init(void);

bool audio_is_playing(void);

void audio_play_default(void);

void audio_play(const char *file);

void audio_stop(void);

void audio_set_volume(uint8_t volume);

#endif
