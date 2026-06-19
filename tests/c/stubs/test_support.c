#include "test_support.h"

#include <string.h>

test_state_t g_test_state;

void test_support_reset(void) {
    memset(&g_test_state, 0, sizeof(g_test_state));
    g_test_state.next_queue_create_result = (QueueHandle_t)0x11;
    g_test_state.next_xqueue_send_result = pdPASS;
    g_test_state.next_httpd_req_recv_result = 0;
    g_test_state.next_nvs_flash_init_results[0] = ESP_OK;
    g_test_state.next_nvs_flash_init_result_count = 1;
}
