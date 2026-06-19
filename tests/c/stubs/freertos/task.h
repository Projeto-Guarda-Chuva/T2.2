#pragma once

#include "freertos/FreeRTOS.h"

typedef void *TaskHandle_t;

BaseType_t xTaskCreate(
    void (*task_func)(void *),
    const char *name,
    unsigned int stack_depth,
    void *params,
    unsigned int priority,
    TaskHandle_t *created_task
);
void vTaskDelay(TickType_t ticks_to_delay);
