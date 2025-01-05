#ifndef APP_DIR_H
#define APP_DIR_H
#include "lvgl.h"
#include "stdio.h"

lv_obj_t * create_text(lv_obj_t * base, 
        void(*text_next_page)(lv_event_t * e), void (*text_pre_page)(lv_event_t * e),
        void *user_data);

typedef struct{
    int file_idx;
    lv_obj_t *base;
}lv_dir_data_t;

/// @brief 数据结构文件页面返回使用的数据结构
typedef struct{
    FILE *fp;
    lv_obj_t *base;
}lv_dile_back_data_t;
void lv_dir_page(lv_obj_t *obj3);
#endif 