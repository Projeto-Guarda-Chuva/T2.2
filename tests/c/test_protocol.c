#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "protocol.h"

static int display_call_count;
static uint8_t last_r;
static uint8_t last_g;
static uint8_t last_b;

void display_set_bg_color(uint8_t r, uint8_t g, uint8_t b) {
    display_call_count++;
    last_r = r;
    last_g = g;
    last_b = b;
}

static void reset_display_spy(void) {
    display_call_count = 0;
    last_r = 0;
    last_g = 0;
    last_b = 0;
}

static void assert_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void assert_color(uint8_t r, uint8_t g, uint8_t b, const char *message) {
    assert_true(display_call_count == 1, message);
    assert_true(last_r == r && last_g == g && last_b == b, message);
}

static void test_invalid_json_does_not_change_display(void) {
    reset_display_spy();
    process_command_json("not-json");
    assert_true(display_call_count == 0, "invalid json should not update display");
}

static void test_unknown_id_does_not_change_display(void) {
    reset_display_spy();
    process_command_json("{\"id\":99}");
    assert_true(display_call_count == 0, "unknown id should not update display");
}

static void test_predefined_red_color(void) {
    reset_display_spy();
    process_command_json("{\"id\":11}");
    assert_color(255, 0, 0, "id 11 should set red");
}

static void test_predefined_white_color(void) {
    reset_display_spy();
    process_command_json("{\"id\":15}");
    assert_color(255, 255, 255, "id 15 should set white");
}

static void test_custom_rgb_color(void) {
    reset_display_spy();
    process_command_json("{\"id\":10,\"r\":12,\"g\":34,\"b\":56}");
    assert_color(12, 34, 56, "id 10 should use provided rgb");
}

static void test_custom_rgb_defaults_missing_fields_to_zero(void) {
    reset_display_spy();
    process_command_json("{\"id\":10,\"r\":200}");
    assert_color(200, 0, 0, "missing custom rgb fields should default to zero");
}

int main(void) {
    test_invalid_json_does_not_change_display();
    test_unknown_id_does_not_change_display();
    test_predefined_red_color();
    test_predefined_white_color();
    test_custom_rgb_color();
    test_custom_rgb_defaults_missing_fields_to_zero();

    puts("protocol tests passed");
    return 0;
}
