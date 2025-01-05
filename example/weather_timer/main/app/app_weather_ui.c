/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-17 09:14:33
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-05 09:31:40
 */
/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-17 09:14:33
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-12-08 13:54:36
 */
#include "app_weather_ui.h"
#include "esp_log.h"
#include "time.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "esp_netif_sntp.h"
#include "zlib.h"
#include "esp_tls.h"

#include "app_wifi.h"
#include "app_API.h"
#include "app_ui.h"
#include "app_weather_ui.h"

#include "app_API.h"
#include "device_bsp.h"
#include "device_manage.h"

#include "user_config.h"

static const char *TAG = "app_weather_ui";

LV_IMG_DECLARE(image_taikong);
// LV_FONT_DECLARE(font_alipuhui);
LV_FONT_DECLARE(beijing);
LV_FONT_DECLARE(font_qweather);
LV_FONT_DECLARE(font_led);
LV_FONT_DECLARE(font_myawesome);

extern time_t now;
extern struct tm timeinfo;
extern int qwdaily_tempMax;       // 当天最高温度
extern int qwdaily_tempMin;       // 当天最低温度
extern char qwdaily_sunrise[10];  // 当天日出时间
extern char qwdaily_sunset[10];   // 当天日落时间

extern int qwnow_temp; // 实时天气温度
extern int qwnow_humi; // 实时天气湿度
extern int qwnow_icon; // 实时天气图标
extern char qwnow_text[32]; // 实时天气状态

extern int qanow_level;       // 实时空气质量等级





lv_style_t main_style;
void lv_app_page(lv_obj_t *base, lv_obj_t *app_page);
lv_wifi_t *wifi_ui = NULL;
// 显示天气图标
void lv_qweather_icon_show(void)
{
    switch (qwnow_icon)
    {
        case 100: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x81"); strcpy(qwnow_text, "晴"); break;
        case 101: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x82"); strcpy(qwnow_text, "多云"); break;
        case 102: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x83"); strcpy(qwnow_text, "少云"); break;
        case 103: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x84"); strcpy(qwnow_text, "晴间多云"); break;
        case 104: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x85"); strcpy(qwnow_text, "阴"); break;
        case 150: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x86"); strcpy(qwnow_text, "晴"); break;
        case 151: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x87"); strcpy(qwnow_text, "多云"); break;
        case 152: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x88"); strcpy(qwnow_text, "少云"); break;
        case 153: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x89"); strcpy(qwnow_text, "晴间多云"); break;
        case 300: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x8A"); strcpy(qwnow_text, "阵雨"); break;
        case 301: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x8B"); strcpy(qwnow_text, "强阵雨"); break;
        case 302: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x8C"); strcpy(qwnow_text, "雷阵雨"); break;
        case 303: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x8D"); strcpy(qwnow_text, "强雷阵雨"); break;
        case 304: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x8E"); strcpy(qwnow_text, "雷阵雨伴有冰雹"); break;
        case 305: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x8F"); strcpy(qwnow_text, "小雨"); break;
        case 306: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x90"); strcpy(qwnow_text, "中雨"); break;
        case 307: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x91"); strcpy(qwnow_text, "大雨"); break;
        case 308: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x92"); strcpy(qwnow_text, "极端降雨"); break;
        case 309: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x93"); strcpy(qwnow_text, "毛毛雨"); break;
        case 310: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x94"); strcpy(qwnow_text, "暴雨"); break;
        case 311: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x95"); strcpy(qwnow_text, "大暴雨"); break;
        case 312: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x96"); strcpy(qwnow_text, "特大暴雨"); break;
        case 313: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x97"); strcpy(qwnow_text, "冻雨"); break;
        case 314: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x98"); strcpy(qwnow_text, "小到中雨"); break;
        case 315: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x99"); strcpy(qwnow_text, "中到大雨"); break;
        case 316: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x9A"); strcpy(qwnow_text, "大到暴雨"); break;
        case 317: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x9B"); strcpy(qwnow_text, "暴雨到大暴雨"); break;
        case 318: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x9C"); strcpy(qwnow_text, "大暴雨到特大暴雨"); break;
        case 350: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x9D"); strcpy(qwnow_text, "阵雨"); break;
        case 351: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x9E"); strcpy(qwnow_text, "强阵雨"); break;
        case 399: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\x9F"); strcpy(qwnow_text, "雨"); break;
        case 400: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xA0"); strcpy(qwnow_text, "小雪"); break;
        case 401: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xA1"); strcpy(qwnow_text, "中雪"); break;
        case 402: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xA2"); strcpy(qwnow_text, "大雪"); break;
        case 403: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xA3"); strcpy(qwnow_text, "暴雪"); break;
        case 404: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xA4"); strcpy(qwnow_text, "雨夹雪"); break;
        case 405: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xA5"); strcpy(qwnow_text, "雨雪天气"); break;
        case 406: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xA6"); strcpy(qwnow_text, "阵雨夹雪"); break;
        case 407: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xA7"); strcpy(qwnow_text, "阵雪"); break;
        case 408: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xA8"); strcpy(qwnow_text, "小到中雪"); break;
        case 409: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xA9"); strcpy(qwnow_text, "中到大雪"); break;
        case 410: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xAA"); strcpy(qwnow_text, "大到暴雪"); break;
        case 456: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xAB"); strcpy(qwnow_text, "阵雨夹雪"); break;
        case 457: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xAC"); strcpy(qwnow_text, "阵雪"); break;
        case 499: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xAD"); strcpy(qwnow_text, "雪"); break;
        case 500: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xAE"); strcpy(qwnow_text, "薄雾"); break;
        case 501: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xAF"); strcpy(qwnow_text, "雾"); break;
        case 502: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xB0"); strcpy(qwnow_text, "霾"); break;
        case 503: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xB1"); strcpy(qwnow_text, "扬沙"); break;
        case 504: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xB2"); strcpy(qwnow_text, "浮尘"); break;
        case 507: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xB3"); strcpy(qwnow_text, "沙尘暴"); break;
        case 508: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xB4"); strcpy(qwnow_text, "强沙尘暴"); break;
        case 509: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xB5"); strcpy(qwnow_text, "浓雾"); break;
        case 510: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xB6"); strcpy(qwnow_text, "强浓雾"); break;
        case 511: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xB7"); strcpy(qwnow_text, "中度霾"); break;
        case 512: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xB8"); strcpy(qwnow_text, "重度霾"); break;
        case 513: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xB9"); strcpy(qwnow_text, "严重霾"); break;
        case 514: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xBA"); strcpy(qwnow_text, "大雾"); break;
        case 515: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x84\xBB"); strcpy(qwnow_text, "特强浓雾"); break;
        case 900: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x85\x84"); strcpy(qwnow_text, "热"); break;
        case 901: lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x85\x85"); strcpy(qwnow_text, "冷"); break;
    
        default:
            printf("ICON_CODE:%d\n", qwnow_icon);
            lv_label_set_text(wifi_ui->qweather_icon_label, "\xEF\x85\x86");
            strcpy(qwnow_text, "未知天气");
            break;
    }
}

