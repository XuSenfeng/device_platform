#ifndef CAMERA_H
#define CAMERA_H
#include "esp_camera.h"

#define CAMERA_PIN_PWDN -1
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 5
#define CAMERA_PIN_SIOD 1
#define CAMERA_PIN_SIOC 2

#define CAMERA_PIN_D7 9
#define CAMERA_PIN_D6 4
#define CAMERA_PIN_D5 6
#define CAMERA_PIN_D4 15
#define CAMERA_PIN_D3 17
#define CAMERA_PIN_D2 8
#define CAMERA_PIN_D1 18
#define CAMERA_PIN_D0 16
#define CAMERA_PIN_VSYNC 3
#define CAMERA_PIN_HREF 46
#define CAMERA_PIN_PCLK 7


#define XCLK_FREQ_HZ 12000000
extern int if_camer_show;
extern int to_photo_flag;
void bsp_camera_init(void);
void app_camera_lcd(void);

#define CAMERA_REGISTER  {                                                   \
                            device = (device_t *)malloc(sizeof(device_t));  \
                            memset(device, 0, sizeof(device_t));            \
                            device->name = "camera";                        \
                            device->init = bsp_camera_init;                     \
                            device->read = NULL;                     \
                            device->id = DEVICE_CAMERA_NUM;                 \
                            device->state = NULL;                   \
                            device_register(device);                        \
                        }while(0);

#endif 