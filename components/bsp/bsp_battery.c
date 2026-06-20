#include "bsp_battery.h"
#include "bsp_i2c.h"

#ifndef BQ27220_REG_H
#define BQ27220_REG_H
#include "bq27220_reg.h"
#endif

#ifndef BQ27220_DATA_MEMORY_H
#define BQ27220_DATA_MEMORY_H
#include "bq27220_data_memory.h"
#endif

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BSP_BATTERY";
static bool s_initialized = false;
static uint8_t s_bq_addr = 0x55;

// ===== 内部 I2C 读写 =====
static esp_err_t bq_write_bytes(uint8_t cmd, const uint8_t *data, size_t len) {
  return bsp_i2c_write_bat(s_bq_addr, data, len, 1000);
}

static esp_err_t bq_read_bytes(uint8_t cmd, uint8_t *data, size_t len) {
  return bsp_i2c_write_read_bat(s_bq_addr, &cmd, 1, data, len, 1000);
}

static uint16_t bq_read_word(uint8_t cmd) {
  uint8_t buf[2];
  if (bq_read_bytes(cmd, buf, 2) != ESP_OK) {
    ESP_LOGE(TAG, "Read cmd 0x%02X failed", cmd);
    return 0;
  }
  return (uint16_t)(buf[0] | (buf[1] << 8));
}

static esp_err_t bq_write_word(uint8_t cmd, uint16_t value) {
  uint8_t buf[2] = {value & 0xFF, (value >> 8) & 0xFF};
  return bq_write_bytes(cmd, buf, 2);
}

static esp_err_t bq_control(uint16_t control_cmd) {
  return bq_write_word(COMMAND_CONTROL, control_cmd);
}

// ===== 参数访问（内部使用枚举类型） =====
static esp_err_t bq_set_param(bq27220_data_memory_name_t name, uint16_t value) {
  // 查找地址
  const bq27220_data_memory_entry_t *entry = NULL;
  for (size_t i = 0; i < sizeof(dm_table) / sizeof(dm_table[0]); i++) {
    if (dm_table[i].name == name) {
      entry = &dm_table[i];
      break;
    }
  }
  if (!entry)
    return ESP_ERR_NOT_FOUND;

  // 写选择子类+数据
  uint8_t buf[4];
  buf[0] = entry->address & 0xFF;
  buf[1] = (entry->address >> 8) & 0xFF;
  buf[2] = (value >> 8) & 0xFF;
  buf[3] = value & 0xFF;
  esp_err_t ret = bq_write_bytes(COMMAND_SELECT_SUBCLASS, buf, 4);
  if (ret != ESP_OK)
    return ret;
  vTaskDelay(pdMS_TO_TICKS(10));

  // 校验和
  uint8_t checksum = 0;
  for (int i = 0; i < 4; i++)
    checksum += buf[i];
  checksum = 0xFF - checksum;
  uint8_t sum_buf[2] = {checksum, 6};
  ret = bq_write_bytes(COMMAND_MAC_DATA_SUM, sum_buf, 2);
  if (ret != ESP_OK)
    return ret;
  vTaskDelay(pdMS_TO_TICKS(10));
  return ESP_OK;
}

static uint16_t bq_get_param(bq27220_data_memory_name_t name) {
  const bq27220_data_memory_entry_t *entry = NULL;
  for (size_t i = 0; i < sizeof(dm_table) / sizeof(dm_table[0]); i++) {
    if (dm_table[i].name == name) {
      entry = &dm_table[i];
      break;
    }
  }
  if (!entry)
    return 0;

  uint8_t addr_buf[2] = {entry->address & 0xFF, (entry->address >> 8) & 0xFF};
  if (bq_write_bytes(COMMAND_SELECT_SUBCLASS, addr_buf, 2) != ESP_OK)
    return 0;
  vTaskDelay(pdMS_TO_TICKS(10));
  return bq_read_word(COMMAND_MAC_DATA);
}

// ===== 公开函数 =====

esp_err_t bsp_battery_init(void) {
  if (s_initialized)
    return ESP_OK;
  uint16_t vol = bq_read_word(COMMAND_VOLTAGE);
  if (vol == 0) {
    ESP_LOGE(TAG, "BQ27220 not responding");
    return ESP_ERR_INVALID_RESPONSE;
  }
  bq_control(CONTROL_DEVICE_NUMBER);
  vTaskDelay(pdMS_TO_TICKS(15));
  uint16_t dev_id = bq_read_word(COMMAND_MAC_DATA);
  if (dev_id != 0x0220) {
    ESP_LOGE(TAG, "Unexpected dev id: 0x%04X", dev_id);
    return ESP_ERR_INVALID_VERSION;
  }
  s_initialized = true;
  ESP_LOGI(TAG, "BQ27220 OK, vol=%d mV", vol);
  return ESP_OK;
}

