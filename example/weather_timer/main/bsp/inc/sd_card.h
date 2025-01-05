/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-15 18:42:06
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-11-30 09:59:26
 */
#ifndef SD_CARD_H
#define SD_CARD_H

#include "esp_err.h"
#include "esp_log.h"
#include "driver/sdmmc_host.h"
#include "file_iterator.h"

esp_err_t s_example_read_file(const char *path);
void sd_card_init(void *data);
esp_err_t s_example_write_file(const char *path, char *data);
extern file_iterator_instance_t *file_iterator_sd;
#define SD_CARD_REGISTER    {                                                   \
                                device = (device_t *)malloc(sizeof(device_t));  \
                                memset(device, 0, sizeof(device_t));            \
                                device->name = "qmi8658c";                      \
                                device->init = sd_card_init;                   \
                                device->read = NULL;                            \
                                device->state = NULL;                           \
                                device->id = DEVIDE_SD_CARD_NUM;                \
                                device->state = NULL;                           \
                                device_register(device);                        \
                            }while(0);

#endif
