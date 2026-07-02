#pragma once

#define TEST_LOG_MAX     64
#define TEST_LOG_MSG_LEN 256

typedef struct {
    char level;
    char msg[TEST_LOG_MSG_LEN];
} test_log_entry_t;

typedef struct {
    int count;
    test_log_entry_t entries[TEST_LOG_MAX];
} test_log_state_t;

extern test_log_state_t g_test_log;

void test_log_reset(void);
void test_log_record(char level, const char *fmt, ...);

int test_log_count_level(char level);
int test_log_contains(const char *substr);
