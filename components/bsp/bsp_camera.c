#include "bsp_camera.h"
#include "bsp_config.h"
#include "bsp_pca9539.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BSP_CAMERA";
static bool s_initialized = false;

esp_err_t bsp_camera_init(uint32_t xclk_freq_hz, pixformat_t pixel_format,
                          framesize_t frame_size, uint8_t fb_count) {
  if (s_initialized) {
    ESP_LOGW(TAG, "Camera already initialized");
    return ESP_OK;
  }

  // 检查摄像头电源是否已开启
  bool powered;
  bsp_pca9539_get_pin_level(BSP_PIN_CAM_PWR, &powered);
  if (!powered) {
    ESP_LOGE(TAG,
             "Camera power is off, call bsp_board_camera_power_up() first");
    return ESP_ERR_INVALID_STATE;
  }

  camera_config_t config = {
      .pin_pwdn = BSP_CAM_PWDN,
      .pin_reset = BSP_CAM_RESET,
      .pin_xclk = BSP_CAM_MCLK,
      .pin_sccb_sda = BSP_I2C_MAIN_SDA, // 使用主 I2C 引脚
      .pin_sccb_scl = BSP_I2C_MAIN_SCL,
      .sccb_i2c_port = -1, // -1 表示使用上面指定的引脚自己初始化

      .pin_d7 = BSP_CAM_D7,
      .pin_d6 = BSP_CAM_D6,
      .pin_d5 = BSP_CAM_D5,
      .pin_d4 = BSP_CAM_D4,
      .pin_d3 = BSP_CAM_D3,
      .pin_d2 = BSP_CAM_D2,
      .pin_d1 = BSP_CAM_D1,
      .pin_d0 = BSP_CAM_D0,
      .pin_vsync = BSP_CAM_VSYNC,
      .pin_href = BSP_CAM_HSYNC,
      .pin_pclk = BSP_CAM_PCLK,

      .xclk_freq_hz = xclk_freq_hz,
      .ledc_timer = LEDC_TIMER_0,
      .ledc_channel = LEDC_CHANNEL_0,

      .pixel_format = pixel_format,
      .frame_size = frame_size,
      .jpeg_quality = 12,
      .fb_count = fb_count,
      .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
  };

  esp_err_t ret = esp_camera_init(&config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Camera init failed: %s", esp_err_to_name(ret));
    return ret;
  }

  // 获取传感器并限制帧大小
  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    // 如果请求的帧大小超出传感器支持，降级
    if (frame_size > s->status.framesize) {
      s->set_framesize(s, s->status.framesize);
    }
  }

  s_initialized = true;
  ESP_LOGI(TAG, "Camera initialized");
  return ESP_OK;
}

camera_fb_t *bsp_camera_get_frame(void) {
  if (!s_initialized) {
    ESP_LOGE(TAG, "Camera not initialized");
    return NULL;
  }
  return esp_camera_fb_get();
}

void bsp_camera_return_frame(camera_fb_t *fb) {
  if (fb)
    esp_camera_fb_return(fb);
}

esp_err_t bsp_camera_deinit(void) {
  if (!s_initialized)
    return ESP_OK;
  esp_err_t ret = esp_camera_deinit();
  if (ret == ESP_OK)
    s_initialized = false;
  return ret;
}

const camera_sensor_info_t *bsp_camera_get_sensor_info(void) {
  sensor_t *s = esp_camera_sensor_get();
  if (!s)
    return NULL;
  return esp_camera_sensor_get_info(&s->id);
}