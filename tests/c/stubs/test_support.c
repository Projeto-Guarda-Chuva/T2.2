#include "test_support.h"

#include <string.h>

#include "http_comm.h"

test_state_t g_test_state;

void test_support_reset(void) {
    memset(&g_test_state, 0, sizeof(g_test_state));
    g_test_state.http_client_status_code = 200;
    g_test_state.next_queue_create_result = (QueueHandle_t)0x11;
    g_test_state.next_xqueue_send_result = pdPASS;
}
