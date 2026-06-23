#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_event.h"
#include "nvs_flash.h"
#include "test_support.h"

void app_main(void);

static void assert_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void test_app_main_initializes_subsystems_and_starts_server(void) {
    test_support_reset();

    app_main();

    assert_true(g_test_state.nvs_flash_init_call_count == 1, "nvs init should run once on happy path");
    assert_true(g_test_state.nvs_flash_erase_call_count == 0, "nvs erase should not run on happy path");
    assert_true(g_test_state.xtask_create_call_count == 1, "display task should be created once");
    assert_true(strcmp(g_test_state.last_task_name, "display_init") == 0, "display task name should match");
    assert_true(g_test_state.last_task_stack_depth == 8192, "display task stack should match");
    assert_true(g_test_state.last_task_priority == 5, "display task priority should match");
    assert_true(g_test_state.wifi_init_simple_call_count == 1, "wifi init should be triggered once");
    assert_true(
        g_test_state.esp_event_handler_register_call_count == 1,
        "IP event handler should be registered once"
    );
    assert_true(
        g_test_state.last_event_base != NULL && strcmp(g_test_state.last_event_base, IP_EVENT) == 0,
        "registered event base should be IP_EVENT"
    );
    assert_true(
        g_test_state.last_event_id == IP_EVENT_STA_GOT_IP,
        "registered event id should be IP_EVENT_STA_GOT_IP"
    );
    assert_true(g_test_state.last_event_handler != NULL, "event handler callback should be captured");
    assert_true(g_test_state.vtask_delay_call_count == 1, "main flow should wait once for wifi");
    assert_true(g_test_state.last_delay_ticks == 8000, "wifi wait should match firmware delay");
    assert_true(g_test_state.start_web_server_call_count == 1, "web server should start once");
}

static void test_app_main_recovers_from_nvs_full_pages_error(void) {
    test_support_reset();
    g_test_state.next_nvs_flash_init_results[0] = ESP_ERR_NVS_NO_FREE_PAGES;
    g_test_state.next_nvs_flash_init_results[1] = ESP_OK;
    g_test_state.next_nvs_flash_init_result_count = 2;

    app_main();

    assert_true(g_test_state.nvs_flash_init_call_count == 2, "nvs init should retry after erase");
    assert_true(g_test_state.nvs_flash_erase_call_count == 1, "nvs erase should run on recoverable error");
}

static void test_display_task_initializes_display_and_sets_default_color(void) {
    test_support_reset();

    app_main();

    assert_true(g_test_state.last_created_task_func != NULL, "display task function should be captured");
    g_test_state.last_created_task_func(g_test_state.last_created_task_arg);

    assert_true(g_test_state.display_init_call_count == 1, "display should initialize inside task");
    assert_true(
        g_test_state.display_set_bg_color_call_count == 1,
        "display task should set initial background color"
    );
    assert_true(g_test_state.last_display_r == 255, "display red channel should start at 255");
    assert_true(g_test_state.last_display_g == 0, "display green channel should start at 0");
    assert_true(g_test_state.last_display_b == 255, "display blue channel should start at 255");
    assert_true(g_test_state.vtask_delete_call_count == 1, "display task should delete itself");
}

int main(void) {
    test_app_main_initializes_subsystems_and_starts_server();
    test_app_main_recovers_from_nvs_full_pages_error();
    test_display_task_initializes_display_and_sets_default_color();
    puts("tela main tests passed");
    return 0;
}
