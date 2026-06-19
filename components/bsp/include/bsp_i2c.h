#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "driver/i2c_master.h"
#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化主 I2C 总线（音频、PCA9539、触摸）
 */
esp_err_t bsp_i2c_init_main(void);

/**
 * @brief 初始化电池 I2C 总线（BQ27220）
 */
esp_err_t bsp_i2c_init_bat(void);

/**
 * @brief 获取主总线句柄
 */
i2c_master_bus_handle_t bsp_i2c_get_main_handle(void);

/**
 * @brief 获取电池总线句柄
 */
i2c_master_bus_handle_t bsp_i2c_get_bat_handle(void);

/**
 * @brief 主总线：写数据
 */
esp_err_t bsp_i2c_write_main(uint8_t addr_7bit, const uint8_t *data, size_t len,
                             int timeout_ms);

/**
 * @brief 主总线：读数据
 */
esp_err_t bsp_i2c_read_main(uint8_t addr_7bit, uint8_t *data, size_t len,
                            int timeout_ms);

/**
 * @brief 主总线：写-读组合（先写寄存器地址，再读）
 */
esp_err_t bsp_i2c_write_read_main(uint8_t addr_7bit, const uint8_t *write_data,
                                  size_t write_len, uint8_t *read_data,
                                  size_t read_len, int timeout_ms);

/**
 * @brief 电池总线：写数据
 */
esp_err_t bsp_i2c_write_bat(uint8_t addr_7bit, const uint8_t *data, size_t len,
                            int timeout_ms);

/**
 * @brief 电池总线：读数据
 */
esp_err_t bsp_i2c_read_bat(uint8_t addr_7bit, uint8_t *data, size_t len,
                           int timeout_ms);

/**
 * @brief 电池总线：写-读组合
 */
esp_err_t bsp_i2c_write_read_bat(uint8_t addr_7bit, const uint8_t *write_data,
                                 size_t write_len, uint8_t *read_data,
                                 size_t read_len, int timeout_ms);

/**
 * @brief 扫描 I2C 总线（调试用）
 */
void bsp_i2c_scan(i2c_master_bus_handle_t bus);

#ifdef __cplusplus
}
#endif

#endif // BSP_I2C_H