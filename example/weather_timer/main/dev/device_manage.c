/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-10-18 21:34:08
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-04 12:39:33
 */
#include "device_manage.h"
#include "esp_log.h"
#include <string.h>
#include "device_platform.h"
#include "device_bsp.h"

static const char * TAG = "device";

device_manage_t device_manage = {
    .head = NULL,
    .tail = NULL,
    .count = 0,
    .mutex = NULL
};
#define FIRST_I2C       0x1
#define FIRST_EXTEND_IO 0x2
#define FIRST_FLATFORM  0x4

int first_time = 0;
void device_init(int device_to_init)
{
    // Initialize the sensor
    // ...
    device_manage.mutex = xSemaphoreCreateMutex();
    ESP_LOGI(TAG, "device_init");
    device_t *device = NULL;
// #if DEVICE_LIST & REQUIRE_I2C
    if((device_to_init & REQUIRE_I2C) || ((first_time & FIRST_I2C) == 0)){
        ESP_LOGI(TAG, "i2c_master_init");
        first_time |= FIRST_I2C;
        i2c_master_init();
    }
// #endif

// #if DEVICE_LIST & REQUIRE_EXTENDIO
#if BOARD_S3
    if((device_to_init & REQUIRE_EXTENDIO) || ((first_time & FIRST_EXTEND_IO) == 0)){
        ESP_LOGI(TAG, "pca9557_init");
        first_time |= FIRST_EXTEND_IO;
        pca9557_init();
    }
#endif
// #endif

    //初始化设备
// #if DEVICE_LIST & DEVICE_GXHTC3_NUM
    if(device_to_init & DEVICE_GXHTC3_NUM){
        // 添加设备rxhtc3
        GXTC3_REGISTER
    }
// #endif

// #if DEVICE_LIST & DEVICE_QMI8658C_NUM
    if(device_to_init & DEVICE_QMI8658C_NUM){
        // 添加设备qmi8658c
        QMI8658C_REGISTER
    }
// #endif

// #if DEVICE_LIST & DEVICE_QMC5883L_NUM
    if(device_to_init & DEVICE_QMC5883L_NUM){
        QMC5883L_REGISTER
    }
// #endif 

// #if DEVICE_LIST & DEVICE_LCD_NUM
    if(device_to_init & DEVICE_LCD_NUM){
        LCD_REGISTER
    }
// #endif

// #if DEVICE_LIST & DEVICE_MUSIC_NUM
    if(device_to_init & DEVICE_MUSIC_NUM){
        MUSIC_REGISTER
    }
// #endif

// #if DEVICE_LIST & DEVICE_TTV_NUM
    if(device_to_init & DEVICE_TTV_NUM){
        TTV_REGISTER
    }
// #endif


// #if DEVICE_LIST & DEVIDE_SD_CARD_NUM
    if(device_to_init & DEVIDE_SD_CARD_NUM){
        SD_CARD_REGISTER
    }
// #endif

    if(device_to_init & DEVICE_CAMERA_NUM){
        CAMERA_REGISTER
    }

    // 初始化设备
    device = device_manage.head;
    while(device != NULL){
        if((device_to_init & device->id) && (device->init != NULL)){
            ESP_LOGI(TAG, "device_init id:%d, name:%s", (int)device->id, device->name);
            device->init();
        }
        device = device->next;
    }
    if((first_time & FIRST_FLATFORM)== 0){
        dev_plt_init_t dev_plt_init_config;
        dev_plt_init_config.channel = DEVICE_CHANNEL;
        dev_plt_init_config.connect_mode = DEVICE_CHANNEL_MODE;
        dev_plt_init(&dev_plt_init_config);
        first_time |= FIRST_FLATFORM;
    }

}
/// @brief 设备的去初始化
/// @param  
void device_deinit(uint32_t id)
{
    // Deinitialize the sensor
    // ...
    ESP_LOGI(TAG, "device_deinit");
    device_t *device = device_find(id);
    if(device != NULL && device->deinit != NULL){
        xSemaphoreTake(device_manage.mutex, portMAX_DELAY); //获取互斥锁
        device->deinit();
        xSemaphoreGive(device_manage.mutex);
    }
}

/// @brief 注册一下设备
/// @param device 要注册的设备
void device_register(device_t *device)
{
    // Register the sensor
    // ...
    ESP_LOGI(TAG, "device_register");
    xSemaphoreTake(device_manage.mutex, portMAX_DELAY); //获取互斥锁
    //把设备加入到设备管理链表中
    if(device_manage.head == NULL){
        device_manage.head = device;
        device_manage.tail = device;
    }
    else{
        device_manage.tail->next = device;
        device_manage.tail = device;
    }
    xSemaphoreGive(device_manage.mutex);
}

/// @brief 使用id查找设备
/// @param id 设备的id
/// @return 找到的设备, 没有的话返回NULL
device_t * device_find(uint32_t id)
{
    // Find the sensor
    // ...
    // ESP_LOGI(TAG, "device_find");
    device_t *device = device_manage.head;
    while(device != NULL){
        if(device->id == id){
            // ESP_LOGI(TAG, "device_find id:%d, name:%s", (int)device->id, device->name);
            break;
        }
        device = device->next;
    }
    return device;
}

/// @brief 读取数据
/// @param id 
/// @param data 
/// @param len 
/// @param reomte_read 获取失败的时候是不是尝试从远程获取数据
int device_read(uint32_t id, void *data, uint32_t len, uint8_t remote_read)
{
    // Read the sensor
    // ...
    // ESP_LOGI(TAG, "device_read");
    device_t *device = device_find(id);
    int err = ESP_FAIL;
    if(device != NULL && device->read != NULL){
        xSemaphoreTake(device_manage.mutex, portMAX_DELAY); //获取互斥锁
        err = device->read(data, len);
        xSemaphoreGive(device_manage.mutex);
    }else if(remote_read){
        // 尝试从远程获取数据
        xSemaphoreTake(device_manage.mutex, portMAX_DELAY); //获取互斥锁
        err = dev_plt_get_data(id, data, len);
        xSemaphoreGive(device_manage.mutex);
    }
    
    return err;
}
/// @brief 对一个设备进行写入
/// @param id 设备的id
/// @param data 要写入的数据
/// @param len 数据的长度
/// @param remote_write 是不是尝试从远程写入
/// @return 成功返回数据长度, 失败返回ESP_FAIL
int device_write(uint32_t id, void *data, uint32_t len, uint8_t remote_write){
    device_t *device = device_find(id);
    int err = ESP_FAIL;
    if(device != NULL && device->write != NULL){
        xSemaphoreTake(device_manage.mutex, portMAX_DELAY); //获取互斥锁
        err = device->write(data, len);
        xSemaphoreGive(device_manage.mutex);
    }else if(remote_write){
        // 尝试从远程获取数据
        xSemaphoreTake(device_manage.mutex, portMAX_DELAY); //获取互斥锁
        // err = dev_plt_get_data(id, data, len);
        ESP_LOGI(TAG, "device_write remote not support now");
        xSemaphoreGive(device_manage.mutex);
    }
    return err;
}
