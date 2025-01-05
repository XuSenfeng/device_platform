/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-30 23:04:12
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-03 22:18:58
 */
#ifndef __2048_H
#define __2048_H
 

#include "math.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
 
#define LIMIT1 3000
#define LIMIT2 6000
#define LIMIT3 10000
#define BLOCKH 40
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define WIDTH 4
#define NEW_NUM 1
 
void event_cb(lv_event_t* e);
void show_color_block(uint8_t n,uint16_t px,uint16_t py,lv_obj_t* bg,uint8_t i,uint8_t j);
void show_mb(lv_obj_t* bg);
void bg_hd(uint8_t toward,lv_obj_t* bg);
void rands(int n, uint16_t s);
void game_init(lv_obj_t* bg);
_Bool detect_end();
void game_main(lv_obj_t * base);
void end_ami();
 
#endif