// 显示星期几
void lv_week_show(void)
{
    switch (timeinfo.tm_wday)
    {
        case 0: lv_label_set_text(wifi_ui->week_label, "星期日"); break;
        case 1: lv_label_set_text(wifi_ui->week_label, "星期一"); break;
        case 2: lv_label_set_text(wifi_ui->week_label, "星期二"); break;
        case 3: lv_label_set_text(wifi_ui->week_label, "星期三"); break;
        case 4: lv_label_set_text(wifi_ui->week_label, "星期四"); break; 
        case 5: lv_label_set_text(wifi_ui->week_label, "星期五"); break;
        case 6: lv_label_set_text(wifi_ui->week_label, "星期六"); break;
        default: lv_label_set_text(wifi_ui->week_label, "星期日"); break;
    }
}

// 显示空气质量
void lv_qair_level_show(void)
{
    switch (qanow_level)
    {
        case 1: 
            lv_label_set_text(wifi_ui->qair_level_label, "优"); 
            lv_obj_set_style_bg_color(wifi_ui->qair_level_obj, lv_palette_main(LV_PALETTE_GREEN), 0); 
            lv_obj_set_style_text_color(wifi_ui->qair_level_label, lv_color_hex(0xFFFFFF), 0);
            break;
        case 2: 
            lv_label_set_text(wifi_ui->qair_level_label, "良"); 
            lv_obj_set_style_bg_color(wifi_ui->qair_level_obj, lv_palette_main(LV_PALETTE_YELLOW), 0); 
            lv_obj_set_style_text_color(wifi_ui->qair_level_label, lv_color_hex(0x000000), 0);
            break;
        case 3: 
            lv_label_set_text(wifi_ui->qair_level_label, "轻");
            lv_obj_set_style_bg_color(wifi_ui->qair_level_obj, lv_palette_main(LV_PALETTE_ORANGE), 0); 
            lv_obj_set_style_text_color(wifi_ui->qair_level_label, lv_color_hex(0xFFFFFF), 0); 
            break;
        case 4: 
            lv_label_set_text(wifi_ui->qair_level_label, "中"); 
            lv_obj_set_style_bg_color(wifi_ui->qair_level_obj, lv_palette_main(LV_PALETTE_RED), 0); 
            lv_obj_set_style_text_color(wifi_ui->qair_level_label, lv_color_hex(0xFFFFFF), 0);
            break; 
        case 5: 
            lv_label_set_text(wifi_ui->qair_level_label, "重"); 
            lv_obj_set_style_bg_color(wifi_ui->qair_level_obj, lv_palette_main(LV_PALETTE_PURPLE), 0); 
            lv_obj_set_style_text_color(wifi_ui->qair_level_label, lv_color_hex(0xFFFFFF), 0);
            break;
        case 6: 
            lv_label_set_text(wifi_ui->qair_level_label, "严"); 
            lv_obj_set_style_bg_color(wifi_ui->qair_level_obj, lv_palette_main(LV_PALETTE_BROWN), 0); 
            lv_obj_set_style_text_color(wifi_ui->qair_level_label, lv_color_hex(0xFFFFFF), 0);
            break;
        default: 
            lv_label_set_text(wifi_ui->qair_level_label, "未"); 
            lv_obj_set_style_bg_color(wifi_ui->qair_level_obj, lv_palette_main(LV_PALETTE_GREEN), 0); 
            lv_obj_set_style_text_color(wifi_ui->qair_level_label, lv_color_hex(0xFFFFFF), 0);
            break;
    }
}
/// @brief 通过设备平台获取室内温度
/// @param  
void get_indoor_th(void)
{
    // 获取室内温湿度
    gxhtc3_t gxhtc3;
    device_read(DEVICE_GXHTC3_NUM, &gxhtc3, sizeof(gxhtc3_t), 1);
    wifi_ui->temp_value = gxhtc3.temp_int;
    wifi_ui->humi_value = gxhtc3.humi_int;
    wifi_ui->th_update_flag = 1;
    ESP_LOGI(TAG, "temp:%d, humi:%d", wifi_ui->temp_value, wifi_ui->humi_value);
}

