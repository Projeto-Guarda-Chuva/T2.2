#include "display_manager.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_jd9365.h"
#include "esp_ldo_regulator.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_err.h"
#include "lvgl.h"

static const char *TAG = "DISPLAY";

#define LCD_BACKLIGHT_GPIO               GPIO_NUM_23
#define LCD_RST_GPIO                     GPIO_NUM_27
#define LCD_H_RES                        800
#define LCD_V_RES                        1280
#define MIPI_DSI_LANE_NUM                2
#define MIPI_DSI_LANE_MBPS               1500
#define MIPI_DSI_PHY_PWR_LDO_CHAN        3
#define MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500
#define LVGL_BUF_LINES                   100

static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_display_t          *lv_disp     = NULL;
static SemaphoreHandle_t      lvgl_mutex  = NULL;
static lv_obj_t              *motor_label = NULL;

/*
_set_state: definida ANTES de qualquer funcao que a chama
Aplica cor de fundo + simbolo central. Mutex deve estar travado
pelo chamador antes de invocar esta funcao.
*/
static void _set_state(uint8_t r, uint8_t g, uint8_t b, const char *symbol)
{
    lv_obj_t *scr = lv_display_get_screen_active(lv_disp);
    if (scr == NULL) return;

    lv_obj_set_style_bg_color(scr, lv_color_make(r, g, b), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_invalidate(scr);

    if (motor_label != NULL) {
        lv_label_set_text(motor_label, symbol);
        lv_obj_center(motor_label);
    }
}

/*
Callback do DMA: chamado quando o flush termina de verdade.
E o UNICO lugar que deve chamar lv_display_flush_ready.
*/
static bool IRAM_ATTR flush_done_cb(esp_lcd_panel_handle_t panel,
                                     esp_lcd_dpi_panel_event_data_t *edata,
                                     void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return false;
}

/*
 Envia o tile para o hardware via DMA.
 NAO chama lv_display_flush_ready aqui — o flush_done_cb faz isso
 quando o DMA realmente terminar.
 */
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_draw_bitmap(panel_handle,
                              area->x1, area->y1,
                              area->x2 + 1, area->y2 + 1,
                              px_map);
}

