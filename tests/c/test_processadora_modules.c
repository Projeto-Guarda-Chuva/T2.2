#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print.h"
#include "http_comm.h"
#include "monitor.h"
#include "test_support.h"

static void assert_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void test_monitor_init_registers_monitor_endpoint(void) {
    test_support_reset();

    monitor_init();

    assert_true(
        g_test_state.register_get_endpoint_call_count == 1,
        "monitor_init should register one GET endpoint"
    );
    assert_true(
        strcmp(g_test_state.last_register_get_uri, "/monitor") == 0,
        "monitor_init should register /monitor"
    );
    assert_true(
        g_test_state.last_register_get_callback != NULL,
        "monitor_init should provide a callback"
    );
}

static void test_print_task_init_creates_queue_registers_endpoint_and_task(void) {
    test_support_reset();

    print_task_init();

    assert_true(g_test_state.xqueue_create_call_count == 1, "print should create queue");
    assert_true(g_test_state.last_queue_length == 10, "print queue length should be 10");
    assert_true(
        g_test_state.last_queue_item_size == sizeof(char *),
        "print queue item size should match char*"
    );
    assert_true(
        g_test_state.register_post_endpoint_call_count == 1,
        "print should register one POST endpoint"
    );
    assert_true(
        strcmp(g_test_state.last_register_post_uri, "/print") == 0,
        "print should register /print"
    );
    assert_true(g_test_state.xtask_create_call_count == 1, "print should create a task");
    assert_true(
        strcmp(g_test_state.last_task_name, "print_task") == 0,
        "print task name should be print_task"
    );
}

static void test_print_task_init_stops_when_queue_creation_fails(void) {
    test_support_reset();
    g_test_state.next_queue_create_result = NULL;

    print_task_init();

    assert_true(
        g_test_state.register_post_endpoint_call_count == 0,
        "print should not register endpoint when queue creation fails"
    );
    assert_true(
        g_test_state.xtask_create_call_count == 0,
        "print should not create task when queue creation fails"
    );
}

int main(void) {
    test_monitor_init_registers_monitor_endpoint();
    test_print_task_init_creates_queue_registers_endpoint_and_task();
    test_print_task_init_stops_when_queue_creation_fails();
    puts("processadora module tests passed");
    return 0;
}
