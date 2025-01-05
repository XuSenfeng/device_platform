#ifndef APP_SET_H
#define APP_SET_H

#include "user_config.h"
void lv_set_page(lv_obj_t *base);
typedef void (*page_handle_cb_t)(lv_obj_t * base) ;
typedef struct set_item
{
    lv_obj_t *btn;
#if USING_CHINESE
    char *name_ch;
#else
    char *name_en;
#endif 
    char *item_log;
    page_handle_cb_t page_handle_cb;  // 处理在设置页面选项的控制函数, 用于初始化不同的设置界面
    struct set_item *next;
}set_item_t;
 
esp_err_t set_item_add(char *name, page_handle_cb_t handle, lv_obj_t * setting, lv_obj_t *base, 
                        char * item_log);
page_handle_cb_t app_item_get_handler(lv_obj_t *obj);
void app_item_free();
#endif
