#ifndef GENI2C_H
#define GENI2C_H

#include "esp_err.h"
#include "driver/i2c.h"
esp_err_t i2c_master_init(void);


#define I2C_MASTER_SCL_IO           GPIO_NUM_2      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           GPIO_NUM_1      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              1               /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          50000          /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0               /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0               /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000            /*!< I2C timeout in milliseconds */

/*

#define BSP_I2C_SDA           (GPIO_NUM_1)   // SDA引脚
#define BSP_I2C_SCL           (GPIO_NUM_2)   // SCL引脚

#define BSP_I2C_NUM           (1)            // I2C外设
#define BSP_I2C_FREQ_HZ       50000         // 100kHz

esp_err_t bsp_i2c_init(void);   // 初始化I2C接口

*/

#endif /*GENI2C_H*/