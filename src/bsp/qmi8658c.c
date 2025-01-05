/*
 * @Descripttion: 六轴传感器驱动
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-10-19 12:19:55
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-11-28 11:03:45
 */

#include "gen_i2c.h"
#include "qmi8658c.h"
#include "esp_log.h"
#include "device_manage.h"
#include "math.h"
#include "device_bsp.h"

#if BOARD_C3 || BOARD_S3

static const char * TAG = "qmi8658c";

esp_err_t qmi8658c_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, QMI8658C_SENSOR_ADDR,  &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t qmi8658c_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};

    return i2c_master_write_to_device(I2C_MASTER_NUM, QMI8658C_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}


void qmi8658c_init_cmd(void)
{
    uint8_t id = 0;

    qmi8658c_register_read(QMI8658C_WHO_AM_I, &id ,1);
    while (id != 0x05)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        qmi8658c_register_read(QMI8658C_WHO_AM_I, &id ,1);
    }
    ESP_LOGI(TAG, "QMI8658C OK!");

    qmi8658c_register_write_byte(QMI8658C_RESET, 0xb0);  // 复位
    vTaskDelay(10 / portTICK_PERIOD_MS);                // 延时10ms
    qmi8658c_register_write_byte(QMI8658C_CTRL1, 0x40); // CTRL1 设置地址自动增加, 没有中断, 使用内部时钟
    qmi8658c_register_write_byte(QMI8658C_CTRL7, 0x03); // CTRL7 允许加速度和陀螺仪
    qmi8658c_register_write_byte(QMI8658C_CTRL2, 0x95); // CTRL2 设置ACC 4g 250Hz
    qmi8658c_register_write_byte(QMI8658C_CTRL3, 0xd5); // CTRL3 设置GRY测试 512dps 250Hz
}

esp_err_t qmi8658c_state(void){
    uint8_t id = 0;
    qmi8658c_register_read(QMI8658C_WHO_AM_I, &id ,1);
    if (id == 0x05){
        return ESP_OK;
    }else
        return ESP_FAIL;
}

/// @brief 初始化函数
/// @param  
void qmi8658c_init(void)
{
    ESP_LOGI(TAG, "qmi8658c_init");
    Analog_keying_init();
    qmi8658c_init_cmd();
}

/// @brief 获取加速度和陀螺仪数据
/// @param p 
esp_err_t qmi8658c_Read_AccAndGry(sQMI8658C_t *p)
{
    int16_t buf[6];
    uint8_t status;
    qmi8658c_register_read(QMI8658C_STATUS0, &status, 1); // 读状态寄存器 
    if (status & 0x03){ // 判断加速度和陀螺仪数据是否可读
        qmi8658c_register_read(QMI8658C_AX_L, (uint8_t *)buf, 12); // 读加速度值
        p->acc_x = buf[0];
        p->acc_y = buf[1];
        p->acc_z = buf[2];
        p->gyr_x = buf[3];
        p->gyr_y = buf[4];
        p->gyr_z = buf[5];
        
        // printf("acc_x = %d, acc_y = %d, acc_z = %d, gyr_x = %d, gyr_y = %d, gyr_z = %d\n", p->acc_x, p->acc_y, p->acc_z, p->gyr_x, p->gyr_y, p->gyr_z);
        return ESP_OK;
    }else{
        printf("status = %d\n", status);
        ESP_LOGE(TAG, "qmi8658c_Read_AccAndGry error");
        qmi8658c_init_cmd();
        return ESP_FAIL;
    }
}

esp_err_t qmi8658c_fetch_angleFromAcc(sQMI8658C_t *p)
{
    float temp;
    esp_err_t ret = ESP_FAIL;
    ret = qmi8658c_Read_AccAndGry(p);
    if(ret != ESP_OK)
    {
        return ret;
    }
    temp = (float)p->acc_x / sqrt( ((float)p->acc_y * (float)p->acc_y + (float)p->acc_z * (float)p->acc_z) );
    p->AngleX = atan(temp)*57.3f; // 180/3.14=57.3
    temp = (float)p->acc_y / sqrt( ((float)p->acc_x * (float)p->acc_x + (float)p->acc_z * (float)p->acc_z) );
    p->AngleY = atan(temp)*57.3f; // 180/3.14=57.3
    temp = (float)p->acc_z / sqrt( ((float)p->acc_x * (float)p->acc_x + (float)p->acc_y * (float)p->acc_y) );
    p->AngleZ = atan(temp)*57.3f; // 180/3.14=57.3
    return sizeof(sQMI8658C_t);
}

