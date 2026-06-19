#include "bsp_i2c.h"
#include "bsp_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BSP_I2C";
static i2c_master_bus_handle_t s_main_bus = NULL;
static i2c_master_bus_handle_t s_bat_bus = NULL;

// 设备句柄缓存（优化性能，避免频繁 add/remove）
typedef struct {
  i2c_master_dev_handle_t dev;
  uint8_t addr;
} dev_cache_t;
static dev_cache_t s_main_dev = {.dev = NULL, .addr = 0};
static dev_cache_t s_bat_dev = {.dev = NULL, .addr = 0};

static esp_err_t get_device(i2c_master_bus_handle_t bus, dev_cache_t *cache,
                            uint8_t addr_7bit, uint32_t freq) {
  if (cache->dev != NULL && cache->addr == addr_7bit) {
    return ESP_OK;
  }
  // 移除旧设备
  if (cache->dev != NULL) {
    i2c_master_bus_rm_device(cache->dev);
    cache->dev = NULL;
  }
  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = addr_7bit,
      .scl_speed_hz = freq,
  };
  esp_err_t ret = i2c_master_bus_add_device(bus, &dev_cfg, &cache->dev);
  if (ret == ESP_OK) {
    cache->addr = addr_7bit;
  }
  return ret;
}

esp_err_t bsp_i2c_init_main(void) {
  i2c_master_bus_config_t bus_cfg = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = BSP_I2C_MAIN_PORT,
      .scl_io_num = BSP_I2C_MAIN_SCL,
      .sda_io_num = BSP_I2C_MAIN_SDA,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };
  esp_err_t ret = i2c_new_master_bus(&bus_cfg, &s_main_bus);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Main I2C init failed: %s", esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG, "Main I2C initialized");
  }
  return ret;
}

esp_err_t bsp_i2c_init_bat(void) {
  i2c_master_bus_config_t bus_cfg = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = BSP_I2C_BAT_PORT,
      .scl_io_num = BSP_I2C_BAT_SCL,
      .sda_io_num = BSP_I2C_BAT_SDA,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };
  esp_err_t ret = i2c_new_master_bus(&bus_cfg, &s_bat_bus);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Battery I2C init failed: %s", esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG, "Battery I2C initialized");
  }
  return ret;
}

i2c_master_bus_handle_t bsp_i2c_get_main_handle(void) { return s_main_bus; }
i2c_master_bus_handle_t bsp_i2c_get_bat_handle(void) { return s_bat_bus; }

// === 主总线读写 ===
esp_err_t bsp_i2c_write_main(uint8_t addr_7bit, const uint8_t *data, size_t len,
                             int timeout_ms) {
  if (s_main_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  esp_err_t ret =
      get_device(s_main_bus, &s_main_dev, addr_7bit, BSP_I2C_MAIN_FREQ);
  if (ret != ESP_OK)
    return ret;
  return i2c_master_transmit(s_main_dev.dev, data, len,
                             pdMS_TO_TICKS(timeout_ms));
}

esp_err_t bsp_i2c_read_main(uint8_t addr_7bit, uint8_t *data, size_t len,
                            int timeout_ms) {
  if (s_main_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  esp_err_t ret =
      get_device(s_main_bus, &s_main_dev, addr_7bit, BSP_I2C_MAIN_FREQ);
  if (ret != ESP_OK)
    return ret;
  return i2c_master_receive(s_main_dev.dev, data, len,
                            pdMS_TO_TICKS(timeout_ms));
}

esp_err_t bsp_i2c_write_read_main(uint8_t addr_7bit, const uint8_t *write_data,
                                  size_t write_len, uint8_t *read_data,
                                  size_t read_len, int timeout_ms) {
  if (s_main_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  esp_err_t ret =
      get_device(s_main_bus, &s_main_dev, addr_7bit, BSP_I2C_MAIN_FREQ);
  if (ret != ESP_OK)
    return ret;
  return i2c_master_transmit_receive(s_main_dev.dev, write_data, write_len,
                                     read_data, read_len,
                                     pdMS_TO_TICKS(timeout_ms));
}

// === 电池总线读写 ===
esp_err_t bsp_i2c_write_bat(uint8_t addr_7bit, const uint8_t *data, size_t len,
                            int timeout_ms) {
  if (s_bat_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  esp_err_t ret =
      get_device(s_bat_bus, &s_bat_dev, addr_7bit, BSP_I2C_BAT_FREQ);
  if (ret != ESP_OK)
    return ret;
  return i2c_master_transmit(s_bat_dev.dev, data, len,
                             pdMS_TO_TICKS(timeout_ms));
}

esp_err_t bsp_i2c_read_bat(uint8_t addr_7bit, uint8_t *data, size_t len,
                           int timeout_ms) {
  if (s_bat_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  esp_err_t ret =
      get_device(s_bat_bus, &s_bat_dev, addr_7bit, BSP_I2C_BAT_FREQ);
  if (ret != ESP_OK)
    return ret;
  return i2c_master_receive(s_bat_dev.dev, data, len,
                            pdMS_TO_TICKS(timeout_ms));
}

esp_err_t bsp_i2c_write_read_bat(uint8_t addr_7bit, const uint8_t *write_data,
                                 size_t write_len, uint8_t *read_data,
                                 size_t read_len, int timeout_ms) {
  if (s_bat_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  esp_err_t ret =
      get_device(s_bat_bus, &s_bat_dev, addr_7bit, BSP_I2C_BAT_FREQ);
  if (ret != ESP_OK)
    return ret;
  return i2c_master_transmit_receive(s_bat_dev.dev, write_data, write_len,
                                     read_data, read_len,
                                     pdMS_TO_TICKS(timeout_ms));
}

void bsp_i2c_scan(i2c_master_bus_handle_t bus) {
  if (bus == NULL)
    return;
  ESP_LOGI(TAG, "Scanning I2C bus...");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t probe;
    esp_err_t ret = i2c_master_bus_add_device(bus, &dev_cfg, &probe);
    if (ret == ESP_OK) {
      ret = i2c_master_transmit(probe, NULL, 0, pdMS_TO_TICKS(100));
      if (ret == ESP_OK) {
        ESP_LOGI(TAG, "  Found device at 0x%02X", addr);
        found++;
      }
      i2c_master_bus_rm_device(probe);
    }
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  ESP_LOGI(TAG, "Scan complete, found %d devices", found);
}