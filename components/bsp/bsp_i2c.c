#include "bsp_i2c.h"
#include "bsp_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BSP_I2C";
static i2c_bus_handle_t s_main_bus = NULL;
static i2c_bus_handle_t s_bat_bus = NULL;

esp_err_t bsp_i2c_init_main(void) {
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = BSP_I2C_MAIN_SDA,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_io_num = BSP_I2C_MAIN_SCL,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = BSP_I2C_MAIN_FREQ,
  };
  s_main_bus = i2c_bus_create(BSP_I2C_MAIN_PORT, &conf);
  if (s_main_bus == NULL) {
    ESP_LOGE(TAG, "Main I2C init failed");
    return ESP_ERR_NO_MEM;
  }
  ESP_LOGI(TAG, "Main I2C initialized");
  return ESP_OK;
}

esp_err_t bsp_i2c_init_bat(void) {
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = BSP_I2C_BAT_SDA,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_io_num = BSP_I2C_BAT_SCL,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = BSP_I2C_BAT_FREQ,
  };
  s_bat_bus = i2c_bus_create(BSP_I2C_BAT_PORT, &conf);
  if (s_bat_bus == NULL) {
    ESP_LOGE(TAG, "Battery I2C init failed");
    return ESP_ERR_NO_MEM;
  }
  ESP_LOGI(TAG, "Battery I2C initialized");
  return ESP_OK;
}

i2c_bus_handle_t bsp_i2c_get_main_bus(void) { return s_main_bus; }
i2c_bus_handle_t bsp_i2c_get_bat_bus(void) { return s_bat_bus; }
int bsp_i2c_get_main_port(void) { return BSP_I2C_MAIN_PORT; }
int bsp_i2c_get_bat_port(void) { return BSP_I2C_BAT_PORT; }

esp_err_t bsp_i2c_write_main(uint8_t addr_7bit, const uint8_t *data, size_t len,
                             int timeout_ms) {
  if (s_main_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  return i2c_bus_write_bytes(s_main_bus, addr_7bit << 1, data, len);
}

esp_err_t bsp_i2c_read_main(uint8_t addr_7bit, uint8_t *data, size_t len,
                            int timeout_ms) {
  if (s_main_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  return i2c_bus_read_bytes(s_main_bus, addr_7bit << 1, NULL, 0, data, len);
}

esp_err_t bsp_i2c_write_read_main(uint8_t addr_7bit, const uint8_t *write_data,
                                  size_t write_len, uint8_t *read_data,
                                  size_t read_len, int timeout_ms) {
  if (s_main_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  esp_err_t ret =
      i2c_bus_write_bytes(s_main_bus, addr_7bit << 1, write_data, write_len);
  if (ret != ESP_OK)
    return ret;
  return i2c_bus_read_bytes(s_main_bus, addr_7bit << 1, NULL, 0, read_data,
                            read_len);
}

esp_err_t bsp_i2c_write_bat(uint8_t addr_7bit, const uint8_t *data, size_t len,
                            int timeout_ms) {
  if (s_bat_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  return i2c_bus_write_bytes(s_bat_bus, addr_7bit << 1, data, len);
}

esp_err_t bsp_i2c_read_bat(uint8_t addr_7bit, uint8_t *data, size_t len,
                           int timeout_ms) {
  if (s_bat_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  return i2c_bus_read_bytes(s_bat_bus, addr_7bit << 1, NULL, 0, data, len);
}

esp_err_t bsp_i2c_write_read_bat(uint8_t addr_7bit, const uint8_t *write_data,
                                 size_t write_len, uint8_t *read_data,
                                 size_t read_len, int timeout_ms) {
  if (s_bat_bus == NULL)
    return ESP_ERR_INVALID_STATE;
  esp_err_t ret =
      i2c_bus_write_bytes(s_bat_bus, addr_7bit << 1, write_data, write_len);
  if (ret != ESP_OK)
    return ret;
  return i2c_bus_read_bytes(s_bat_bus, addr_7bit << 1, NULL, 0, read_data,
                            read_len);
}

void bsp_i2c_scan(i2c_bus_handle_t bus) {
  if (bus == NULL)
    return;
  ESP_LOGI(TAG, "Scanning I2C bus...");
  int found = 0;
  for (uint8_t addr_7bit = 1; addr_7bit < 127; addr_7bit++) {
    uint8_t addr_8bit = addr_7bit << 1;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr_8bit | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t ret =
        i2c_master_cmd_begin(i2c_bus_get_port(bus), cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "  Found device at 0x%02X", addr_7bit);
      found++;
    }
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  ESP_LOGI(TAG, "Scan complete, found %d devices", found);
}