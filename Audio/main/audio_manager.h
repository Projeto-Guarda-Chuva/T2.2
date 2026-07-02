#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

void audio_init(void);

bool audio_is_playing(void);

void audio_play_default(void);

void audio_play(const char *file);

void audio_stop(void);

void audio_set_volume(uint8_t volume);

#endif
