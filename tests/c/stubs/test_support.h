#pragma once

#include <stddef.h>

#include "freertos/queue.h"
#include "freertos/task.h"
#include "http_comm.h"
#include "esp_http_client.h"
#include "esp_http_server.h"

typedef struct {
    int httpd_start_call_count;
    int httpd_register_call_count;
    httpd_uri_t last_registered_uri;
    int http_client_init_call_count;
    esp_http_client_config_t last_http_client_config;
    int http_client_set_post_field_call_count;
    char last_post_field[256];
    int last_post_field_len;
    int http_client_set_header_call_count;
    char last_header_key[64];
    char last_header_value[64];
    int http_client_perform_call_count;
    int http_client_cleanup_call_count;
    int http_client_status_code;
    char read_response_data[256];
    int xqueue_create_call_count;
    unsigned int last_queue_length;
    unsigned int last_queue_item_size;
    QueueHandle_t next_queue_create_result;
    int xqueue_send_call_count;
    BaseType_t next_xqueue_send_result;
    int xtask_create_call_count;
    char last_task_name[64];
    unsigned int last_task_stack_depth;
    unsigned int last_task_priority;
    int register_post_endpoint_call_count;
    char last_register_post_uri[64];
    QueueHandle_t last_register_post_queue;
    int register_get_endpoint_call_count;
    char last_register_get_uri[64];
    http_get_data_provider_t last_register_get_callback;
} test_state_t;

extern test_state_t g_test_state;

void test_support_reset(void);
