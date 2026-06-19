#pragma once
#include <stdbool.h>
#include <stdint.h>

void display_init(void);
bool display_lock(uint32_t timeout_ms);
void display_unlock(void);
void display_set_bg_color(uint8_t r, uint8_t g, uint8_t b);