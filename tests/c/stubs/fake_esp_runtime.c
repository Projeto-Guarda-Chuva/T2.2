#include "esp_system.h"
#include "esp_timer.h"

#include <stdint.h>

int64_t esp_timer_get_time(void) {
    return 1234567;
}

uint32_t esp_get_free_heap_size(void) {
    return 4096;
}

uint32_t esp_get_minimum_free_heap_size(void) {
    return 2048;
}
