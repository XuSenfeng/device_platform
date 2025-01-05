/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-10 09:09:31
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-11-25 19:41:31
 */
#ifndef QMC5883L_H
#define QMC5883L_H

#define  QMC5883L_SENSOR_ADDR    0x0D
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

/**************
 * qmc5883l地磁传感器
 **************/
enum qmc5883l_reg
{
    QMC5883L_XOUT_L,
    QMC5883L_XOUT_H,
    QMC5883L_YOUT_L,
    QMC5883L_YOUT_H,
    QMC5883L_ZOUT_L,
    QMC5883L_ZOUT_H,
    QMC5883L_STATUS,
    QMC5883L_TOUT_L,
    QMC5883L_TOUT_H,
    QMC5883L_CTRL1,
    QMC5883L_CTRL2,
    QMC5883L_FBR,
    QMC5883L_CHIPID = 13
};
typedef struct{
    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;
    float   azimuth;
}sQMC5883L_t;

#define QMC5883L_REGISTER   {                                                   \
                                device = (device_t *)malloc(sizeof(device_t));  \
                                memset(device, 0, sizeof(device_t));            \
                                device->name = "qmc5883l";                      \
                                device->init = qmc5883l_init;                   \
                                device->read = qmc5883l_read;                   \
                                device->id = DEVICE_QMC5883L_NUM;               \
                                device->state = qmc5833l_state;                 \
                                device_register(device);                        \
                            }while(0);


int qmc5883l_read(void *data, uint32_t len);
void qmc5883l_init(void);
esp_err_t qmc5833l_state(void);
#endif // QMC5883L_H


