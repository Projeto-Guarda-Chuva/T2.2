#include "test_support.h"

#include <stdint.h>

void wifi_init_simple(void) {
    g_test_state.wifi_init_simple_call_count++;
}

void start_web_server(void) {
    g_test_state.start_web_server_call_count++;
}

void display_init(void) {
    g_test_state.display_init_call_count++;
}

void display_set_bg_color(uint8_t r, uint8_t g, uint8_t b) {
    g_test_state.display_set_bg_color_call_count++;
    g_test_state.last_display_r = r;
    g_test_state.last_display_g = g;
    g_test_state.last_display_b = b;
}

void lighting_init(void) {
    g_test_state.lighting_init_call_count++;
}

void lighting_set_color(uint8_t r, uint8_t g, uint8_t b) {
    int index = g_test_state.lighting_set_color_call_count;
    g_test_state.lighting_set_color_call_count++;
    g_test_state.last_lighting_r = r;
    g_test_state.last_lighting_g = g;
    g_test_state.last_lighting_b = b;
    if (index < (int)(sizeof(g_test_state.lighting_r_history) / sizeof(g_test_state.lighting_r_history[0]))) {
        g_test_state.lighting_r_history[index] = r;
        g_test_state.lighting_g_history[index] = g;
        g_test_state.lighting_b_history[index] = b;
    }
}
