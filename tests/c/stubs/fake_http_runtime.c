#include "test_support.h"

#include <string.h>

esp_err_t httpd_start(httpd_handle_t *handle, httpd_config_t *config) {
    g_test_state.httpd_start_call_count++;
    if (handle != NULL) {
        *handle = (httpd_handle_t)0xCAFE;
    }
    (void)config;
    return ESP_OK;
}

esp_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t *uri_handler) {
    (void)handle;
    g_test_state.httpd_register_call_count++;
    if (uri_handler != NULL) {
        g_test_state.last_registered_uri = *uri_handler;
    }
    return ESP_OK;
}

esp_err_t httpd_resp_send(httpd_req_t *req, const char *buf, int buf_len) {
    (void)req;
    (void)buf_len;
    g_test_state.httpd_resp_send_call_count++;
    strncpy(g_test_state.last_response_body, buf, sizeof(g_test_state.last_response_body) - 1);
    return ESP_OK;
}

esp_err_t httpd_resp_sendstr(httpd_req_t *req, const char *str) {
    return httpd_resp_send(req, str, HTTPD_RESP_USE_STRLEN);
}

esp_err_t httpd_resp_send_err(httpd_req_t *req, int error, const char *message) {
    (void)req;
    (void)error;
    (void)message;
    return ESP_OK;
}

esp_err_t httpd_resp_send_500(httpd_req_t *req) {
    (void)req;
    g_test_state.httpd_resp_send_500_call_count++;
    return ESP_FAIL;
}

int httpd_req_recv(httpd_req_t *req, char *buf, size_t buf_len) {
    size_t len;
    (void)req;
    len = strlen(g_test_state.next_request_body);
    if (len > buf_len) {
        len = buf_len;
    }
    memcpy(buf, g_test_state.next_request_body, len);
    return g_test_state.next_httpd_req_recv_result ? g_test_state.next_httpd_req_recv_result : (int)len;
}
