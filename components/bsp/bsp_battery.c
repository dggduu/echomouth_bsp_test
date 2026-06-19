#include "bsp_battery.h"
#include "bsp_config.h"
#include "bsp_i2c.h"
#include "esp_log.h"


static const char *TAG = "BSP_BATTERY";
static uint8_t s_addr = BSP_BQ27220_ADDR;

#define CMD_VOLTAGE 0x02
#define CMD_TEMPERATURE 0x06
#define CMD_CURRENT 0x0E
#define CMD_REM_CAP 0x10
#define CMD_FULL_CAP 0x12
#define CMD_SOC 0x1C

static uint16_t read_word(uint8_t cmd) {
  uint8_t data[2];
  esp_err_t ret = bsp_i2c_write_read_bat(s_addr, &cmd, 1, data, 2, 1000);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Read word failed for cmd 0x%02X", cmd);
    return 0;
  }
  return (uint16_t)(data[0] | (data[1] << 8));
}

static int16_t read_signed_word(uint8_t cmd) { return (int16_t)read_word(cmd); }

esp_err_t bsp_battery_init(void) {
  uint16_t vol = read_word(CMD_VOLTAGE);
  if (vol == 0) {
    ESP_LOGE(TAG, "BQ27220 communication failed");
    return ESP_ERR_INVALID_RESPONSE;
  }
  ESP_LOGI(TAG, "BQ27220 initialized, voltage=%d mV", vol);
  return ESP_OK;
}

uint16_t bsp_battery_get_voltage(void) { return read_word(CMD_VOLTAGE); }
int16_t bsp_battery_get_current(void) { return read_signed_word(CMD_CURRENT); }
uint16_t bsp_battery_get_remaining_capacity(void) {
  return read_word(CMD_REM_CAP);
}
uint16_t bsp_battery_get_full_capacity(void) { return read_word(CMD_FULL_CAP); }
uint16_t bsp_battery_get_soc(void) { return read_word(CMD_SOC); }
float bsp_battery_get_temperature(void) {
  uint16_t raw = read_word(CMD_TEMPERATURE);
  return (float)(raw / 10.0f - 273.0f);
}