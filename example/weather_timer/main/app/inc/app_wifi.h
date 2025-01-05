/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-15 10:00:06
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-04 15:53:02
 */
#pragma once
#include "esp_err.h"
void app_wifi_connect(void* data);
void app_wifi_deinit(void);

#define WIFI_UI_REGISTER_CODE   {                                                       \
                                    ui_config_t *wifi_ui = malloc(sizeof(ui_config_t)); \
                                    memset(wifi_ui, 0, sizeof(ui_config_t));            \
                                    wifi_ui->init = app_wifi_connect;                   \
                                    wifi_ui->deinit = app_wifi_deinit;                  \
                                    wifi_ui->ID = UI_ID_WIFI;                           \
                                    app_ui_add(wifi_ui);                                \
                                }while(0);
void example_wifi_shutdown(void);
esp_err_t example_wifi_reconnect(void);
