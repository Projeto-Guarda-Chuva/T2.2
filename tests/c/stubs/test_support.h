#pragma once

#include <stddef.h>

#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_wifi.h"

typedef struct {
    int httpd_start_call_count;
    int httpd_register_call_count;
    httpd_uri_t last_registered_uri;
    int httpd_resp_send_call_count;
    char last_response_body[256];
    int httpd_resp_send_500_call_count;
    char next_request_body[256];
    int next_httpd_req_recv_result;
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
    int process_command_json_call_count;
    char last_process_command_json[256];
    int led_strip_new_device_call_count;
    int led_strip_set_pixel_call_count;
    int led_strip_refresh_call_count;
    int led_strip_clear_call_count;
    int last_pixel_index;
    int last_pixel_r;
    int last_pixel_g;
    int last_pixel_b;
    int esp_netif_init_call_count;
    int esp_event_loop_create_default_call_count;
    int esp_netif_create_default_wifi_sta_call_count;
    int esp_wifi_init_call_count;
    int esp_wifi_set_mode_call_count;
    int last_wifi_mode;
    int esp_wifi_set_config_call_count;
    int last_wifi_interface_id;
    wifi_config_t last_wifi_config;
    int esp_wifi_start_call_count;
    int esp_wifi_connect_call_count;
    int nvs_flash_init_call_count;
    int nvs_flash_erase_call_count;
    esp_err_t next_nvs_flash_init_results[4];
    int next_nvs_flash_init_result_count;
    int next_nvs_flash_init_result_index;
    int esp_event_handler_register_call_count;
    esp_event_base_t last_event_base;
    int32_t last_event_id;
    esp_event_handler_t last_event_handler;
    void *last_event_handler_arg;
    int wifi_init_simple_call_count;
    int start_web_server_call_count;
    int display_init_call_count;
    int display_set_bg_color_call_count;
    int last_display_r;
    int last_display_g;
    int last_display_b;
    void (*last_created_task_func)(void *);
    void *last_created_task_arg;
    int xtask_create_pinned_to_core_call_count;
    BaseType_t last_task_core_id;
    int vtask_delay_call_count;
    TickType_t last_delay_ticks;
    TickType_t recorded_delays[8];
    int recorded_delay_count;
    int vtask_delete_call_count;
    int lighting_init_call_count;
    int lighting_set_color_call_count;
    int last_lighting_r;
    int last_lighting_g;
    int last_lighting_b;
    int lighting_r_history[8];
    int lighting_g_history[8];
    int lighting_b_history[8];
} test_state_t;

extern test_state_t g_test_state;

void test_support_reset(void);
