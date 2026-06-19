#ifndef LIGHTING_MANAGER_H
#define LIGHTING_MANAGER_H

#include <stdint.h>

void lighting_init(void);

void lighting_set_intensity(uint8_t intensity);

void lighting_set_color(uint8_t r, uint8_t g, uint8_t b);

void lighting_off(void);

#endif