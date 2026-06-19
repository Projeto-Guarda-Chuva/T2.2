#pragma once

#include <stddef.h>

#include "esp_err.h"
#include "esp_http_server.h"
#include "freertos/queue.h"

typedef char *(*http_get_data_provider_t)(void);

esp_err_t http_server_start(void);
esp_err_t register_POST_endpoint_queue(const char *uri, QueueHandle_t queue);
esp_err_t register_GET_endpoint_callback(const char *uri, http_get_data_provider_t func);
esp_err_t POST_command(const char *ip, const char *path, const char *message);
esp_err_t GET_command(const char *ip, const char *path, char *response_buffer, size_t buffer_len);
