#ifndef BSP_AUDIO_H
#define BSP_AUDIO_H

#include "driver/i2s_std.h"
#include "esp_err.h"


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bsp_audio_init(i2s_chan_handle_t tx_handle,
                         i2s_chan_handle_t rx_handle);
esp_err_t bsp_audio_set_volume(float volume);
esp_err_t bsp_audio_set_mic_gain(float gain);
esp_err_t bsp_audio_play(const uint8_t *data, size_t len);
esp_err_t bsp_audio_record(uint8_t *buf, size_t len);
esp_err_t bsp_audio_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // BSP_AUDIO_H