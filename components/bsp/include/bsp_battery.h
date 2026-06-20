#ifndef BSP_BATTERY_H
#define BSP_BATTERY_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bsp_battery_init(void);
uint16_t bsp_battery_get_voltage(void);
int16_t bsp_battery_get_current(void);
uint16_t bsp_battery_get_remaining_capacity(void);
uint16_t bsp_battery_get_full_capacity(void);
uint16_t bsp_battery_get_soc(void);
float bsp_battery_get_temperature(void);

// 扩展
uint16_t bsp_battery_get_fw_version(void);
uint16_t bsp_battery_get_hw_version(void);
uint16_t bsp_battery_get_design_capacity(void);
uint16_t bsp_battery_get_cycle_count(void);
int16_t bsp_battery_get_average_power(void);
uint16_t bsp_battery_get_time_to_empty(void);
uint16_t bsp_battery_get_time_to_full(void);

// 高级配置 - 参数使用 uint16_t 表示枚举值，在 .c 内部转换
esp_err_t bsp_battery_seal(void);
esp_err_t bsp_battery_unseal(void);
esp_err_t bsp_battery_enter_config_update(void);
esp_err_t bsp_battery_exit_config_update(void);
esp_err_t bsp_battery_set_parameter_u16(uint16_t param_enum_value,
                                        uint16_t value);
uint16_t bsp_battery_get_parameter_u16(uint16_t param_enum_value);

#ifdef __cplusplus
}
#endif

#endif // BSP_BATTERY_H