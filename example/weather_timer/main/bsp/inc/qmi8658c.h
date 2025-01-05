/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-10 09:09:31
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-12-06 21:51:44
 */
#ifndef QMI8658C_H
#define QMI8658C_H
#include "lvgl.h"
/*******************
 * 姿态传感器数据
 *******************/
enum qmi8658c_reg
{
    QMI8658C_WHO_AM_I,
    QMI8658C_REVISION_ID,
    QMI8658C_CTRL1,
    QMI8658C_CTRL2,
    QMI8658C_CTRL3,
    QMI8658C_CTRL4,
    QMI8658C_CTRL5,
    QMI8658C_CTRL6,
    QMI8658C_CTRL7,
    QMI8658C_CTRL8,
    QMI8658C_CTRL9,
    QMI8658C_CATL1_L,
    QMI8658C_CATL1_H,
    QMI8658C_CATL2_L,
    QMI8658C_CATL2_H,
    QMI8658C_CATL3_L,
    QMI8658C_CATL3_H,
    QMI8658C_CATL4_L,
    QMI8658C_CATL4_H,
    QMI8658C_FIFO_WTM_TH,
    QMI8658C_FIFO_CTRL,
    QMI8658C_FIFO_SMPL_CNT,
    QMI8658C_FIFO_STATUS,
    QMI8658C_FIFO_DATA,
    QMI8658C_I2CM_STATUS = 44,
    QMI8658C_STATUSINT,
    QMI8658C_STATUS0,
    QMI8658C_STATUS1,
    QMI8658C_TIMESTAMP_LOW,
    QMI8658C_TIMESTAMP_MID,
    QMI8658C_TIMESTAMP_HIGH,
    QMI8658C_TEMP_L,
    QMI8658C_TEMP_H,
    QMI8658C_AX_L,
    QMI8658C_AX_H,
    QMI8658C_AY_L,
    QMI8658C_AY_H,
    QMI8658C_AZ_L,
    QMI8658C_AZ_H,
    QMI8658C_GX_L,
    QMI8658C_GX_H,
    QMI8658C_GY_L,
    QMI8658C_GY_H,
    QMI8658C_GZ_L,
    QMI8658C_GZ_H,
    QMI8658C_MX_L,
    QMI8658C_MX_H,
    QMI8658C_MY_L,
    QMI8658C_MY_H,
    QMI8658C_MZ_L,
    QMI8658C_MZ_H,
    QMI8658C_dQW_L = 73,
    QMI8658C_dQW_H,
    QMI8658C_dQX_L,
    QMI8658C_dQX_H,
    QMI8658C_dQY_L,
    QMI8658C_dQY_H,
    QMI8658C_dQZ_L,
    QMI8658C_dQZ_H,
    QMI8658C_dVX_L,
    QMI8658C_dVX_H,
    QMI8658C_dVY_L,
    QMI8658C_dVY_H,
    QMI8658C_dVZ_L,
    QMI8658C_dVZ_H,
    QMI8658C_AE_REG1,
    QMI8658C_AE_REG2,
    QMI8658C_RESET = 96
};

typedef struct{
    int16_t acc_y;
    int16_t acc_x;
    int16_t acc_z;
    int16_t gyr_y;
    int16_t gyr_x;
    int16_t gyr_z;
    float AngleX;
    float AngleY;
    float AngleZ;
}sQMI8658C_t;

#define  QMI8658C_SENSOR_ADDR       0x6A


void Analog_keying_task(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
int qmi8658c_read(void *data, uint32_t len);
void qmi8658c_init(void);
esp_err_t qmi8658c_state(void);
void Analog_keying_init(void);
#define QMI8658C_REGISTER   {                                                   \
                                device = (device_t *)malloc(sizeof(device_t));  \
                                memset(device, 0, sizeof(device_t));            \
                                device->name = "qmi8658c";                      \
                                device->init = qmi8658c_init;                   \
                                device->read = qmi8658c_read;                   \
                                device->state = qmi8658c_state;                 \
                                device->id = DEVICE_QMI8658C_NUM;               \
                                device_register(device);                        \
                            }while(0);
#endif /*QMI8658C_H*/