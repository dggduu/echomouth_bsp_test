#include "bsp_pca9539.h"
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "esp_log.h"


static const char *TAG = "BSP_PCA9539";
static uint8_t s_addr = 0x74;

#define REG_INPUT0 0x00
#define REG_OUTPUT0 0x02
#define REG_CONFIG0 0x06

static esp_err_t write_reg(uint8_t reg, uint8_t data) {
  uint8_t buf[2] = {reg, data};
  return bsp_i2c_write_main(s_addr, buf, 2, 1000);
}

static esp_err_t read_reg(uint8_t reg, uint8_t *data) {
  return bsp_i2c_write_read_main(s_addr, &reg, 1, data, 1, 1000);
}

esp_err_t bsp_pca9539_init(uint8_t addr_7bit) {
  s_addr = addr_7bit;
  esp_err_t ret = write_reg(REG_CONFIG0, 0x00); // all outputs
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Config failed");
    return ret;
  }
  ret = write_reg(REG_OUTPUT0, 0x00); // default low
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Set outputs low failed");
    return ret;
  }
  ESP_LOGI(TAG, "PCA9539 initialized at 0x%02X", addr_7bit);
  return ESP_OK;
}

esp_err_t bsp_pca9539_set_pin_direction(uint8_t pin, bool is_input) {
  if (pin > 7)
    return ESP_ERR_INVALID_ARG;
  uint8_t config;
  esp_err_t ret = read_reg(REG_CONFIG0, &config);
  if (ret != ESP_OK)
    return ret;
  if (is_input)
    config |= (1 << pin);
  else
    config &= ~(1 << pin);
  return write_reg(REG_CONFIG0, config);
}

esp_err_t bsp_pca9539_set_pin_level(uint8_t pin, bool level) {
  if (pin > 7)
    return ESP_ERR_INVALID_ARG;
  uint8_t output;
  esp_err_t ret = read_reg(REG_OUTPUT0, &output);
  if (ret != ESP_OK)
    return ret;
  if (level)
    output |= (1 << pin);
  else
    output &= ~(1 << pin);
  return write_reg(REG_OUTPUT0, output);
}

esp_err_t bsp_pca9539_get_pin_level(uint8_t pin, bool *level) {
  if (pin > 7 || level == NULL)
    return ESP_ERR_INVALID_ARG;
  uint8_t input;
  esp_err_t ret = read_reg(REG_INPUT0, &input);
  if (ret != ESP_OK)
    return ret;
  *level = (input >> pin) & 0x01;
  return ESP_OK;
}

esp_err_t bsp_pca9539_write_output_port0(uint8_t value) {
  return write_reg(REG_OUTPUT0, value);
}