uint16_t bsp_battery_get_voltage(void) { return bq_read_word(COMMAND_VOLTAGE); }
int16_t bsp_battery_get_current(void) {
  return (int16_t)bq_read_word(COMMAND_CURRENT);
}
uint16_t bsp_battery_get_remaining_capacity(void) {
  return bq_read_word(COMMAND_REMAINING_CAPACITY);
}
uint16_t bsp_battery_get_full_capacity(void) {
  return bq_read_word(COMMAND_FULL_CHARGE_CAPACITY);
}
uint16_t bsp_battery_get_soc(void) {
  return bq_read_word(COMMAND_STATE_OF_CHARGE);
}
float bsp_battery_get_temperature(void) {
  uint16_t raw = bq_read_word(COMMAND_TEMPERATURE);
  return (raw != 0) ? (raw / 10.0f - 273.0f) : 0.0f;
}

uint16_t bsp_battery_get_fw_version(void) {
  bq_control(CONTROL_FW_VERSION);
  vTaskDelay(pdMS_TO_TICKS(15));
  return bq_read_word(COMMAND_MAC_DATA);
}
uint16_t bsp_battery_get_hw_version(void) {
  bq_control(CONTROL_HW_VERSION);
  vTaskDelay(pdMS_TO_TICKS(15));
  return bq_read_word(COMMAND_MAC_DATA);
}
uint16_t bsp_battery_get_design_capacity(void) {
  return bq_read_word(COMMAND_DESIGN_CAPACITY);
}
uint16_t bsp_battery_get_cycle_count(void) {
  return bq_read_word(COMMAND_CYCLE_COUNT);
}
int16_t bsp_battery_get_average_power(void) {
  return (int16_t)bq_read_word(COMMAND_AVERAGE_POWER);
}
uint16_t bsp_battery_get_time_to_empty(void) {
  return bq_read_word(COMMAND_TIME_TO_EMPTY);
}
uint16_t bsp_battery_get_time_to_full(void) {
  return bq_read_word(COMMAND_TIME_TO_FULL);
}

esp_err_t bsp_battery_seal(void) { return bq_control(CONTROL_SEALED); }
esp_err_t bsp_battery_unseal(void) {
  esp_err_t ret = bq_control(UNSEALKEY1);
  if (ret != ESP_OK)
    return ret;
  vTaskDelay(pdMS_TO_TICKS(10));
  ret = bq_control(UNSEALKEY2);
  if (ret != ESP_OK)
    return ret;
  vTaskDelay(pdMS_TO_TICKS(10));
  return ESP_OK;
}
esp_err_t bsp_battery_enter_config_update(void) {
  esp_err_t ret = bq_control(CONTROL_ENTER_CFG_UPDATE);
  if (ret != ESP_OK)
    return ret;
  uint32_t timeout = 20;
  while (timeout--) {
    uint16_t status = bq_read_word(COMMAND_OPERATION_STATUS);
    if (status & (1 << 6))
      return ESP_OK;
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  return ESP_ERR_TIMEOUT;
}
esp_err_t bsp_battery_exit_config_update(void) {
  esp_err_t ret = bq_control(CONTROL_EXIT_CFG_UPDATE_REINIT);
  if (ret != ESP_OK)
    return ret;
  vTaskDelay(pdMS_TO_TICKS(10));
  uint32_t timeout = 20;
  while (timeout--) {
    uint16_t status = bq_read_word(COMMAND_OPERATION_STATUS);
    if (!(status & (1 << 6)))
      return ESP_OK;
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  return ESP_ERR_TIMEOUT;
}

// 外部接口：参数枚举值作为 uint16_t 传入，内部转换
esp_err_t bsp_battery_set_parameter_u16(uint16_t param_enum_value,
                                        uint16_t value) {
  bq27220_data_memory_name_t param =
      (bq27220_data_memory_name_t)param_enum_value;
  if (!s_initialized)
    return ESP_ERR_INVALID_STATE;
  esp_err_t ret = bsp_battery_unseal();
  if (ret != ESP_OK)
    return ret;
  ret = bsp_battery_enter_config_update();
  if (ret != ESP_OK)
    return ret;
  ret = bq_set_param(param, value);
  if (ret != ESP_OK) {
    bsp_battery_exit_config_update();
    return ret;
  }
  ret = bsp_battery_exit_config_update();
  if (ret != ESP_OK)
    return ret;
  return bsp_battery_seal();
}

uint16_t bsp_battery_get_parameter_u16(uint16_t param_enum_value) {
  bq27220_data_memory_name_t param =
      (bq27220_data_memory_name_t)param_enum_value;
  if (!s_initialized)
    return 0;
  bsp_battery_unseal(); // 忽略错误
  return bq_get_param(param);
}