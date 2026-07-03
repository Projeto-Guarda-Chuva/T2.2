#include "test_log_capture.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

test_log_state_t g_test_log;

void test_log_reset(void) {
    g_test_log.count = 0;
}

void test_log_record(char level, const char *fmt, ...) {
    if (g_test_log.count >= TEST_LOG_MAX) {
        return;
    }

    test_log_entry_t *entry = &g_test_log.entries[g_test_log.count];
    entry->level = level;

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(entry->msg, TEST_LOG_MSG_LEN, fmt, ap);
    va_end(ap);

    g_test_log.count++;
}

int test_log_count_level(char level) {
    int n = 0;
    for (int i = 0; i < g_test_log.count; i++) {
        if (g_test_log.entries[i].level == level) {
            n++;
        }
    }
    return n;
}

int test_log_contains(const char *substr) {
    for (int i = 0; i < g_test_log.count; i++) {
        if (strstr(g_test_log.entries[i].msg, substr) != NULL) {
            return 1;
        }
    }
    return 0;
}