static void lvgl_tick_task(void *arg)
{
    while (1) {
        lv_tick_inc(10);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void lvgl_handler_task(void *arg)
{
    while (1) {
        if (xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
            uint32_t sleep_ms = lv_task_handler();
            xSemaphoreGive(lvgl_mutex);
            if (sleep_ms < 10)  sleep_ms = 10;
            if (sleep_ms > 100) sleep_ms = 100;
            vTaskDelay(pdMS_TO_TICKS(sleep_ms));
        }
    }
}


bool display_lock(uint32_t timeout_ms)
{
    return xSemaphoreTake(lvgl_mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void display_unlock(void)
{
    xSemaphoreGive(lvgl_mutex);
}


void display_init(void)
{
    ESP_LOGI(TAG, "Iniciando display JD9365 800x1280...");

    lvgl_mutex = xSemaphoreCreateMutex();
    configASSERT(lvgl_mutex);

    /* LDO para PHY MIPI */
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
    esp_ldo_channel_config_t ldo_cfg = {
        .chan_id    = MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_cfg, &ldo_mipi_phy));

    // Backlight desligado at o painel estar pronto
    gpio_config_t bk_cfg = {
        .pin_bit_mask = 1ULL << LCD_BACKLIGHT_GPIO,
        .mode         = GPIO_MODE_OUTPUT,
    };
    gpio_config(&bk_cfg);
    gpio_set_level(LCD_BACKLIGHT_GPIO, 0);

    // Barramento MIPI DSI
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL;
    esp_lcd_dsi_bus_config_t bus_cfg = {
        .bus_id             = 0,
        .num_data_lanes     = MIPI_DSI_LANE_NUM,
        .phy_clk_src        = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
        .lane_bit_rate_mbps = MIPI_DSI_LANE_MBPS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_cfg, &mipi_dsi_bus));

    //IO DBI - canal de comandos
    esp_lcd_panel_io_handle_t mipi_dbi_io = NULL;
    esp_lcd_dbi_io_config_t dbi_cfg = {
        .virtual_channel = 0,
        .lcd_cmd_bits    = 8,
        .lcd_param_bits  = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_cfg, &mipi_dbi_io));

    /*
        Configuracao DPI:
        Clock 30 MHz (era 60): a PSRAM da placa roda a 20 MHz e nao
        alimenta o DPI a 60 MHz -> causava "underrun".
        num_fbs=1 (era 0): ativa framebuffer interno no DPI, reduz
        a pressao no barramento de memoria.
        Timing ajustado para o painel 800x1280 MIPI 2-lane (JC8012P4A1).
     */
    esp_lcd_dpi_panel_config_t dpi_cfg = {
        .virtual_channel    = 0,
        .dpi_clk_src        = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = 20,
        .pixel_format       = LCD_COLOR_PIXEL_FORMAT_RGB565,
        .num_fbs            = 1,
        .video_timing = {
            .h_size            = LCD_H_RES,
            .v_size            = LCD_V_RES,
            .hsync_pulse_width = 10,
            .hsync_back_porch  = 160,
            .hsync_front_porch = 160,
            .vsync_pulse_width = 2,
            .vsync_back_porch  = 23,
            .vsync_front_porch = 12,
        },
        .flags = {
            .use_dma2d = true,
        },
    };

    jd9365_vendor_config_t vendor_cfg = {
        .mipi_config = {
            .dsi_bus    = mipi_dsi_bus,
            .dpi_config = &dpi_cfg,
        },
    };
    esp_lcd_panel_dev_config_t panel_dev_cfg = {
        .reset_gpio_num = LCD_RST_GPIO,
        .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config  = &vendor_cfg,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_jd9365(mipi_dbi_io, &panel_dev_cfg, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    gpio_set_level(LCD_BACKLIGHT_GPIO, 1);
    ESP_LOGI(TAG, "Backlight ligado!");


    lv_init();
    lv_disp = lv_display_create(LCD_H_RES, LCD_V_RES);
    lv_display_set_flush_cb(lv_disp, lvgl_flush_cb);
    lv_display_set_color_format(lv_disp, LV_COLOR_FORMAT_RGB565);

    /*
     Double-buffer na DIRAM com MALLOC_CAP_DMA.
     800 x 100 x 2 = 160.000 bytes (~156 KB) cada buffer.
     Enquanto o DMA envia buf1, o LVGL ja renderiza em buf2 — evita
     o erro "previous draw operation is not finished".
     */
    size_t buf_size = LCD_H_RES * LVGL_BUF_LINES * sizeof(uint16_t);
    void *buf1 = heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    void *buf2 = heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!buf1 || !buf2) {
        ESP_LOGE(TAG, "Falha ao alocar buffers LVGL (%zu bytes cada na DIRAM)!", buf_size);
        return;
    }

    lv_display_set_buffers(lv_disp, buf1, buf2, buf_size,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    /*
     IMPORTANTE: registra o callback DEPOIS de lv_display_create.
     lv_disp precisa ser valido (nao NULL) para ser passado como
     user_ctx, senao o callback do DMA causa crash (Store access
     fault em lv_display_flush_ready com disp=0x0).
     */
    esp_lcd_dpi_panel_event_callbacks_t dpi_cbs = {
        .on_color_trans_done = flush_done_cb,
    };
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_register_event_callbacks(panel_handle, &dpi_cbs, lv_disp));

    ESP_LOGI(TAG, "Buffers: 2 x %zu bytes na DIRAM (double-buffer DMA)", buf_size);

    //Tela inicial: fundo preto + label de seta
    lv_obj_t *scr = lv_display_get_screen_active(lv_disp);
    lv_obj_set_style_bg_color(scr, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    motor_label = lv_label_create(scr);
    lv_obj_set_style_text_font(motor_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(motor_label, lv_color_white(), 0);
    lv_label_set_text(motor_label, "");
    lv_obj_center(motor_label);

    /*
     lvgl_tick_task no core 1 (junto com Wi-Fi/SDIO).
     lvgl_handler_task no core 0 — separado do core que processa
     o protocolo HTTP, evitando disputa pelo mutex (resolvia o
     erro "Timeout ao travar LVGL!").
     */
    xTaskCreatePinnedToCore(lvgl_tick_task,    "lvgl_tick",    1024, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(lvgl_handler_task, "lvgl_handler", 4096, NULL, 4, NULL, 0);

    ESP_LOGI(TAG, "Display pronto!");
}


void display_set_bg_color(uint8_t r, uint8_t g, uint8_t b)
{
    if (lv_disp == NULL) {
        ESP_LOGW(TAG, "display_set_bg_color chamado antes de display_init!");
        return;
    }
    // timeout 5000ms (era 1000ms) — margem maior sob carga de Wi-Fi
    if (!display_lock(5000)) {
        ESP_LOGE(TAG, "Timeout ao travar LVGL!");
        return;
    }
    _set_state(r, g, b, "");
    display_unlock();
    ESP_LOGI(TAG, "Cor definida: R=%d G=%d B=%d", r, g, b);
}