int qmi8658c_read(void *data, uint32_t len)
{
    if(len < sizeof(sQMI8658C_t) || data == NULL)
    {
        ESP_LOGE(TAG, "len is too short");
        return ESP_FAIL;
    }
    sQMI8658C_t *p = (sQMI8658C_t *)data;
    return qmi8658c_fetch_angleFromAcc(p);
}

#include "driver/gpio.h"

#define KEY_GPIO_NUM            0
/// @brief 初始化开发板的原始的按键
/// @param  
void Analog_keying_init(void){
    //初始化一下实体按键
    gpio_config_t gpio_t = 
    {
        .intr_type = GPIO_INTR_DISABLE,     //禁止中断
        .mode = GPIO_MODE_INPUT,           //输出模式
        .pin_bit_mask = (1ull<<KEY_GPIO_NUM),   //GPIO引脚号
        .pull_down_en = GPIO_PULLDOWN_DISABLE,//下拉禁止
        .pull_up_en = GPIO_PULLUP_DISABLE,   //上拉禁止, 有外置的上拉
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_t));
}
/// @brief 按键的几个状态
enum Analog_keying{
    KEY_NONE = 0,       // 没有按键
    KEY_DEBOUNCE,       // 按键消抖
    KEY_SHORT,          // 按键短按
    KEY_LONG,           // 按键长按
};
/// @brief 读取五次数据取平均值, 返回角度
/// @param angleX 
/// @param angleY 
void read_average_data(float *angleX, float *angleY){
    sQMI8658C_t angle;
    int i = 0;
    float Angle_X = 0, Angle_Y = 0;
    for(i = 0; i < 5; i++){
        int len = qmi8658c_read(&angle, sizeof(sQMI8658C_t));
        // printf("AngleX = %f, AngleY = %f ", angle.AngleX, angle.AngleY);
        // float up = 80.0, down = -80.0;
        // printf(" %d %d %d %d\n", Angle_X <= 80.0 , Angle_X >= -80.0 , Angle_Y <= 80.0 , Angle_Y >= -80.0);
        if(len == sizeof(sQMI8658C_t)){
            Angle_X += angle.AngleX;
            Angle_Y += angle.AngleY;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    *angleX = Angle_X / 5;
    *angleY = Angle_Y / 5;
}

/// @brief 按键检测的任务, 实际上是被lvgl循环调用
/// @param indev_drv 
/// @param data 
void Analog_keying_task(lv_indev_drv_t * indev_drv, lv_indev_data_t * data){
    //获取按键
    static enum Analog_keying key_status;
    static float angle_X, angle_Y;
    float angle_n_X, angle_n_Y;
    int key = gpio_get_level(KEY_GPIO_NUM);
    uint32_t act_key = 0;
    if(key == 0){
        data->state = LV_INDEV_STATE_PR;
        // 按键按下
        switch (key_status)
        {
        case KEY_NONE:
            key_status = KEY_DEBOUNCE;
            //按键刚刚按下, 进行一下消抖
            break;
        case KEY_DEBOUNCE:
            //消抖成功
            key_status = KEY_SHORT;
            bsp_display_brightness_reset();
            //获取一下当前的角度
            // qmi8658c_read(&angle, sizeof(sQMI8658C_t));
            read_average_data(&angle_X, &angle_Y);
            break;
        case KEY_SHORT:
            key_status = KEY_LONG;
            if(if_camer_show){
                to_photo_flag = 1;
            }
            //按键按下时间超过1s, 进行长按
            break;
        case KEY_LONG:
            //按键长按
            read_average_data(&angle_n_X, &angle_n_Y);
            float down = -20.0, top = 20.0;
            if(angle_n_X > top){
                //向下
                printf("向下\n");
                act_key = LV_KEY_DOWN;
            }else if(angle_n_X < down){
                //向上
                printf("向上\n");
                act_key = LV_KEY_UP;
            }
            if(angle_n_Y > top){
                //向右
                printf("向右\n");
                act_key = LV_KEY_RIGHT;
            }else if(angle_n_Y < down){
                //向左
                printf("向左\n");
                act_key = LV_KEY_LEFT;
            }
            break;
        }
    }else{
        if(key_status == KEY_LONG){
            data->state = LV_INDEV_STATE_PR;
            printf("ENTER\n");
            act_key = LV_KEY_ENTER;
        }else{
            data->state = LV_INDEV_STATE_REL;
        }
        // 按键松开
        key_status = KEY_NONE;
        angle_X = 0;
        angle_Y = 0;
    }
    data->key = act_key;
}

#endif