#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

typedef struct led_strip_s *led_strip_handle_t;

typedef enum {
    LED_PIXEL_FORMAT_GRB,
    LED_PIXEL_FORMAT_GRBW,
} led_pixel_format_t;

typedef enum {
    LED_MODEL_WS2812,
    LED_MODEL_SK6812,
} led_model_t;

typedef enum {
    RMT_CLK_SRC_DEFAULT,
} rmt_clock_source_t;

typedef struct {
    int                strip_gpio_num;
    uint32_t           max_leds;
    led_pixel_format_t led_pixel_format;
    led_model_t        led_model;
} led_strip_config_t;

typedef struct {
    rmt_clock_source_t clk_src;
    uint32_t           resolution_hz;
    struct {
        uint32_t with_dma;
    } flags;
} led_strip_rmt_config_t;

esp_err_t led_strip_new_rmt_device(const led_strip_config_t *config,
                                   const led_strip_rmt_config_t *rmt_config,
                                   led_strip_handle_t *ret_strip);
esp_err_t led_strip_set_pixel(led_strip_handle_t strip, uint32_t index,
                              uint32_t red, uint32_t green, uint32_t blue);
esp_err_t led_strip_refresh(led_strip_handle_t strip);
esp_err_t led_strip_clear(led_strip_handle_t strip);

typedef struct {
    int      new_device_count;
    int      set_pixel_count;
    int      refresh_count;
    int      clear_count;
    uint32_t last_index;
    uint8_t  last_r;
    uint8_t  last_g;
    uint8_t  last_b;
} led_spy_t;

extern led_spy_t g_led_spy;
void led_spy_reset(void);
