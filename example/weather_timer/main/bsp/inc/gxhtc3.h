#ifndef GXHTC3_H
#define GXHTC3_H
#include "esp_err.h"
/********************
 * 温湿度传感器
 * ******************/
typedef struct{
    float temp, humi;
    uint8_t temp_int, humi_int;
} gxhtc3_t;


#define GXTC3_REGISTER  {                                                   \
                            device = (device_t *)malloc(sizeof(device_t));  \
                            memset(device, 0, sizeof(device_t));            \
                            device->name = "gxhtc3";                        \
                            device->init = gxhtc3_init;                     \
                            device->read = gxhtc3_read;                     \
                            device->id = DEVICE_GXHTC3_NUM;                 \
                            device->state = gxhtc3_state;                   \
                            device_register(device);                        \
                        }while(0);

void gxhtc3_init(void);
int gxhtc3_read(void * data, uint32_t len);
esp_err_t gxhtc3_state(void);
esp_err_t gxhtc3_deinit(void);
#endif /*GXHTC3_H*/