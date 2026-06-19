#include "test_support.h"

#include <string.h>

#include "http_comm.h"

esp_err_t register_POST_endpoint_queue(const char *uri, QueueHandle_t queue) {
    g_test_state.register_post_endpoint_call_count++;
    strncpy(
        g_test_state.last_register_post_uri,
        uri,
        sizeof(g_test_state.last_register_post_uri) - 1
    );
    g_test_state.last_register_post_queue = queue;
    return ESP_OK;
}

esp_err_t register_GET_endpoint_callback(const char *uri, http_get_data_provider_t func) {
    g_test_state.register_get_endpoint_call_count++;
    strncpy(
        g_test_state.last_register_get_uri,
        uri,
        sizeof(g_test_state.last_register_get_uri) - 1
    );
    g_test_state.last_register_get_callback = func;
    return ESP_OK;
}
