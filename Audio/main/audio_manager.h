#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <stdbool.h>
#include <stdint.h>


typedef enum {
    AUDIO_IDLE,
    AUDIO_REQUESTED,
    AUDIO_PLAYING,
    AUDIO_STOPPED,
    AUDIO_ERROR
} audio_state_t;


void audio_init(void);

bool audio_request_play(const char *file);

bool audio_request_stop(void);

bool audio_request_volume(uint8_t volume);

audio_state_t audio_get_state(void);

uint8_t audio_get_volume(void);

const char *audio_get_last_file(void);

#endif
