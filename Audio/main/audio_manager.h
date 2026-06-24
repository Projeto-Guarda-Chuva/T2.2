#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <stdint.h>

void audio_init(void);

void audio_play(void);

void audio_stop(void);

void audio_set_volume(uint8_t volume);

#endif