/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-10 09:09:31
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-11-24 11:01:00
 */
#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "app_ui.h"
// 使用map的方式来实现设备管理
#define DEVICE_GXHTC3_NUM   0x1
#define DEVICE_QMI8658C_NUM 0x2
#define DEVICE_QMC5883L_NUM 0x4
#define DEVICE_LCD_NUM      (0x8 | DEVICE_QMI8658C_NUM)
#define DEVICE_MUSIC_NUM    0x10
#define DEVICE_TTV_NUM      (DEVICE_MUSIC_NUM | 0x20)
#define DEVIDE_SD_CARD_NUM  0x40
#define DEVICE_CAMERA_NUM   0x80



// 启用的设备管理
#if APP_REGISTER & UI_ID_TIMER
#define DEVICE_LIST  APP_TIMER_DEVICE
#elif APP_REGISTER & UI_ID_MUSIC
#define DEVICE_LIST  APP_MUSIC_DEVICE
#else 
#define DEVICE_LIST  (DEVICE_QMI8658C_NUM | DEVICE_LCD_NUM | DEVICE_MUSIC_NUM | DEVICE_TTV_NUM | DEVIDE_SD_CARD_NUM)
#endif

#define REQUIRE_I2C  (DEVICE_GXHTC3_NUM | DEVICE_QMI8658C_NUM | DEVICE_QMC5883L_NUM | DEVICE_LCD_NUM | DEVICE_MUSIC_NUM | DEVICE_TTV_NUM)
#define REQUIRE_EXTENDIO    (DEVICE_LCD_NUM | DEVICE_MUSIC_NUM | DEVICE_TTV_NUM)

typedef struct _device_t{
    struct _device_t *next; //指向下一个设备
    uint32_t id; //设备ID
    char *name; //设备名称
    void *data; //设备数据
    void (*init)(void); //设备初始化函数
    void (*deinit)(void); //设备去初始化函数
    int (*read)(void * data, uint32_t len); //设备读取函数
    int (*write)(char *data, int len); //设备写入函数
    void (*ioctl)(void); //设备控制函数
    esp_err_t (*state)(void); //设备状态函数
}device_t;

typedef struct  
{
    device_t *head;
    device_t *tail; //指向最后一个设备
    int count; //设备数量
    SemaphoreHandle_t mutex;
}device_manage_t;

void device_init(int device_to_init);
void device_deinit(uint32_t id);
void device_register(device_t *device);
int device_read(uint32_t id, void *data, uint32_t len, uint8_t remote_read);
device_t * device_find(uint32_t id);
int device_write(uint32_t id, void *data, uint32_t len, uint8_t remote_write);
#endif /*DEVICE_H*/
