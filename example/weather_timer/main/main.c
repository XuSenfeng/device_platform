/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-09 11:41:07
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-04 19:13:42
 */
#include <stdio.h>
#include "app_music.h"
#include "app_sr.h"
#include "device_bsp.h"
#include "lvgl.h"
#include "device_manage.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"

#include "app_wifi.h"
#include "app_ui.h"
#include "esp_lvgl_port.h"

extern lv_style_t main_style;

void app_main(void)
{
    printf("hello world\n");


    lv_style_init(&main_style);
    lv_style_set_radius(&main_style, 10);  // 设置圆角半径
    lv_style_set_bg_opa( &main_style, LV_OPA_50 );
    lv_style_set_bg_color(&main_style, lv_color_hex(0x080808));
    lv_style_set_border_width(&main_style, 0);
    lv_style_set_pad_all(&main_style, 10);
    lv_style_set_width(&main_style, 320);  // 设置宽
    lv_style_set_height(&main_style, 240); // 设置高
    lv_style_set_text_color(&main_style, lv_color_hex(0xdfdfdf));

    device_init(APP_TIMER_DEVICE);
    device_init(DEVICE_LCD_NUM);

    init_ui();
    
    if(!init_get_mode()){
        // 设备初始化
        device_init(DEVICE_CAMERA_NUM);
    }

    ESP_LOGI("init_mode", "get init_mode: %d", init_get_mode());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);

    if(!init_get_mode()){
        ESP_LOGI("app_main", "register music ui");
        app_ui_init(APP_MUSIC);
        app_ui_show_id(UI_ID_MUSIC);
    }else{
        ESP_LOGI("app_main", "register weather ui");
        app_ui_init(APP_WEATHER);
        // // 注册Wifi页面
        app_ui_show_id(UI_ID_WIFI);
    }
    while(1){
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
