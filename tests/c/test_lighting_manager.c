#include <stdio.h>
#include <stdlib.h>

#include "lighting_manager.h"
#include "test_support.h"

static void assert_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void test_lighting_init_initializes_led_strip_once(void) {
    test_support_reset();

    lighting_init();
    lighting_init();

    assert_true(
        g_test_state.led_strip_new_device_call_count == 1,
        "lighting_init should create led strip only once"
    );
}

static void test_lighting_set_color_updates_all_leds_and_refreshes(void) {
    test_support_reset();
    lighting_init();

    lighting_set_color(10, 20, 30);

    assert_true(
        g_test_state.led_strip_set_pixel_call_count == 24,
        "lighting_set_color should update all 24 leds"
    );
    assert_true(g_test_state.last_pixel_index == 23, "last updated led should be index 23");
    assert_true(g_test_state.last_pixel_r == 10, "red component should match");
    assert_true(g_test_state.last_pixel_g == 20, "green component should match");
    assert_true(g_test_state.last_pixel_b == 30, "blue component should match");
    assert_true(g_test_state.led_strip_refresh_call_count == 1, "refresh should be called once");
}

static void test_lighting_set_intensity_reapplies_scaled_color(void) {
    test_support_reset();
    lighting_init();
    lighting_set_color(200, 100, 50);
    g_test_state.led_strip_set_pixel_call_count = 0;
    g_test_state.led_strip_refresh_call_count = 0;

    lighting_set_intensity(128);

    assert_true(
        g_test_state.led_strip_set_pixel_call_count == 24,
        "changing intensity should repaint all leds"
    );
    assert_true(g_test_state.last_pixel_r == 100, "scaled red should be applied");
    assert_true(g_test_state.last_pixel_g == 50, "scaled green should be applied");
    assert_true(g_test_state.last_pixel_b == 25, "scaled blue should be applied");
}

static void test_lighting_off_clears_strip_and_refreshes(void) {
    test_support_reset();
    lighting_init();

    lighting_off();

    assert_true(g_test_state.led_strip_clear_call_count == 1, "lighting_off should clear strip");
    assert_true(g_test_state.led_strip_refresh_call_count == 1, "lighting_off should refresh strip");
}

int main(void) {
    test_lighting_init_initializes_led_strip_once();
    test_lighting_set_color_updates_all_leds_and_refreshes();
    test_lighting_set_intensity_reapplies_scaled_color();
    test_lighting_off_clears_strip_and_refreshes();
    puts("lighting_manager tests passed");
    return 0;
}
