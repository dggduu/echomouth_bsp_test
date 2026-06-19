#include "bsp_lcd.h"
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_lcd_st77916.h"
#include "esp_lcd_touch_cst816s.h"

static const char *TAG = "BSP_LCD";
static esp_lcd_panel_handle_t s_panel = NULL;
static esp_lcd_touch_handle_t s_touch = NULL;

static void backlight_init(void) {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer);
    ledc_channel_config_t ch = {
        .gpio_num = BSP_LCD_BLK,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&ch);
}

esp_err_t bsp_lcd_init(esp_lcd_panel_handle_t *lcd_panel, esp_lcd_touch_handle_t *touch_handle) {
    backlight_init();
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    // SPI 总线（QSPI）
    spi_bus_config_t buscfg = {
        .mosi_io_num = -1,
        .miso_io_num = -1,
        .sclk_io_num = BSP_LCD_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = BSP_LCD_H_RES * 80 * BSP_LCD_BPP / 8,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(BSP_LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // LCD IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    const esp_lcd_panel_io_spi_config_t io_config = ST77916_PANEL_IO_QSPI_CONFIG(BSP_LCD_CS, NULL, NULL);
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_HOST, &io_config, &io_handle));

    // ST77916 驱动
    st77916_vendor_config_t vendor_config = { .flags = { .use_qspi_interface = 1 } };
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = BSP_LCD_BPP,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st77916(io_handle, &panel_config, &s_panel));
    esp_lcd_panel_reset(s_panel);
    esp_lcd_panel_init(s_panel);
    esp_lcd_panel_disp_on_off(s_panel, true);

    // 触摸（使用新版 I2C 主总线）
    i2c_master_bus_handle_t i2c_bus = bsp_i2c_get_main_handle();
    if (i2c_bus) {
        esp_lcd_panel_io_handle_t tp_io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_config, &tp_io_handle));

        esp_lcd_touch_config_t tp_cfg = {
            .x_max = BSP_LCD_H_RES,
            .y_max = BSP_LCD_V_RES,
            .rst_gpio_num = GPIO_NUM_NC,
            .int_gpio_num = BSP_TOUCH_INT,
            .levels = { .reset = 0, .interrupt = 0 },
            .flags = { .swap_xy = 0, .mirror_x = 1, .mirror_y = 0 },
        };
        ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &s_touch));
        ESP_LOGI(TAG, "Touch initialized");
    } else {
        ESP_LOGW(TAG, "Touch skipped: I2C bus not ready");
    }

    *lcd_panel = s_panel;
    *touch_handle = s_touch;
    return ESP_OK;
}


esp_err_t bsp_lcd_set_backlight(uint8_t brightness) {
  if (brightness > 100)
    brightness = 100;
  uint32_t duty = (brightness * 255) / 100;
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
  return ESP_OK;
}

esp_err_t bsp_lcd_touch_get_point(esp_lcd_touch_handle_t touch, uint16_t *x,
                                  uint16_t *y) {
  if (!touch)
    return ESP_ERR_INVALID_STATE;
  uint8_t cnt;
  esp_lcd_touch_point_data_t point[1];
  esp_lcd_touch_get_data(touch, point, &cnt, 1);
  if (cnt > 0) {
    *x = point[0].x;
    *y = point[0].y;
    return ESP_OK;
  }
  return ESP_ERR_NOT_FOUND;
}