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

#ifdef __cplusplus
}
#endif

#endif // BSP_BATTERY_H