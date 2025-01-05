/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-17 09:16:07
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-04 13:14:29
 */
#ifndef APP_UI_H
#define APP_UI_H

#include "esp_err.h"
#include "stdint.h"
#include "app_API.h"
#include "app_wifi.h"
#include "app_timer.h"
#include "app_weather_ui.h"
#include "app_sr.h"
#include "app_music.h"
#include "app_dir.h"
#include "app_game.h"
#include "app_2048.h"
#include "app_set.h"
#include "app_init.h"

#define UI_SELECTED     1

#define UI_ID_TIMER         0x1
#define UI_ID_MUSIC         0x2
#define UI_ID_WIFI          0x4


#define APP_TIMER_DEVICE ( DEVICE_TTV_NUM | DEVIDE_SD_CARD_NUM)
#define APP_MUSIC_DEVICE ( DEVICE_TTV_NUM | DEVIDE_SD_CARD_NUM)

#define APP_MUSIC           UI_ID_MUSIC
#define APP_WEATHER         (UI_ID_TIMER | UI_ID_WIFI)

#define APP_REGISTER   APP_WEATHER

typedef struct _ui_config_t{
    uint32_t ID;
    void (*init)(void *);
    void (*deinit)(void);
    struct _ui_config_t *next_l;
    struct _ui_config_t *next_r;
    struct _ui_config_t *next_u;
    struct _ui_config_t *next_d;
    struct _ui_config_t *next;
    void *user_data;
}ui_config_t;

typedef struct {
    ui_config_t *current;
    ui_config_t *head;
} ui_contorl_t;

void app_ui_init(int register_ui);
esp_err_t app_ui_add_pos(ui_config_t *ui, ui_config_t *now_pos_ui, char pos);
esp_err_t app_ui_show_pos(char pos);
esp_err_t app_ui_show_id(uint32_t id);
esp_err_t app_ui_add(ui_config_t *ui);

#endif // APP_UI_H