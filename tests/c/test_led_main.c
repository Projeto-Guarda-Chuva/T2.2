#include <stdio.h>
#include <stdlib.h>

#include "test_support.h"

void app_main(void);

static void assert_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void test_led_app_main_initializes_lighting_and_cycles_rgb_colors(void) {
    test_support_reset();

    app_main();

    assert_true(g_test_state.lighting_init_call_count == 1, "lighting should initialize once");
    assert_true(g_test_state.lighting_set_color_call_count == 3, "lighting should set three colors");
    assert_true(
        g_test_state.lighting_r_history[0] == 255 &&
        g_test_state.lighting_g_history[0] == 0 &&
        g_test_state.lighting_b_history[0] == 0,
        "first color should be red"
    );
    assert_true(
        g_test_state.lighting_r_history[1] == 0 &&
        g_test_state.lighting_g_history[1] == 255 &&
        g_test_state.lighting_b_history[1] == 0,
        "second color should be green"
    );
    assert_true(
        g_test_state.lighting_r_history[2] == 0 &&
        g_test_state.lighting_g_history[2] == 0 &&
        g_test_state.lighting_b_history[2] == 255,
        "third color should be blue"
    );
    assert_true(g_test_state.vtask_delay_call_count == 2, "app should wait between the first two transitions");
    assert_true(g_test_state.recorded_delay_count == 2, "two delays should be recorded");
    assert_true(g_test_state.recorded_delays[0] == 3000, "first delay should be 3000 ms");
    assert_true(g_test_state.recorded_delays[1] == 3000, "second delay should be 3000 ms");
}

int main(void) {
    test_led_app_main_initializes_lighting_and_cycles_rgb_colors();
    puts("led main tests passed");
    return 0;
}
