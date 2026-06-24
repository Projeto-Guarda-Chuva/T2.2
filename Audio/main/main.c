#include "audio_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {
    audio_init();

    vTaskDelay(pdMS_TO_TICKS(2000));

    audio_set_volume(50);

    vTaskDelay(pdMS_TO_TICKS(1000));

    audio_play();

    vTaskDelay(pdMS_TO_TICKS(5000));

    audio_set_volume(80);

    vTaskDelay(pdMS_TO_TICKS(3000));

    audio_stop();
}