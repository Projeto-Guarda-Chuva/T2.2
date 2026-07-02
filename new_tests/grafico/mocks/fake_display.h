#pragma once

#include <stdint.h>

typedef struct {
    int     call_count;
    uint8_t last_r;
    uint8_t last_g;
    uint8_t last_b;
} display_spy_t;

extern display_spy_t g_display_spy;

void display_spy_reset(void);
