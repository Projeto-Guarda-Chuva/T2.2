#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_http_server.h"
#include "http_server.h"
#include "test_support.h"

static void assert_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void test_start_web_server_registers_cmd_post_endpoint(void) {
    test_support_reset();

    start_web_server();

    assert_true(g_test_state.httpd_start_call_count == 1, "server should start once");
    assert_true(g_test_state.httpd_register_call_count == 1, "server should register one uri");
    assert_true(strcmp(g_test_state.last_registered_uri.uri, "/cmd") == 0, "uri should be /cmd");
    assert_true(g_test_state.last_registered_uri.method == HTTP_POST, "uri should be POST");
    assert_true(g_test_state.last_registered_uri.handler != NULL, "handler should be registered");
}

static void test_registered_handler_processes_valid_payload(void) {
    httpd_req_t req = {.content_len = 11, .user_ctx = NULL};

    test_support_reset();
    start_web_server();
    strcpy(g_test_state.next_request_body, "{\"id\": 13}");

    assert_true(
        g_test_state.last_registered_uri.handler(&req) == ESP_OK,
        "handler should return success for valid request"
    );
    assert_true(
        g_test_state.process_command_json_call_count == 1,
        "handler should forward payload to protocol processor"
    );
    assert_true(
        strcmp(g_test_state.last_process_command_json, "{\"id\": 13}") == 0,
        "handler should preserve request body"
    );
    assert_true(
        strcmp(g_test_state.last_response_body, "{\"status\": \"ok\"}") == 0,
        "handler should send ok response"
    );
}

static void test_registered_handler_rejects_oversized_payload(void) {
    httpd_req_t req = {.content_len = 200, .user_ctx = NULL};

    test_support_reset();
    start_web_server();

    assert_true(
        g_test_state.last_registered_uri.handler(&req) == ESP_FAIL,
        "handler should fail for oversized request"
    );
    assert_true(
        g_test_state.httpd_resp_send_500_call_count == 1,
        "handler should send 500 for oversized request"
    );
}

int main(void) {
    test_start_web_server_registers_cmd_post_endpoint();
    test_registered_handler_processes_valid_payload();
    test_registered_handler_rejects_oversized_payload();
    puts("http_server tests passed");
    return 0;
}
