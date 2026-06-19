#include "lighting_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void app_main(void) {
    lighting_init();

    lighting_set_color(255, 0, 0);

    vTaskDelay(pdMS_TO_TICKS(3000));

    lighting_set_color(0, 255, 0);

    vTaskDelay(pdMS_TO_TICKS(3000));

    lighting_set_color(0, 0, 255);
}
