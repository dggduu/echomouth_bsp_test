#include "bsp_board.h"
#include "bsp_audio.h"
#include "bsp_battery.h"
#include "bsp_camera.h"
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "bsp_lcd.h"
#include "bsp_pca9539.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BSP_BOARD";
static i2s_chan_handle_t s_tx_handle = NULL;
static i2s_chan_handle_t s_rx_handle = NULL;

// ===== I2S 初始化（播放+录音） =====
static void init_i2s(void) {
  i2s_chan_config_t chan_cfg =
      I2S_CHANNEL_DEFAULT_CONFIG(BSP_I2S_PORT, I2S_ROLE_MASTER);
  chan_cfg.dma_desc_num = 8;
  chan_cfg.dma_frame_num = 1024;
  ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &s_tx_handle, &s_rx_handle));

  i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(16, I2S_SLOT_MODE_STEREO),
      .gpio_cfg =
          {
              .mclk = BSP_I2S_MCLK,
              .bclk = BSP_I2S_BCK,
              .ws = BSP_I2S_WS,
              .dout = BSP_I2S_DOUT,
              .din = BSP_I2S_DIN,
          },
  };
  ESP_ERROR_CHECK(i2s_channel_init_std_mode(s_tx_handle, &std_cfg));
  ESP_ERROR_CHECK(i2s_channel_init_std_mode(s_rx_handle, &std_cfg));
  ESP_ERROR_CHECK(i2s_channel_enable(s_tx_handle));
  ESP_ERROR_CHECK(i2s_channel_enable(s_rx_handle));
}

// ===== 板级初始化 =====
esp_err_t bsp_board_init(void) {
  ESP_ERROR_CHECK(bsp_i2c_init_main());
  ESP_ERROR_CHECK(bsp_i2c_init_bat()); // 新版电池总线
  ESP_ERROR_CHECK(bsp_pca9539_init(BSP_PCA9539_ADDR));
  ESP_ERROR_CHECK(bsp_board_power_off_all());
  init_i2s();
  bsp_battery_init(); // 尝试初始化
  // bsp_audio_power_on();
  // vTaskDelay(pdMS_TO_TICKS(1000));
  // bsp_i2c_scan(bsp_i2c_get_main_handle());
  ESP_LOGI(TAG, "Board initialized");
  return ESP_OK;
}

// ===== 音频电源 =====
esp_err_t bsp_board_audio_power_up(void) {
  esp_err_t ret = bsp_audio_power_on();
  if (ret == ESP_OK) {
    vTaskDelay(pdMS_TO_TICKS(300));
    // 初始化音频编解码器，传入 I2S 句柄
    ret = bsp_audio_init(s_tx_handle, s_rx_handle);
  }
  return ret;
}

esp_err_t bsp_board_audio_power_down(void) {
  bsp_audio_deinit();
  return bsp_audio_power_off();
}

// ===== 屏幕电源 =====
esp_err_t bsp_board_screen_power_up(void) {
  esp_err_t ret = bsp_screen_power_on();
  if (ret == ESP_OK) {
    vTaskDelay(pdMS_TO_TICKS(100));
    bsp_lcd_reset_high();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  return ret;
}

esp_err_t bsp_board_screen_power_down(void) {
  bsp_lcd_reset_low();
  return bsp_screen_power_off();
}

// ===== 摄像头电源 =====
esp_err_t bsp_board_camera_power_up(void) {
  esp_err_t ret = bsp_cam_power_on();
  if (ret == ESP_OK) {
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  return ret;
}

esp_err_t bsp_board_camera_power_down(void) { return bsp_cam_power_off(); }

// ===== 关闭所有电源 =====
esp_err_t bsp_board_power_off_and_deinit_all(void) {
  bsp_audio_power_off();
  bsp_screen_power_off();
  bsp_cam_power_off();
  bsp_pa_power_off();
  return ESP_OK;
}

/**
 * @brief 拉低所有的引脚
 * @note 默认为高阻，需要给一个显性低电平
 * @return esp_err_t
 */
esp_err_t bsp_board_power_off_all(void) {
  bsp_cam_power_off();
  bsp_audio_power_off();
  bsp_screen_power_off();
  return ESP_OK;
}

// ===== 获取 I2S 句柄（可选） =====
i2s_chan_handle_t bsp_board_get_i2s_tx(void) { return s_tx_handle; }
i2s_chan_handle_t bsp_board_get_i2s_rx(void) { return s_rx_handle; }