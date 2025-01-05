/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-09 11:41:07
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-12-03 23:41:55
 */
#pragma once
#include "stdint.h"
#include "lvgl.h"
uint8_t music_get_now_volume(void);
void music_set_now_volume(uint8_t volume);
void music_ui(lv_obj_t *base);
void music_main_page(void);
void mp3_player_init(void *data);
void ai_gui_in(void);
void ai_gui_out(void);

void ai_play(void);
void ai_pause(void);
void ai_resume(void);
void ai_prev_music(void);
void ai_next_music(void);
void ai_volume_up(void);
void ai_volume_down(void);

#define MUSIC_UI_REGISTER_CODE   {                                                       \
                                    ui_config_t *music_ui = malloc(sizeof(ui_config_t)); \
                                    memset(music_ui, 0, sizeof(ui_config_t));            \
                                    music_ui->init = mp3_player_init;                   \
                                    music_ui->deinit = NULL;                            \
                                    music_ui->ID = UI_ID_MUSIC;                           \
                                    app_ui_add(music_ui);                                \
                                }while(0);



