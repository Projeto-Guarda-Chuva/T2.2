#include "esp_log.h"
#include "test_log_capture.h"
#include <stdarg.h>
#include <stdio.h>

void esp_log_write(esp_log_level_t level, const char *tag, const char *fmt, ...)
{
    (void)tag;

    va_list args;
    va_start(args, fmt);
    char msg[256];
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    char log_level = 'I';
    if (level == ESP_LOG_ERROR) {
        log_level = 'E';
    } else if (level == ESP_LOG_WARN) {
        log_level = 'W';
    }

    test_log_record(log_level, "%s", msg);
}
