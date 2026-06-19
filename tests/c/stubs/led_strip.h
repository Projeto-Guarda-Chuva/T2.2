#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

typedef void *led_strip_handle_t;

typedef enum {
    LED_PIXEL_FORMAT_GRB = 0,
} led_pixel_format_t;

typedef enum {
    LED_MODEL_WS2812 = 0,
} led_model_t;

typedef enum {
    RMT_CLK_SRC_DEFAULT = 0,
} rmt_clock_source_t;

typedef struct {
    int strip_gpio_num;
    int max_leds;
    led_pixel_format_t led_pixel_format;
    led_model_t led_model;
} led_strip_config_t;

typedef struct {
    rmt_clock_source_t clk_src;
    int resolution_hz;
    struct {
        bool with_dma;
    } flags;
} led_strip_rmt_config_t;

esp_err_t led_strip_new_rmt_device(
    const led_strip_config_t *strip_config,
    const led_strip_rmt_config_t *rmt_config,
    led_strip_handle_t *ret_strip
);
esp_err_t led_strip_set_pixel(
    led_strip_handle_t strip,
    uint32_t index,
    uint32_t red,
    uint32_t green,
    uint32_t blue
);
esp_err_t led_strip_refresh(led_strip_handle_t strip);
esp_err_t led_strip_clear(led_strip_handle_t strip);
