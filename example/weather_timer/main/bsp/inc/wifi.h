#ifndef WIFI_H
#define WIFI_H

void wifi_sta_init(void);



#define WIFI_REGISTER       {                                                   \
                                device = (device_t *)malloc(sizeof(device_t));  \
                                memset(device, 0, sizeof(device_t));            \
                                device->name = "WIFI";                           \
                                device->init = wifi_sta_init;                        \
                                device->read = NULL;                            \
                                device->write = NULL;               \
                                device->id = DEVICE_WIFI_NUM;                    \
                                device->state = NULL;                           \
                                device_register(device);                        \
                            }while(0);

#endif