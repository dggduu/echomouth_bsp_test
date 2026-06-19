#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "esp_err.h"
#include "i2c_bus.h"
#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bsp_i2c_init_main(void);
esp_err_t bsp_i2c_init_bat(void);
i2c_bus_handle_t bsp_i2c_get_main_bus(void);
i2c_bus_handle_t bsp_i2c_get_bat_bus(void);
int bsp_i2c_get_main_port(void);
int bsp_i2c_get_bat_port(void);

esp_err_t bsp_i2c_write_main(uint8_t addr_7bit, const uint8_t *data, size_t len,
                             int timeout_ms);
esp_err_t bsp_i2c_read_main(uint8_t addr_7bit, uint8_t *data, size_t len,
                            int timeout_ms);
esp_err_t bsp_i2c_write_read_main(uint8_t addr_7bit, const uint8_t *write_data,
                                  size_t write_len, uint8_t *read_data,
                                  size_t read_len, int timeout_ms);

esp_err_t bsp_i2c_write_bat(uint8_t addr_7bit, const uint8_t *data, size_t len,
                            int timeout_ms);
esp_err_t bsp_i2c_read_bat(uint8_t addr_7bit, uint8_t *data, size_t len,
                           int timeout_ms);
esp_err_t bsp_i2c_write_read_bat(uint8_t addr_7bit, const uint8_t *write_data,
                                 size_t write_len, uint8_t *read_data,
                                 size_t read_len, int timeout_ms);

void bsp_i2c_scan(i2c_bus_handle_t bus);

#ifdef __cplusplus
}
#endif

#endif // BSP_I2C_H