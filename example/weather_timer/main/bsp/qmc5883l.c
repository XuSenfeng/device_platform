/*
 * @Descripttion: 地磁传感器QMC5883L驱动
 * @version: 0.0.1
 * @Author: XvSenfeng
 * @Date: 2024-10-20 22:29:38
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-11-10 09:54:53
 */


#include "qmc5883l.h"
#include "esp_err.h"
#include "esp_log.h"
#include "dev/device_manage.h"
#include "bsp/general/gen_i2c.h"
#include <math.h>


// #if BOARD_C3

static const char *TAG = "qmc5883l";


esp_err_t qmc5883L_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, QMC5883L_SENSOR_ADDR,  &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t qmc5883L_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};

    return i2c_master_write_to_device(I2C_MASTER_NUM, QMC5883L_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}


void qmc5883l_init_cmd(void)
{
    uint8_t id = 0;

    ESP_ERROR_CHECK(qmc5883L_register_read(QMC5883L_CHIPID, &id ,1));
    while (id != 0xff)  // 确定ID号是否正确
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_ERROR_CHECK(qmc5883L_register_read(QMC5883L_CHIPID, &id ,1));
    }
    ESP_LOGI(TAG, "QMC5883L OK!");
    ESP_ERROR_CHECK(qmc5883L_register_write_byte(QMC5883L_CTRL2, 0x80)); // 复位芯片 
    vTaskDelay(10 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(qmc5883L_register_write_byte(QMC5883L_CTRL1, 0x05)); //Continuous模式 50Hz 
    ESP_ERROR_CHECK(qmc5883L_register_write_byte(QMC5883L_CTRL2, 0x00)); 
    ESP_ERROR_CHECK(qmc5883L_register_write_byte(QMC5883L_FBR, 0x01)); 
}

esp_err_t qmc5833l_state(void){
    uint8_t id = 0;
    ESP_ERROR_CHECK(qmc5883L_register_read(QMC5883L_CHIPID, &id ,1));
    if(id != 0xff){
        return ESP_FAIL;
    }else{
        return ESP_OK;
    }
}

/// @brief 初始化地磁传感器
/// @param  
void qmc5883l_init(void){

    qmc5883l_init_cmd();
}



/// @brief 读取数据
/// @param p 
esp_err_t qmc5883l_read_xyz(sQMC5883L_t* p)
{
    
    uint8_t status;
    int16_t mag_reg[3];

    qmc5883L_register_read(QMC5883L_STATUS, &status, 1); // 读状态寄存器 
    
    if (status & 0x01)
    {
        qmc5883L_register_read(QMC5883L_XOUT_L, (uint8_t *)mag_reg, 6);

        p->mag_x = mag_reg[0];
        p->mag_y = mag_reg[1];
        p->mag_z = mag_reg[2];
        return ESP_OK;
    }else{
        return ESP_FAIL;
    }
}

int qmc5883l_read(void *data, uint32_t len)
{
    sQMC5883L_t * p = (sQMC5883L_t *)data;
    if(len < sizeof(sQMC5883L_t) || p == NULL)
    {
        ESP_LOGE(TAG, "len is too short");
        return ESP_FAIL;
    }
    esp_err_t err = qmc5883l_read_xyz(p);
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "read data fail");
        return ESP_FAIL;
    }
    p->azimuth = (float)atan2(p->mag_y, p->mag_x) * 180.0 / 3.1415926 + 180.0;
    return sizeof(sQMC5883L_t);
}

// #endif