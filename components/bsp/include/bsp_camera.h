#ifndef BSP_CAMERA_H
#define BSP_CAMERA_H

#include "esp_camera.h"
#include "esp_err.h"


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bsp_camera_init(uint32_t xclk_freq_hz, pixformat_t pixel_format,
                          framesize_t frame_size, uint8_t fb_count);
camera_fb_t *bsp_camera_get_frame(void);
void bsp_camera_return_frame(camera_fb_t *fb);
esp_err_t bsp_camera_deinit(void);
const camera_sensor_info_t *bsp_camera_get_sensor_info(void);

#ifdef __cplusplus
}
#endif

#endif // BSP_CAMERA_H