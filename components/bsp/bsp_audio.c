#include "bsp_audio.h"
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "es8311_codec.h"
#include "esp_codec_dev.h"
#include "esp_log.h"


static const char *TAG = "BSP_AUDIO";
static esp_codec_dev_handle_t s_codec_dev = NULL;

// 自定义控制接口（使用旧版I2C）
static esp_err_t ctrl_open(const audio_codec_ctrl_if_t *ctrl, void *ctx) {
  return ESP_OK;
}
static esp_err_t ctrl_close(const audio_codec_ctrl_if_t *ctrl, void *ctx) {
  return ESP_OK;
}
static esp_err_t ctrl_read_reg(const audio_codec_ctrl_if_t *ctrl, void *ctx,
                               uint8_t reg, uint8_t *value) {
  uint8_t addr = (uintptr_t)ctx & 0xFF;
  return bsp_i2c_write_read_main(addr, &reg, 1, value, 1, 1000);
}
static esp_err_t ctrl_write_reg(const audio_codec_ctrl_if_t *ctrl, void *ctx,
                                uint8_t reg, uint8_t value) {
  uint8_t addr = (uintptr_t)ctx & 0xFF;
  uint8_t buf[2] = {reg, value};
  return bsp_i2c_write_main(addr, buf, 2, 1000);
}
static const audio_codec_ctrl_if_t s_custom_ctrl_if = {
    .open = ctrl_open,
    .close = ctrl_close,
    .read_reg = ctrl_read_reg,
    .write_reg = ctrl_write_reg,
};

esp_err_t bsp_audio_init(i2s_chan_handle_t tx_handle,
                         i2s_chan_handle_t rx_handle) {
  // 数据接口
  audio_codec_i2s_cfg_t i2s_cfg = {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
      .rx_handle = rx_handle,
      .tx_handle = tx_handle,
#endif
  };
  const audio_codec_data_if_t *data_if = audio_codec_new_i2s_data(&i2s_cfg);
  if (!data_if) {
    ESP_LOGE(TAG, "I2S data interface failed");
    return ESP_ERR_NO_MEM;
  }

  // 控制接口上下文（ES8311地址）
  void *ctx = (void *)(uintptr_t)BSP_ES8311_ADDR;

  // GPIO接口（不使用PA控制）
  const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();

  // ES8311编解码器
  es8311_codec_cfg_t es8311_cfg = {
      .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
      .ctrl_if = &s_custom_ctrl_if,
      .gpio_if = gpio_if,
      .pa_pin = -1,
      .use_mclk = true,
      .ctx = ctx,
  };
  const audio_codec_if_t *codec_if = es8311_codec_new(&es8311_cfg);
  if (!codec_if) {
    ESP_LOGE(TAG, "ES8311 codec create failed");
    return ESP_ERR_NO_MEM;
  }

  esp_codec_dev_cfg_t dev_cfg = {
      .codec_if = codec_if,
      .data_if = data_if,
      .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT,
  };
  s_codec_dev = esp_codec_dev_new(&dev_cfg);
  if (!s_codec_dev) {
    ESP_LOGE(TAG, "Codec device create failed");
    return ESP_ERR_NO_MEM;
  }

  esp_codec_dev_sample_info_t fs = {
      .sample_rate = 16000,
      .channel = 2,
      .bits_per_sample = 16,
  };
  esp_codec_dev_open(s_codec_dev, &fs);
  esp_codec_dev_set_out_vol(s_codec_dev, 60.0);
  esp_codec_dev_set_in_gain(s_codec_dev, 30.0);

  ESP_LOGI(TAG, "Audio initialized");
  return ESP_OK;
}

esp_err_t bsp_audio_set_volume(float volume) {
  if (!s_codec_dev)
    return ESP_ERR_INVALID_STATE;
  return esp_codec_dev_set_out_vol(s_codec_dev, volume);
}

esp_err_t bsp_audio_set_mic_gain(float gain) {
  if (!s_codec_dev)
    return ESP_ERR_INVALID_STATE;
  return esp_codec_dev_set_in_gain(s_codec_dev, gain);
}

esp_err_t bsp_audio_play(const uint8_t *data, size_t len) {
  if (!s_codec_dev)
    return ESP_ERR_INVALID_STATE;
  return esp_codec_dev_write(s_codec_dev, data, len);
}

esp_err_t bsp_audio_record(uint8_t *buf, size_t len) {
  if (!s_codec_dev)
    return ESP_ERR_INVALID_STATE;
  return esp_codec_dev_read(s_codec_dev, buf, len);
}

esp_err_t bsp_audio_deinit(void) {
  if (s_codec_dev) {
    esp_codec_dev_close(s_codec_dev);
    esp_codec_dev_delete(s_codec_dev);
    s_codec_dev = NULL;
  }
  return ESP_OK;
}