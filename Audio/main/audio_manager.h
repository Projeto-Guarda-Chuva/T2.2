#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    AUDIO_IDLE = 0,
    AUDIO_PLAYING,
    AUDIO_STOPPED,
    AUDIO_ERROR
} audio_state_t;

bool audio_init(void);

void audio_deinit(void);

bool audio_play(const char *file);

void audio_stop(void);

bool audio_set_volume(uint8_t volume);

audio_state_t audio_get_state(void);

bool audio_is_playing(void);

uint8_t audio_get_volume(void);

const char *audio_get_current_file(void);

const char *audio_get_last_error(void);

#endif