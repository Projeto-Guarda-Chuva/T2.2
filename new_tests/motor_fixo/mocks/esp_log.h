#pragma once

typedef int esp_log_level_t;

#define ESP_LOG_ERROR   'E'
#define ESP_LOG_WARN    'W'
#define ESP_LOG_INFO    'I'

void esp_log_write(esp_log_level_t level, const char *tag, const char *fmt, ...);

#define ESP_LOGE(tag, fmt, ...) esp_log_write(ESP_LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) esp_log_write(ESP_LOG_WARN, tag, fmt, ##__VA_ARGS__)
#define ESP_LOGI(tag, fmt, ...) esp_log_write(ESP_LOG_INFO, tag, fmt, ##__VA_ARGS__)
