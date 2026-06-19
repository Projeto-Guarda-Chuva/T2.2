#include "test_support.h"

#include <string.h>

static int fake_status_code(void) {
    return g_test_state.http_client_status_code;
}

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
    (void)buf;
    (void)buf_len;
    return ESP_OK;
}

esp_err_t httpd_resp_send_err(httpd_req_t *req, int error, const char *message) {
    (void)req;
    (void)error;
    (void)message;
    return ESP_OK;
}

esp_err_t httpd_resp_send_500(httpd_req_t *req) {
    (void)req;
    return ESP_FAIL;
}

int httpd_req_recv(httpd_req_t *req, char *buf, size_t buf_len) {
    (void)req;
    (void)buf;
    (void)buf_len;
    return 0;
}

esp_http_client_handle_t esp_http_client_init(const esp_http_client_config_t *config) {
    g_test_state.http_client_init_call_count++;
    if (config != NULL) {
        g_test_state.last_http_client_config = *config;
    }
    return (esp_http_client_handle_t)0xBEEF;
}

esp_err_t esp_http_client_set_post_field(
    esp_http_client_handle_t client,
    const char *data,
    int len
) {
    (void)client;
    g_test_state.http_client_set_post_field_call_count++;
    g_test_state.last_post_field_len = len;
    memset(g_test_state.last_post_field, 0, sizeof(g_test_state.last_post_field));
    memcpy(g_test_state.last_post_field, data, (size_t)len);
    return ESP_OK;
}

esp_err_t esp_http_client_set_header(
    esp_http_client_handle_t client,
    const char *key,
    const char *value
) {
    (void)client;
    g_test_state.http_client_set_header_call_count++;
    strncpy(g_test_state.last_header_key, key, sizeof(g_test_state.last_header_key) - 1);
    strncpy(
        g_test_state.last_header_value,
        value,
        sizeof(g_test_state.last_header_value) - 1
    );
    return ESP_OK;
}

esp_err_t esp_http_client_perform(esp_http_client_handle_t client) {
    (void)client;
    g_test_state.http_client_perform_call_count++;
    return ESP_OK;
}

int esp_http_client_get_status_code(esp_http_client_handle_t client) {
    (void)client;
    return fake_status_code();
}

int esp_http_client_read_response(
    esp_http_client_handle_t client,
    char *buffer,
    int buffer_len
) {
    size_t len;
    (void)client;
    len = strlen(g_test_state.read_response_data);
    if ((int)len > buffer_len) {
        len = (size_t)buffer_len;
    }
    memcpy(buffer, g_test_state.read_response_data, len);
    return (int)len;
}

esp_err_t esp_http_client_cleanup(esp_http_client_handle_t client) {
    (void)client;
    g_test_state.http_client_cleanup_call_count++;
    return ESP_OK;
}
