/*
 * @Descripttion: 一个设备管理平台, 用于连接以及管理远程的各种设备
 * @version: 0.0.1
 * @Author: XvSenfeng
 * @Date: 2024-10-26 09:42:11
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-11-21 23:07:57
 */
#ifndef DEVICE_PLATFORM_H
#define DEVICE_PLATFORM_H

#define DEVICE_CHANNEL1_SDA_IO 0
#define DEVICE_CHANNEL1_SCL_IO 1

#define DEVICE_CHANNEL2_GPIO1 10
#define DEVICE_CHANNEL2_GPIO2 11

#define DEVIDE_CHANNEL2_MODE_UART 0
#define DEVIDE_CHANNEL2_MODE_I2C 1
#define DEVIDE_CHANNEL2_MODE_SPI 2

#define DEVICE_MASTER           0
#define DEVICE_SLAVE            1

#define DEV_PLT_USART_NUM               UART_NUM_2
#define RD_BUF_SIZE             1024

#define DEVICE_MODE             DEVICE_MASTER
/***********************USER SET****************************/
#define DEVICE_CHANNEL          2
#define DEVICE_CHANNEL_MODE    DEVIDE_CHANNEL2_MODE_UART
/***********************************************************/
#include <stdint.h>
#include "esp_err.h"

typedef struct{
    int channel;
    int connect_mode;
}dev_plt_init_t;

#pragma pack(1)

/// @brief 设备平台交流的数据格式
typedef struct{
    uint32_t len;  //从机的时候记录主机需要获取的数据的长度, 主机的时候记录返回的数据的长度, 数据在结构体后面
    char direction;
    uint32_t device_num;
}dev_data_t;

#pragma pack()
typedef struct _wait_data{
    uint32_t device_num;
    char * data;
    uint32_t len;
    struct _wait_data * next;
}wait_data_t;


void dev_plt_task(void * data);
esp_err_t dev_plt_init(dev_plt_init_t * dev_plt_init);
esp_err_t dev_plt_get_data(uint32_t device_num, char * data, uint32_t len);
#endif 
