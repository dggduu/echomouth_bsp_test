#include "bsp_audio.h"
#include "bsp_board.h"
#include "bsp_camera.h"
#include "bsp_lcd.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char *TAG = "MAIN";

void app_main(void) {
  ESP_LOGI(TAG, "BSP Demo with all peripherals");

  // 1. 板级初始化
  ESP_ERROR_CHECK(bsp_board_init());

  // 2. 音频
  ESP_ERROR_CHECK(bsp_board_audio_power_up());
  // ESP_ERROR_CHECK(bsp_board_screen_power_up());
  // ESP_ERROR_CHECK(bsp_board_camera_power_up());
  ESP_LOGI(TAG, "Audio ready");

  // // 3. 屏幕
  // ESP_ERROR_CHECK(bsp_board_screen_power_up());
  // esp_lcd_panel_handle_t lcd;
  // esp_lcd_touch_handle_t touch;
  // ESP_ERROR_CHECK(bsp_lcd_init(&lcd, &touch));
  // ESP_LOGI(TAG, "LCD ready");

  // // 4. 摄像头
  // ESP_ERROR_CHECK(bsp_board_camera_power_up());
  // ESP_ERROR_CHECK(bsp_camera_init(20000000, PIXFORMAT_JPEG, FRAMESIZE_SVGA,
  // 2)); ESP_LOGI(TAG, "Camera ready");

  // // 5. 拍照测试
  // camera_fb_t *fb = bsp_camera_get_frame();
  // if (fb) {
  //   ESP_LOGI(TAG, "Captured %zu bytes", fb->len);
  //   bsp_camera_return_frame(fb);
  // }

  // 6. 录音测试
  int16_t pcm[512];
  while (1) {
    esp_err_t ret = bsp_audio_record((uint8_t *)pcm, sizeof(pcm));
    if (ret == ESP_OK) {
      for (int i = 0; i < 32; i++) {
        printf("%6d ", pcm[i]);
      }
      printf("\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}