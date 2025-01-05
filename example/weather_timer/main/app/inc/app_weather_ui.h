/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-17 10:08:20
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-04 11:31:42
 */
#ifndef APP_WEATHER_UI_H
#define APP_WEATHER_UI_H

#include "esp_log.h"
#include "lvgl.h"
void weather_main_page(void * data);
void wifi_main_page_deinit(void);
void timer_read_timer();
char* num_to_str(int num, char *str);
void timer_read_weather();
void lv_app_page(lv_obj_t *base, lv_obj_t *app_page);
#define  WEATHER_TIMER_UI_REGISTER_CODE   {                                                       \
                                    ui_config_t *weather_ui = malloc(sizeof(ui_config_t)); \
                                    memset(weather_ui, 0, sizeof(ui_config_t));            \
                                    weather_ui->init = weather_main_page;                   \
                                    weather_ui->deinit = wifi_main_page_deinit;                  \
                                    weather_ui->ID = UI_ID_TIMER;                           \
                                    app_ui_add(weather_ui);                                \
                                }while(0);
// Wifi程序界面使用的元素
typedef struct{
    int temp_value, humi_value; //室内温湿度, 暂时不能获取
    lv_obj_t * label_wifi;
    lv_obj_t * label_sntp;
    lv_obj_t * label_weather;

    lv_obj_t * qweather_icon_label;
    lv_obj_t * qweather_temp_label;
    lv_obj_t * qweather_text_label;
    lv_obj_t * qair_level_obj;
    lv_obj_t * qair_level_label;
    lv_obj_t * led_time_label;
    lv_obj_t * week_label;
    lv_obj_t * sunset_label;
    lv_obj_t *indoor_temp_label;
    lv_obj_t *indoor_humi_label;
    lv_obj_t *outdoor_temp_label;
    lv_obj_t *outdoor_humi_label;
    lv_obj_t * date_label;
    lv_obj_t *tileview;
    int reset_flag;
    int th_update_flag;
    int qwnow_update_flag;
    int qair_update_flag;
    int qwdaily_update_flag;
    lv_timer_t update_timer;
}lv_wifi_t;

extern lv_style_t main_style;          // 当前的出题
extern lv_wifi_t *wifi_ui;             // 当前的页面元素
extern lv_obj_t *text_context_windows; // 当前使用的文本框
#endif