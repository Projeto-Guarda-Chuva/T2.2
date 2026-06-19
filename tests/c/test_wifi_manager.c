#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_support.h"
#include "wifi_manager.h"

static void assert_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void test_wifi_init_simple_initializes_station_mode_with_expected_credentials(void) {
    test_support_reset();

    wifi_init_simple();

    assert_true(g_test_state.esp_netif_init_call_count == 1, "esp_netif_init should be called once");
    assert_true(
        g_test_state.esp_event_loop_create_default_call_count == 1,
        "event loop should be created once"
    );
    assert_true(
        g_test_state.esp_netif_create_default_wifi_sta_call_count == 1,
        "default wifi sta should be created once"
    );
    assert_true(g_test_state.esp_wifi_init_call_count == 1, "esp_wifi_init should be called once");
    assert_true(g_test_state.esp_wifi_set_mode_call_count == 1, "wifi mode should be configured once");
    assert_true(g_test_state.last_wifi_mode == WIFI_MODE_STA, "wifi mode should be station");
    assert_true(
        g_test_state.esp_wifi_set_config_call_count == 1,
        "wifi config should be configured once"
    );
    assert_true(
        g_test_state.last_wifi_interface_id == WIFI_IF_STA,
        "wifi config should target station interface"
    );
    assert_true(
        strcmp(g_test_state.last_wifi_config.sta.ssid, "StandByMe") == 0,
        "ssid should match firmware configuration"
    );
    assert_true(
        strcmp(g_test_state.last_wifi_config.sta.password, "saojoaodelrei2025") == 0,
        "password should match firmware configuration"
    );
    assert_true(g_test_state.esp_wifi_start_call_count == 1, "wifi start should be called once");
    assert_true(g_test_state.esp_wifi_connect_call_count == 1, "wifi connect should be called once");
}

int main(void) {
    test_wifi_init_simple_initializes_station_mode_with_expected_credentials();
    puts("wifi_manager tests passed");
    return 0;
}