// 主界面各值显示更新函数
void value_update_cb(lv_timer_t * timer)
{
    // 更新日期 星期 时分秒
    time(&now);
    localtime_r(&now, &timeinfo);
    lv_label_set_text_fmt(wifi_ui->led_time_label, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    lv_label_set_text_fmt(wifi_ui->date_label, "%d年%02d月%02d日", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday);
    lv_week_show();
    // get_indoor_th();

    // 日出日落时间交替显示 每隔5秒切换
    if (timeinfo.tm_sec%10 == 0) 
        lv_label_set_text_fmt(wifi_ui->sunset_label, "日落 %s", qwdaily_sunset);
    else if(timeinfo.tm_sec%10 == 5)
        lv_label_set_text_fmt(wifi_ui->sunset_label, "日出 %s", qwdaily_sunrise);

    // 更新温湿度 
    if(wifi_ui->th_update_flag == 1)
    {
        wifi_ui->th_update_flag = 0;
        lv_label_set_text_fmt(wifi_ui->indoor_temp_label, "%d℃", wifi_ui->temp_value);
        lv_label_set_text_fmt(wifi_ui->indoor_humi_label, "%d%%", wifi_ui->humi_value);
    }
    // 更新实时天气
    if(wifi_ui->qwnow_update_flag == 1)
    {
        wifi_ui->qwnow_update_flag = 0;
        lv_qweather_icon_show(); // 更新天气图标
        lv_label_set_text_fmt(wifi_ui->qweather_text_label, "%s", qwnow_text); // 更新天气情况文字描述
        lv_label_set_text_fmt(wifi_ui->outdoor_temp_label, "%d℃", qwnow_temp); // 更新室外温度
        lv_label_set_text_fmt(wifi_ui->outdoor_humi_label, "%d%%", qwnow_humi); // 更新室外湿度
        // get_indoor_th();
    }
    // 更新空气质量
    if(wifi_ui->qair_update_flag ==1)
    {
        wifi_ui->qair_update_flag = 0;
        lv_qair_level_show();
    }
    // 更新每日天气
    if(wifi_ui->qwdaily_update_flag == 1)
    {
        wifi_ui->qwdaily_update_flag = 0;
        lv_label_set_text_fmt(wifi_ui->qweather_temp_label, "%d~%d℃", qwdaily_tempMin, qwdaily_tempMax); // 温度范围
    }
}

// 主界面 任务函数, 是一个时钟, 用于在一定的时间刷新一次
static void main_page_task(void *pvParameters)
{
    example_wifi_shutdown();
    int tm_cnt1 = 0;
    int tm_cnt2 = 0;
    wifi_ui->th_update_flag = 0;
    wifi_ui->qwnow_update_flag = 0;
    wifi_ui->qair_update_flag = 0;
    wifi_ui->qwdaily_update_flag = 0;
    ESP_LOGI(TAG, "main_page_task start");
    lv_timer_create(value_update_cb, 1000, NULL);  // 创建一个lv_timer 每秒更新一次数据

    wifi_ui->reset_flag = 0; // 标记开机完成

    while (1)
    {
        tm_cnt1++;
        if (tm_cnt1 > 1800) // 30分钟更新一次实时天气和实时空气质量
        {
            tm_cnt1 = 0; // 计数清零
            example_wifi_reconnect();
            get_now_weather();  // 获取实时天气信息
            get_air_quality();  // 获取实时空气质量
            tm_cnt2++;
            if (tm_cnt2 > 1) // 60分钟更新一次每日天气
            {
                tm_cnt2 = 0;
                get_daily_weather(); // 获取每日天气信息
            }
            printf("weather update time:%02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            example_wifi_shutdown();
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));    
    }
    
    vTaskDelete(NULL);
}
/// 时钟界面显示函数
void lv_timer_page(lv_obj_t * obj)
{
    // 显示地理位置
    lv_obj_t * addr_label = lv_label_create(obj);
    lv_obj_set_style_text_font(addr_label, &beijing, 0);
    lv_label_set_text(addr_label, "北京市");
    lv_obj_align_to(addr_label, obj, LV_ALIGN_TOP_LEFT, 0, 0);

    // 显示年月日
    wifi_ui->date_label = lv_label_create(obj);
    lv_obj_set_style_text_font(wifi_ui->date_label, &beijing, 0);
    lv_label_set_text_fmt(wifi_ui->date_label, "%d年%02d月%02d日", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday);
    lv_obj_align_to(wifi_ui->date_label, obj, LV_ALIGN_TOP_RIGHT, 0, 0);

    // 显示分割线
    lv_obj_t * above_bar = lv_bar_create(obj);
    lv_obj_set_size(above_bar, 300, 3);
    lv_obj_set_pos(above_bar, 0 , 30);
    lv_bar_set_value(above_bar, 100, LV_ANIM_OFF);

    // 显示天气图标
    wifi_ui->qweather_icon_label = lv_label_create(obj);
    lv_obj_set_style_text_font(wifi_ui->qweather_icon_label, &font_qweather, 0);
    lv_obj_set_pos(wifi_ui->qweather_icon_label, 0 , 40);
    lv_qweather_icon_show();
    

    // 显示空气质量
    static lv_style_t qair_level_style;
    lv_style_init(&qair_level_style);
    lv_style_set_radius(&qair_level_style, 10);  // 设置圆角半径
    lv_style_set_bg_color(&qair_level_style, lv_palette_main(LV_PALETTE_GREEN)); // 绿色
    lv_style_set_text_color(&qair_level_style, lv_color_hex(0xffffff)); // 白色
    lv_style_set_border_width(&qair_level_style, 0);
    lv_style_set_pad_all(&qair_level_style, 0);
    lv_style_set_width(&qair_level_style, 50);  // 设置宽
    lv_style_set_height(&qair_level_style, 26); // 设置高

    wifi_ui->qair_level_obj = lv_obj_create(obj);
    lv_obj_add_style(wifi_ui->qair_level_obj, &qair_level_style, 0);
    lv_obj_align_to(wifi_ui->qair_level_obj, wifi_ui->qweather_icon_label, LV_ALIGN_OUT_RIGHT_TOP, 5, 0);

    wifi_ui->qair_level_label = lv_label_create(wifi_ui->qair_level_obj);
    lv_obj_set_style_text_font(wifi_ui->qair_level_label, &beijing, 0);
    lv_obj_align(wifi_ui->qair_level_label, LV_ALIGN_CENTER, 0, 0);
    lv_qair_level_show();

    // 显示当天室外温度范围
    wifi_ui->qweather_temp_label = lv_label_create(obj);
    lv_obj_set_style_text_font(wifi_ui->qweather_temp_label, &beijing, 0);
    lv_label_set_text_fmt(wifi_ui->qweather_temp_label, "%d~%d℃", qwdaily_tempMin, qwdaily_tempMax);
    lv_obj_align_to(wifi_ui->qweather_temp_label, wifi_ui->qweather_icon_label, LV_ALIGN_OUT_RIGHT_MID, 5, 5);

    // 显示当天天气图标代表的天气状况
    wifi_ui->qweather_text_label = lv_label_create(obj);
    lv_obj_set_style_text_font(wifi_ui->qweather_text_label, &beijing, 0);
    lv_label_set_long_mode(wifi_ui->qweather_text_label, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_obj_set_width(wifi_ui->qweather_text_label, 80);
    lv_label_set_text_fmt(wifi_ui->qweather_text_label, "%s", qwnow_text);
    lv_obj_align_to(wifi_ui->qweather_text_label, wifi_ui->qweather_icon_label, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, 0);
    
    // 显示时间  小时:分钟:秒钟
    wifi_ui->led_time_label = lv_label_create(obj);
    lv_obj_set_style_text_font(wifi_ui->led_time_label, &font_led, 0);
    lv_label_set_text_fmt(wifi_ui->led_time_label, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    lv_obj_set_pos(wifi_ui->led_time_label, 142, 42);

    // 显示星期几
    wifi_ui->week_label = lv_label_create(obj);
    lv_obj_set_style_text_font(wifi_ui->week_label, &beijing, 0);
    lv_obj_align_to(wifi_ui->week_label, wifi_ui->led_time_label, LV_ALIGN_OUT_BOTTOM_RIGHT, -10, 6);
    lv_week_show();

    // 显示日落时间 
    wifi_ui->sunset_label = lv_label_create(obj);
    lv_obj_set_style_text_font(wifi_ui->sunset_label, &beijing, 0);
    lv_label_set_text_fmt(wifi_ui->sunset_label, "日落 %s", qwdaily_sunset);
    lv_obj_set_pos(wifi_ui->sunset_label, 200 , 103);

    // 显示分割线
    lv_obj_t * below_bar = lv_bar_create(obj);
    lv_obj_set_size(below_bar, 300, 3);
    lv_obj_set_pos(below_bar, 0, 130);
    lv_bar_set_value(below_bar, 100, LV_ANIM_OFF);

    // 显示室外温湿度
    static lv_style_t outdoor_style;
    lv_style_init(&outdoor_style);
    lv_style_set_radius(&outdoor_style, 10);  // 设置圆角半径
    lv_style_set_bg_color(&outdoor_style, lv_color_hex(0xd8b010));
    lv_style_set_text_color(&outdoor_style, lv_color_hex(0x00000000)); // 白色
    lv_style_set_border_width(&outdoor_style, 0);
    lv_style_set_pad_all(&outdoor_style, 5);
    lv_style_set_width(&outdoor_style, 100);  // 设置宽
    lv_style_set_height(&outdoor_style, 80); // 设置高

    lv_obj_t * outdoor_obj = lv_obj_create(obj);
    lv_obj_add_style(outdoor_obj, &outdoor_style, 0);
    lv_obj_align(outdoor_obj, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t *outdoor_th_label = lv_label_create(outdoor_obj);
    lv_obj_set_style_text_font(outdoor_th_label, &beijing, 0);
    lv_label_set_text(outdoor_th_label, "室外");
    lv_obj_align(outdoor_th_label, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *temp_symbol_label1 = lv_label_create(outdoor_obj);
    lv_obj_set_style_text_font(temp_symbol_label1, &font_myawesome, 0);
    lv_label_set_text(temp_symbol_label1, "\xEF\x8B\x88");  // 显示温度图标
    lv_obj_align(temp_symbol_label1, LV_ALIGN_LEFT_MID, 10, 0);

    wifi_ui->outdoor_temp_label = lv_label_create(outdoor_obj);
    lv_obj_set_style_text_font(wifi_ui->outdoor_temp_label, &beijing, 0);
    lv_label_set_text_fmt(wifi_ui->outdoor_temp_label, "%d℃", qwnow_temp);
    lv_obj_align_to(wifi_ui->outdoor_temp_label, temp_symbol_label1, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t *humi_symbol_label1 = lv_label_create(outdoor_obj);
    lv_obj_set_style_text_font(humi_symbol_label1, &font_myawesome, 0);
    lv_label_set_text(humi_symbol_label1, "\xEF\x81\x83");  // 显示湿度图标
    lv_obj_align(humi_symbol_label1, LV_ALIGN_BOTTOM_LEFT, 10, 0);

    wifi_ui->outdoor_humi_label = lv_label_create(outdoor_obj);
    lv_obj_set_style_text_font(wifi_ui->outdoor_humi_label, &beijing, 0);
    lv_label_set_text_fmt(wifi_ui->outdoor_humi_label, "%d%%", qwnow_humi);
    lv_obj_align_to(wifi_ui->outdoor_humi_label, humi_symbol_label1, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 显示室内温湿度
    static lv_style_t indoor_style;
    lv_style_init(&indoor_style);
    lv_style_set_radius(&indoor_style, 10);  // 设置圆角半径
    lv_style_set_bg_color(&indoor_style, lv_color_hex(0xfe6464)); //
    lv_style_set_text_color(&indoor_style, lv_color_hex(0x00000000)); // 白色
    lv_style_set_border_width(&indoor_style, 0);
    lv_style_set_pad_all(&indoor_style, 5);
    lv_style_set_width(&indoor_style, 100);  // 设置宽
    lv_style_set_height(&indoor_style, 80); // 设置高

    lv_obj_t * indoor_obj = lv_obj_create(obj);
    lv_obj_add_style(indoor_obj, &indoor_style, 0);
    lv_obj_align(indoor_obj, LV_ALIGN_BOTTOM_MID, 10, 0);

    lv_obj_t *indoor_th_label = lv_label_create(indoor_obj);
    lv_obj_set_style_text_font(indoor_th_label, &beijing, 0);
    lv_label_set_text(indoor_th_label, "室内");
    lv_obj_align(indoor_th_label, LV_ALIGN_TOP_MID, 0, 0);


    lv_obj_t *temp_symbol_label2 = lv_label_create(indoor_obj);
    lv_obj_set_style_text_font(temp_symbol_label2, &font_myawesome, 0);
    lv_label_set_text(temp_symbol_label2, "\xEF\x8B\x88");  // 温度图标
    lv_obj_align(temp_symbol_label2, LV_ALIGN_LEFT_MID, 10, 0);

    wifi_ui->indoor_temp_label = lv_label_create(indoor_obj);
    lv_obj_set_style_text_font(wifi_ui->indoor_temp_label, &beijing, 0);
    lv_label_set_text_fmt(wifi_ui->indoor_temp_label, "%d℃", wifi_ui->temp_value);
    lv_obj_align_to(wifi_ui->indoor_temp_label, temp_symbol_label2, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t *humi_symbol_label2 = lv_label_create(indoor_obj);
    lv_obj_set_style_text_font(humi_symbol_label2, &font_myawesome, 0);
    lv_label_set_text(humi_symbol_label2, "\xEF\x81\x83");  // 湿度图标
    lv_obj_align(humi_symbol_label2, LV_ALIGN_BOTTOM_LEFT, 10, 0);

    wifi_ui->indoor_humi_label = lv_label_create(indoor_obj);
    lv_obj_set_style_text_font(wifi_ui->indoor_humi_label, &beijing, 0);
    lv_label_set_text_fmt(wifi_ui->indoor_humi_label, "%d%%", wifi_ui->humi_value);
    lv_obj_align_to(wifi_ui->indoor_humi_label, humi_symbol_label2, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 显示太空人
    lv_obj_t *tk_gif = lv_gif_create(obj);
    lv_gif_set_src(tk_gif, &image_taikong);
    lv_obj_align(tk_gif, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}

/// @brief 读按钮回调函数
/// @param e 
void text_context_windows_read_cb(lv_event_t * e)
{
    if(text_context_windows)
        device_write(DEVICE_TTV_NUM, (void *)lv_textarea_get_text(text_context_windows), 0, 0);
}

/// @brief 建立一个文本框
/// @param base 页面
/// @param text_next_page 下一页回调函数 
/// @param text_pre_page   上一页回调函数
/// @param user_data 用户数据
/// @return 创建的文本框
lv_obj_t * create_text(lv_obj_t * base, 
        void(*text_next_page)(lv_event_t * e), void (*text_pre_page)(lv_event_t * e),
        void *user_data)
{
#if USING_CHINESE
    LV_FONT_DECLARE(font_alipuhui20);
#endif
    lv_obj_add_style(base, &main_style, 0);
    lv_obj_t *text=lv_textarea_create(base);
#if USING_CHINESE
    lv_obj_set_style_text_font(text, &font_alipuhui20, 0);
#endif
    lv_obj_set_size(text,260,165);
    lv_obj_align(text,LV_ALIGN_OUT_TOP_LEFT,21,21);
    //按钮上
    lv_obj_t *up_btn =lv_btn_create(base);
    lv_obj_t *up=lv_label_create(up_btn);
    lv_label_set_text(up, LV_SYMBOL_UP); 
    lv_obj_set_align(up_btn,LV_ALIGN_BOTTOM_LEFT);
    lv_obj_set_style_text_color(up_btn,lv_color_hex(0x000000),LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(up_btn,lv_color_hex(0x7c7c7c),LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(up_btn, 0, LV_PART_MAIN);
    lv_obj_set_align(up, LV_ALIGN_CENTER);
    lv_obj_set_size(up_btn,30,30);
    if(text_pre_page!=NULL)
    {
        lv_obj_add_event_cb(up_btn,text_pre_page,LV_EVENT_CLICKED,user_data);
    }
    //按钮下
    lv_obj_t *down_btn =lv_btn_create(base);
    lv_obj_t *down=lv_label_create(down_btn);
    lv_label_set_text(down, LV_SYMBOL_DOWN); 
    lv_obj_set_align(down_btn,LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_set_style_text_color(down_btn,lv_color_hex(0x000000),LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(down_btn,lv_color_hex(0x7c7c7c),LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(down_btn, 0, LV_PART_MAIN);
    lv_obj_set_align(down, LV_ALIGN_CENTER);
    lv_obj_set_size(down_btn,30,30);
    if(text_next_page!=NULL)
    {
        lv_obj_add_event_cb(down_btn,text_next_page,LV_EVENT_CLICKED,user_data);
    }
    //按钮播放
    lv_obj_t *volume_btn =lv_btn_create(base);
    lv_obj_t *volume=lv_label_create(volume_btn);
    lv_label_set_text(volume, LV_SYMBOL_VOLUME_MAX); 
    lv_obj_set_align(volume_btn,LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_text_color(volume_btn,lv_color_hex(0x000000),LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(volume_btn,lv_color_hex(0x7c7c7c),LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(volume_btn, 0, LV_PART_MAIN);
    lv_obj_set_align(volume, LV_ALIGN_CENTER);
    lv_obj_set_size(volume_btn,30,30);
    lv_obj_add_event_cb(volume_btn,text_context_windows_read_cb,LV_EVENT_CLICKED,NULL);
    return text;
}






/*************************
 * 主界面
 * ***********************/ 
/// @brief 主页面的显示
/// @param data 
void weather_main_page(void * data)
{
    // 获取一下页面结构体
    wifi_ui = (lv_wifi_t *)malloc(sizeof(lv_wifi_t));
    if(wifi_ui == NULL){
        ESP_LOGE(TAG, "malloc wifi_ui failed");
        return;
    }
    lv_obj_clean(lv_scr_act()); // 清空屏幕
    
    // 获取当前的信息
    memset(wifi_ui, 0, sizeof(lv_wifi_t));
    vTaskDelay(10 / portTICK_PERIOD_MS);
    get_air_quality();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    get_now_weather();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    get_daily_weather();
    
    ESP_LOGI(TAG, "weather_main_page get weather data");
    xTaskCreate(main_page_task, "main_page_task", 8192, NULL, 5, NULL);         // 非一次性任务 主界面任务
    ESP_LOGI(TAG, "weather_main_page get main page");
    lvgl_port_lock(0);
    ESP_LOGI(TAG, "weather_main_page");
    wifi_ui->tileview = lv_tileview_create(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0); // 修改背景为黑色

    /*Create an object with the new style*/
    lv_obj_t * obj = lv_tileview_add_tile(wifi_ui->tileview, 1, 1, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP);
    lv_obj_t * obj2 = lv_tileview_add_tile(wifi_ui->tileview, 0, 1, LV_DIR_RIGHT);
    lv_obj_t * obj3 = lv_tileview_add_tile(wifi_ui->tileview, 1, 0, LV_DIR_BOTTOM);
    lv_obj_t * obj4 = lv_tileview_add_tile(wifi_ui->tileview, 2, 1, LV_DIR_LEFT);
    lv_obj_t * obj5 = lv_tileview_add_tile(wifi_ui->tileview, 3, 1, LV_DIR_NONE);
    // my_list(obj2); //初始化一下左侧的列表

    // 设置初始位置
    lv_obj_remove_style(wifi_ui->tileview, NULL, LV_PART_SCROLLBAR);

    lv_obj_add_style(obj, &main_style, 0);
    lv_obj_add_style(obj2, &main_style, 0);
    lv_obj_add_style(obj3, &main_style, 0);
    lv_obj_add_style(obj4, &main_style, 0);
    lv_obj_add_style(obj5, &main_style, 0);
    lvgl_port_unlock();
    // get_http_data_story();

    ESP_LOGI(TAG, "weather_main_page1");
    lvgl_port_lock(0);
    lv_timer_page(obj);
    lvgl_port_unlock();

    ESP_LOGI(TAG, "weather_main_page2");
    lvgl_port_lock(0);
    lv_set_page(obj2);
    lvgl_port_unlock();


    ESP_LOGI(TAG, "weather_main_page3");
    lvgl_port_lock(0);
    lv_dir_page(obj3);
    lvgl_port_unlock();

    ESP_LOGI(TAG, "weather_main_page4");
    lvgl_port_lock(0);
    lv_app_page(obj4, obj5);
    lv_obj_set_tile_id(wifi_ui->tileview, 1, 1, LV_ANIM_OFF);
    lvgl_port_unlock();

    ESP_LOGI(TAG, "weather finish it");
}

/// @brief 主界面任务deinit
void wifi_main_page_deinit(void){
    lvgl_port_lock(0);
    lv_obj_del(wifi_ui->tileview);
    lvgl_port_unlock();
    free(wifi_ui);
    wifi_ui = NULL;
}
// 把一个数字的字符转换为字符串
char *one_num_to_str(int num){
    switch (num)
    {
        case 1:
            return "一";
        case 2:
            return "二";
        case 3: 
            return "三";
        case 4:
            return "四";
        case 5:
            return "五";
        case 6:
            return "六";
        case 7:
            return "七";
        case 8:
            return "八";
        case 9:
            return "九";
        case 0:
            return "零";
        default:
            return "未知";
    }
}
/// @brief 把数字转换为中文字符串
/// @param num 要转换的数字
/// @param str 数字转换后的字符串
/// @return 转换的字符串
char* num_to_str(int num, char *str){
    if(str== NULL){
        return NULL;
    }
    if(num < 0){
        sprintf(str, "负%s", one_num_to_str(0-num));
    }
    if(num < 10){
        sprintf(str, "%s", one_num_to_str(num));
    }else if(num < 20){
        sprintf(str, "十%s", one_num_to_str(num%10));
    }else if(num < 100){
        sprintf(str, "%s十%s", one_num_to_str(num/10), one_num_to_str(num%10));
    }
    return str;
}


/// @brief 不同的空气质量的描述
const char *air_quality_str[] = {
    "优 空气质量很好 可以正常活动",
    "良 空气质量一般 可以正常活动",
    "轻度污染 空气质量较差 可能对敏感人群有影响 建议减少外出",
    "中度污染 空气质量较差 可能对敏感人群有影响 建议减少外出",
    "重度污染 空气质量很差 对所有人都有影响 建议不要外出",
    "严重污染 空气质量很差 对所有人都有影响 建议不要外出"
};
/// @brief 读时间
void timer_read_timer(){
    time(&now);
    localtime_r(&now, &timeinfo);
    printf("time:%02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    // 把时间转换为中文字符串
    char time_str[200];
    char month[10];
    char day[10];
    char hour[10];
    char min[10];
    char sec[10];
    sprintf(time_str, "%d年%s月%s日 %s点%s分%s秒", timeinfo.tm_year+1900, 
        num_to_str(timeinfo.tm_mon+1, month), num_to_str(timeinfo.tm_mday, day), 
        num_to_str(timeinfo.tm_hour, hour), num_to_str(timeinfo.tm_min, min), 
        num_to_str(timeinfo.tm_sec, sec));
    // sprintf(time_str, "二零二四年十二月三十一日 二十三点五十九分五十九秒");
    // char *test = "二零二四年十二月三十一日 二十三点五十九分五十九秒";
    device_write(DEVICE_TTV_NUM, time_str, 0, 0);
    ttv_wait_to_end();
}

/// @brief 读天气
void timer_read_weather(){
    char weather_str[200];
    char temp[10];
    char wet[10];
    const char *air_str;
    if(0 < qanow_level && qanow_level < 7){
        air_str = air_quality_str[qanow_level - 1];
    }else{
        air_str = "未知";
    }
    sprintf(weather_str, "当前温度是%s摄氏度 湿度百分之%s 空气质量是%s 天气为%s", 
        num_to_str(qwnow_temp, temp), num_to_str(qwnow_humi, wet), air_str, qwnow_text);
    device_write(DEVICE_TTV_NUM, weather_str, strlen(weather_str), 0);
    ttv_wait_to_end();
}

