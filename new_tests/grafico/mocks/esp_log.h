#pragma once

#include "test_log_capture.h"

#define ESP_LOGI(tag, fmt, ...) do { (void)(tag); test_log_record('I', fmt, ##__VA_ARGS__); } while (0)
#define ESP_LOGW(tag, fmt, ...) do { (void)(tag); test_log_record('W', fmt, ##__VA_ARGS__); } while (0)
#define ESP_LOGE(tag, fmt, ...) do { (void)(tag); test_log_record('E', fmt, ##__VA_ARGS__); } while (0)
#define ESP_LOGD(tag, fmt, ...) do { (void)(tag); test_log_record('D', fmt, ##__VA_ARGS__); } while (0)
