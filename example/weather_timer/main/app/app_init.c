#include "lvgl.h"
#include "app_init.h"
#include "app_ui.h"
#include "esp_lvgl_port.h"

QueueHandle_t get_mode;
int init_mode = 0;

int init_get_mode(){
    return init_mode;
}

void chice_event_cb(lv_event_t *e){
    int data = (int)lv_event_get_user_data(e);
    init_mode = data;
    ESP_LOGI("init_mode", "init_mode: %d", init_mode);
    xSemaphoreGive(get_mode);
}
extern lv_style_t main_style;

/// @brief 初始化界面的页面, 用于显示模式
/// @param  
void init_ui(void){
    get_mode = xQueueCreateCountingSemaphore(1, 0);

#if USING_CHINESE
    LV_FONT_DECLARE(font_alipuhui20);
#endif
    lvgl_port_lock(0);
    lv_obj_set_style_bg_color(lv_scr_act(),lv_color_hex(0xCCEEFF),0);
    lv_obj_t *label=lv_label_create(lv_scr_act());
#if USING_CHINESE
    lv_label_set_text(label,"请选择您要进入的模式");
    lv_obj_set_style_text_font(label,&font_alipuhui20,LV_STATE_DEFAULT);
#else
    lv_label_set_text(label,"Please select the mode you want to enter");
#endif
    lv_obj_align(label,LV_ALIGN_TOP_MID,0,10);
    static lv_style_t style_obj;
    lv_style_init(&style_obj);
    lv_style_set_bg_color(&style_obj, lv_color_hex(0x77DDFF));
    lv_style_set_bg_opa(&style_obj,100);
    lv_style_set_text_color(&style_obj, lv_color_black());
    lv_style_set_border_width(&style_obj, 1);
    lv_style_set_border_color(&style_obj,lv_color_black());
    lv_style_set_pad_all(&style_obj, 0);
    lv_style_set_radius(&style_obj, 10);
    //lv_style_set_size(style_obj,140,200);

    //离线模式
    lv_obj_t *obj_off_line_mode=lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj_off_line_mode,&style_obj,0);
    lv_obj_set_size(obj_off_line_mode,130,180);
    lv_obj_align(obj_off_line_mode,LV_ALIGN_LEFT_MID,25,13);

    lv_obj_t *btn_off_line_mode = lv_btn_create(obj_off_line_mode);
    lv_obj_align(btn_off_line_mode, LV_ALIGN_TOP_MID, 0, 12); // 位置
    lv_obj_set_height(btn_off_line_mode, 30); // 宽度
    lv_obj_set_style_bg_opa(btn_off_line_mode,150,0);
    lv_obj_add_event_cb(btn_off_line_mode, chice_event_cb, LV_EVENT_CLICKED, 0); // 事件处理函数

    lv_obj_t *label_off_line_mode = lv_label_create(btn_off_line_mode);
#if USING_CHINESE
    lv_label_set_text(label_off_line_mode, "离线模式");
    lv_obj_set_style_text_font(label_off_line_mode, &font_alipuhui20, 0);
#else
    lv_label_set_text(label_off_line_mode, "Offline mode");
#endif
    lv_obj_center(label_off_line_mode);

    lv_obj_t *label_off_line = lv_label_create(obj_off_line_mode);
    lv_obj_set_height(label_off_line, 120);
    lv_label_set_long_mode(label_off_line, LV_LABEL_LONG_SCROLL_CIRCULAR);
#if USING_CHINESE
    lv_label_set_text(label_off_line, "音乐播放\n文本阅读\n小游戏\n照相机\n");
    lv_obj_set_style_text_font(label_off_line, &font_alipuhui20, 0);
#else
    lv_label_set_text(label_off_line, "Music\nplayback\nText\nreading\nSmall\ngame\ncamera\n");
#endif
    lv_obj_align(label_off_line,LV_ALIGN_BOTTOM_MID,0,-13);

    //在线模式
    lv_obj_t *obj_on_line_mode=lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj_on_line_mode,&style_obj,0);
    lv_obj_set_size(obj_on_line_mode,130,180);
    lv_obj_align(obj_on_line_mode,LV_ALIGN_RIGHT_MID,-25,13);

    lv_obj_t *btn_on_line_mode = lv_btn_create(obj_on_line_mode);
    lv_obj_align(btn_on_line_mode, LV_ALIGN_TOP_MID, 0, 12); // 位置
    lv_obj_set_height(btn_on_line_mode, 30); // 宽度
    lv_obj_set_style_bg_opa(btn_on_line_mode,150,0);
    lv_obj_add_event_cb(btn_on_line_mode, chice_event_cb, LV_EVENT_CLICKED, (void *)1); // 事件处理函数

    lv_obj_t *label_on_line_mode = lv_label_create(btn_on_line_mode);
#if USING_CHINESE
    lv_label_set_text(label_on_line_mode, "在线模式");
    lv_obj_set_style_text_font(label_on_line_mode, &font_alipuhui20, 0);
#else
    lv_label_set_text(label_on_line_mode, "Online mode");
#endif
    lv_obj_center(label_on_line_mode);

    lv_obj_t *label_on_line = lv_label_create(obj_on_line_mode);
    lv_obj_set_height(label_on_line, 120);
    lv_label_set_long_mode(label_on_line, LV_LABEL_LONG_SCROLL_CIRCULAR);
#if USING_CHINESE
    lv_label_set_text(label_on_line, "天气时钟\n文本阅读\n小游戏\n星座运气\n故事播放\n猜谜\n毒鸡汤");
    lv_obj_set_style_text_font(label_on_line, &font_alipuhui20, 0);
#else
    lv_label_set_text(label_on_line, "Weather\nclock\nText\nreading\nSmall\ngame\nConstellation\nluck\nStory\nplayback\nGuess\nPoisonous\nchicken\nsoup");
#endif
    lv_obj_align(label_on_line,LV_ALIGN_BOTTOM_MID,0,-13);

    lvgl_port_unlock();

    // 等待获取到
    xQueueSemaphoreTake(get_mode, portMAX_DELAY);
    vSemaphoreDelete(get_mode);
    lvgl_port_lock(0);
    lv_obj_clean(lv_scr_act());
    lvgl_port_unlock();
}