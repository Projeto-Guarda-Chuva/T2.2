#include "display_manager.h"
#include "fake_display.h"

display_spy_t g_display_spy;

void display_spy_reset(void) {
    g_display_spy.call_count = 0;
    g_display_spy.last_r = 0;
    g_display_spy.last_g = 0;
    g_display_spy.last_b = 0;
}

void display_set_bg_color(uint8_t r, uint8_t g, uint8_t b) {
    g_display_spy.call_count++;
    g_display_spy.last_r = r;
    g_display_spy.last_g = g;
    g_display_spy.last_b = b;
}
