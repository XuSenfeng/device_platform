/*
 * @Descripttion: 温湿度传感器控制实际驱动函数
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-10-18 21:39:12
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-12-04 19:32:34
 */
#include "gxhtc3.h"
#include "esp_log.h"
#include "device_manage.h"
#include <math.h>
#include "device_bsp.h"

// #if BOARD_C3
static const char * TAG = "gxhtc3";

#define POLYNOMIAL  0x31 // P(x) = x^8 + x^5 + x^4 + 1 = 00110001

//CRC校验
uint8_t gxhtc3_calc_crc(uint8_t *crcdata, uint8_t len)
{
    uint8_t crc = 0xFF; 
  
    for(uint8_t i = 0; i < len; i++)
    {
        crc ^= (crcdata[i]);
        for(uint8_t j = 8; j > 0; --j)
        {
            if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
            else           crc = (crc << 1);
        }
    }
    return crc;
}

// 读取ID
esp_err_t gxhtc3_read_id(void)
{
    esp_err_t ret;
    uint8_t data[3];


    const uint8_t data1[] = {0xEF, 0xC8};
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, 0x70, data1, sizeof(data1), 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        goto end;
    }

    ret = i2c_master_read_from_device(I2C_MASTER_NUM, 0x70, data, 3, 1000 / portTICK_PERIOD_MS);
    if(data[2]!=gxhtc3_calc_crc(data,2)){     
        ret = ESP_FAIL;
    }
end:
    return ret;
}

// 初始化
void gxhtc3_init(void)
{
    // Initialize the sensor
    // ...
    ESP_LOGI(TAG, "gxhtc3_init");
    esp_err_t err = gxhtc3_read_id();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "gxhtc3_read_id OK");
    } else {
        ESP_LOGE(TAG, "gxhtc3_read_id failed");
    }
}

esp_err_t gxhtc3_state(void)
{
    // Get the sensor state
    // ...
    ESP_LOGI(TAG, "gxhtc3_state");
    esp_err_t err = gxhtc3_read_id();
    return err;
}

//唤醒
esp_err_t gxhtc3_wake_up(void)
{
    int ret;

    const uint8_t data[] = {0x35, 0x17};
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, 0x70, data, sizeof(data), 1000 / portTICK_PERIOD_MS);
    return ret;
}

// 测量
esp_err_t gxhtc3_measure(void)
{
    esp_err_t ret;

    const uint8_t data[] = {0x7c, 0xa2};
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, 0x70, data, sizeof(data), 1000 / portTICK_PERIOD_MS);
    return ret;
}

// 读出温湿度数据
esp_err_t gxhtc3_read_tah(uint8_t *tah_data)
{
    int ret;
    ret = i2c_master_read_from_device(I2C_MASTER_NUM, 0x70, tah_data, 6, 1000 / portTICK_PERIOD_MS);
    return ret;
}

// 休眠
esp_err_t gxhtc3_sleep(void)
{
    esp_err_t ret;
    const uint8_t data[] = {0xB0, 0x98};
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, 0x70, data, sizeof(data), 1000 / portTICK_PERIOD_MS);
    return ret;
}

float temp=0, humi=0;
uint8_t temp_int, humi_int;




/// @brief 获取并计算温湿度数据
/// @param data 数据结构体
/// @param len 长度
/// @return 返回值成功ESP_OK, 失败ESP_FAIL
int gxhtc3_read(void * data, uint32_t len)
{
    if(len < sizeof(gxhtc3_t) || data == NULL){
        return ESP_FAIL;
    }
    // Read the sensor data
    ESP_ERROR_CHECK(gxhtc3_state());

    gxhtc3_t *gxthc3 = (gxhtc3_t *)data;
    uint16_t rawValueTemp, rawValueHumi;

    uint8_t tah_data[6];
    //获取一次数据
    gxhtc3_wake_up();
    gxhtc3_measure();
    vTaskDelay(20 / portTICK_PERIOD_MS);
    gxhtc3_read_tah(tah_data);
    gxhtc3_sleep();
    //校验数据
    if((tah_data[2]!=gxhtc3_calc_crc(tah_data,2)||(tah_data[5]!=gxhtc3_calc_crc(&tah_data[3],2)))){     
        gxthc3->temp = 0;
        gxthc3->humi = 0;
        gxthc3->temp_int = 0;
        gxthc3->humi_int = 0;
        return ESP_FAIL;
    }
    else{
        rawValueTemp = (tah_data[0]<<8) | tah_data[1];
        rawValueHumi = (tah_data[3]<<8) | tah_data[4];
        gxthc3->temp = (175.0 * (float)rawValueTemp) / 65535.0 - 45.0; 
        gxthc3->humi = (100.0 * (float)rawValueHumi) / 65535.0;
        gxthc3->temp_int = round(temp);
        gxthc3->humi_int = round(humi);
        return sizeof(gxhtc3_t);
    }
}

esp_err_t gxhtc3_deinit(void)
{
    // Deinitialize the sensor
    return ESP_OK;
}

// #endif