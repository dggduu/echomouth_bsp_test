#ifndef BSP_BOARD_H
#define BSP_BOARD_H

#include "driver/i2s_std.h"
#include "esp_err.h"


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bsp_board_init(void);
esp_err_t bsp_board_audio_power_up(void);
esp_err_t bsp_board_screen_power_up(void);
esp_err_t bsp_board_camera_power_up(void);
esp_err_t bsp_board_power_off_all(void);
i2s_chan_handle_t bsp_board_get_i2s_tx(void);
i2s_chan_handle_t bsp_board_get_i2s_rx(void);

#ifdef __cplusplus
}
#endif

#endif // BSP_BOARD_H