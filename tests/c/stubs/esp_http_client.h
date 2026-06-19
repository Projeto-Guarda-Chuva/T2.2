#pragma once

#include <stddef.h>

#include "esp_err.h"

typedef void *esp_http_client_handle_t;

typedef enum {
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_POST = 1,
} esp_http_client_method_t;

typedef struct {
    const char *url;
    esp_http_client_method_t method;
    int timeout_ms;
} esp_http_client_config_t;

esp_http_client_handle_t esp_http_client_init(const esp_http_client_config_t *config);
esp_err_t esp_http_client_set_post_field(
    esp_http_client_handle_t client,
    const char *data,
    int len
);
esp_err_t esp_http_client_set_header(
    esp_http_client_handle_t client,
    const char *key,
    const char *value
);
esp_err_t esp_http_client_perform(esp_http_client_handle_t client);
int esp_http_client_get_status_code(esp_http_client_handle_t client);
int esp_http_client_read_response(
    esp_http_client_handle_t client,
    char *buffer,
    int buffer_len
);
esp_err_t esp_http_client_cleanup(esp_http_client_handle_t client);
