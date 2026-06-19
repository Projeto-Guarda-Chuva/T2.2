#include "test_support.h"

#include <string.h>

QueueHandle_t xQueueCreate(unsigned int queue_length, unsigned int item_size) {
    g_test_state.xqueue_create_call_count++;
    g_test_state.last_queue_length = queue_length;
    g_test_state.last_queue_item_size = item_size;
    return g_test_state.next_queue_create_result;
}

BaseType_t xQueueSend(QueueHandle_t queue, const void *item, TickType_t ticks_to_wait) {
    (void)queue;
    (void)item;
    (void)ticks_to_wait;
    g_test_state.xqueue_send_call_count++;
    return g_test_state.next_xqueue_send_result;
}

BaseType_t xQueueReceive(QueueHandle_t queue, void *buffer, TickType_t ticks_to_wait) {
    (void)queue;
    (void)buffer;
    (void)ticks_to_wait;
    return 0;
}

BaseType_t xTaskCreate(
    void (*task_func)(void *),
    const char *name,
    unsigned int stack_depth,
    void *params,
    unsigned int priority,
    TaskHandle_t *created_task
) {
    (void)task_func;
    (void)params;
    (void)created_task;
    g_test_state.xtask_create_call_count++;
    strncpy(g_test_state.last_task_name, name, sizeof(g_test_state.last_task_name) - 1);
    g_test_state.last_task_stack_depth = stack_depth;
    g_test_state.last_task_priority = priority;
    return pdPASS;
}

void vTaskDelay(TickType_t ticks_to_delay) {
    (void)ticks_to_delay;
}
