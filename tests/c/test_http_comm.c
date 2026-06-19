#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http_comm.h"
#include "test_support.h"

static char *fake_provider(void) {
    return NULL;
}

static void assert_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void test_register_post_requires_started_server(void) {
    test_support_reset();
    assert_true(
        register_POST_endpoint_queue("/cmd", (QueueHandle_t)0x11) == ESP_ERR_INVALID_STATE,
        "POST registration should fail before server start"
    );
}

static void test_http_server_start_starts_only_once(void) {
    test_support_reset();

    assert_true(http_server_start() == ESP_OK, "server start should succeed first time");
    assert_true(http_server_start() == ESP_OK, "server start should succeed second time");
    assert_true(
        g_test_state.httpd_start_call_count == 1,
        "server should start only once"
    );
}

static void test_register_post_registers_uri_handler(void) {
    test_support_reset();
    http_server_start();

    assert_true(
        register_POST_endpoint_queue("/cmd", (QueueHandle_t)0x22) == ESP_OK,
        "POST registration should succeed after start"
    );
    assert_true(
        g_test_state.httpd_register_call_count == 1,
        "POST registration should call httpd_register_uri_handler"
    );
    assert_true(
        strcmp(g_test_state.last_registered_uri.uri, "/cmd") == 0,
        "POST registration should keep uri"
    );
    assert_true(
        g_test_state.last_registered_uri.method == HTTP_POST,
        "POST registration should use HTTP_POST"
    );
}

static void test_register_get_registers_uri_handler(void) {
    test_support_reset();
    http_server_start();

    assert_true(
        register_GET_endpoint_callback("/status", fake_provider) == ESP_OK,
        "GET registration should succeed after start"
    );
    assert_true(
        g_test_state.httpd_register_call_count == 1,
        "GET registration should call httpd_register_uri_handler"
    );
    assert_true(
        strcmp(g_test_state.last_registered_uri.uri, "/status") == 0,
        "GET registration should keep uri"
    );
    assert_true(
        g_test_state.last_registered_uri.method == HTTP_GET,
        "GET registration should use HTTP_GET"
    );
}

static void test_post_command_builds_url_and_sets_text_header(void) {
    test_support_reset();

    assert_true(
        POST_command("192.168.4.1", "/print", "hello") == ESP_OK,
        "POST_command should succeed"
    );
    assert_true(
        strcmp(g_test_state.last_http_client_config.url, "http://192.168.4.1/print") == 0,
        "POST_command should build correct URL"
    );
    assert_true(
        strcmp(g_test_state.last_header_key, "Content-Type") == 0 &&
        strcmp(g_test_state.last_header_value, "text/plain") == 0,
        "POST_command should set text/plain content type"
    );
    assert_true(
        strcmp(g_test_state.last_post_field, "hello") == 0,
        "POST_command should send original message"
    );
}

static void test_get_command_reads_response_and_null_terminates(void) {
    char buffer[32];

    test_support_reset();
    strcpy(g_test_state.read_response_data, "{\"ok\":true}");
    memset(buffer, 'X', sizeof(buffer));

    assert_true(
        GET_command("10.0.0.1", "/status", buffer, sizeof(buffer) - 1) == ESP_OK,
        "GET_command should succeed"
    );
    assert_true(
        strcmp(g_test_state.last_http_client_config.url, "http://10.0.0.1/status") == 0,
        "GET_command should build correct URL"
    );
    assert_true(
        strcmp(buffer, "{\"ok\":true}") == 0,
        "GET_command should copy response to buffer"
    );
}

int main(void) {
    test_register_post_requires_started_server();
    test_http_server_start_starts_only_once();
    test_register_post_registers_uri_handler();
    test_register_get_registers_uri_handler();
    test_post_command_builds_url_and_sets_text_header();
    test_get_command_reads_response_and_null_terminates();
    puts("http_comm tests passed");
    return 0;
}
