#ifndef BSP_PCA9539_H
#define BSP_PCA9539_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_PIN_PA_CTRL 0
#define BSP_PIN_LCD_RST 1
#define BSP_PIN_SCREEN_PWR 2
#define BSP_PIN_AUDIO_PWR 4
#define BSP_PIN_CAM_PWR 5

#define bsp_pa_power_enable() bsp_pca9539_set_pin_level(BSP_PIN_PA_CTRL, true)
#define bsp_pa_power_disable() bsp_pca9539_set_pin_level(BSP_PIN_PA_CTRL, false)
#define bsp_lcd_reset_high() bsp_pca9539_set_pin_level(BSP_PIN_LCD_RST, true)
#define bsp_lcd_reset_low() bsp_pca9539_set_pin_level(BSP_PIN_LCD_RST, false)
#define bsp_screen_power_on()                                                  \
  bsp_pca9539_set_pin_level(BSP_PIN_SCREEN_PWR, true)
#define bsp_screen_power_off()                                                 \
  bsp_pca9539_set_pin_level(BSP_PIN_SCREEN_PWR, false)
#define bsp_audio_power_on() bsp_pca9539_set_pin_level(BSP_PIN_AUDIO_PWR, true)
#define bsp_audio_power_off()                                                  \
  bsp_pca9539_set_pin_level(BSP_PIN_AUDIO_PWR, false)
#define bsp_cam_power_on() bsp_pca9539_set_pin_level(BSP_PIN_CAM_PWR, true)
#define bsp_cam_power_off() bsp_pca9539_set_pin_level(BSP_PIN_CAM_PWR, false)

esp_err_t bsp_pca9539_init(uint8_t addr_7bit);
esp_err_t bsp_pca9539_set_pin_direction(uint8_t pin, bool is_input);
esp_err_t bsp_pca9539_set_pin_level(uint8_t pin, bool level);
esp_err_t bsp_pca9539_get_pin_level(uint8_t pin, bool *level);
esp_err_t bsp_pca9539_write_output_port0(uint8_t value);

#ifdef __cplusplus
}
#endif

#endif // BSP_PCA9539_H