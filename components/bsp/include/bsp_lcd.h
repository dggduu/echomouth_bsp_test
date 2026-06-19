#ifndef BSP_LCD_H
#define BSP_LCD_H

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bsp_lcd_init(esp_lcd_panel_handle_t *lcd_panel,
                       esp_lcd_touch_handle_t *touch_handle);
esp_err_t bsp_lcd_set_backlight(uint8_t brightness);
esp_err_t bsp_lcd_touch_get_point(esp_lcd_touch_handle_t touch, uint16_t *x,
                                  uint16_t *y);

#ifdef __cplusplus
}
#endif

#endif // BSP_LCD_H