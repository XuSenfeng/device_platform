#include "app_ui.h"
#include "esp_system.h"
#include "device_bsp.h"
#include "device_manage.h"
#include "esp_lvgl_port.h"

uint8_t page_read_flag = 1;
static lv_obj_t *roller_num_sec;
static lv_obj_t *roller_num_min;
static lv_obj_t *slider_light;
static lv_obj_t *slider_sound;
static lv_obj_t *slider_label;

lv_obj_t *label_page_read;
lv_obj_t *page_read_btn;

/*****颜色设置界面****/
static lv_obj_t *R_slider;
static lv_obj_t *G_slider;
static lv_obj_t *B_slider;
static lv_obj_t *T_slider;
static lv_obj_t *R_slider_label;
static lv_obj_t *G_slider_label;
static lv_obj_t *B_slider_label;
static lv_obj_t *T_slider_label;
uint32_t defaule_color = DEFAULT_LC_COLOR;
uint32_t defaule_opa = LV_OPA_50;

static const char *TAG = "set";

/// @brief 设置锁屏时间的回调函数
/// @param e 
void set_sleep_time_btn_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        int min = lv_roller_get_selected(roller_num_min);
        int sec = lv_roller_get_selected(roller_num_sec);
        int time = min * 60 + sec;
        printf("time:%d\n", time);
        bsp_display_brightness_set_defalut_time(time);
    }
}

/// @brief 建立锁屏时间界面
/// @param parent 
static void create_lock_time_page(lv_obj_t* parent)
{
    // 创建roller样式
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_white());
    lv_style_set_text_color(&style, lv_color_black());
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_style_set_radius(&style, 0);
    // 创建"数字"roller
    const char * opts_num = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n";

    roller_num_min = lv_roller_create(parent);
    lv_obj_add_style(roller_num_min, &style, 0);
    lv_obj_set_style_bg_opa(roller_num_min, LV_OPA_50, LV_PART_SELECTED);

    lv_roller_set_options(roller_num_min, opts_num, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_num_min, 3); // 显示3行
    lv_roller_set_selected(roller_num_min, 0, LV_ANIM_OFF); // 默认选择
    lv_obj_set_width(roller_num_min, 90);
    lv_obj_set_style_text_font(roller_num_min, &lv_font_montserrat_20, 0);
    lv_obj_align(roller_num_min, LV_ALIGN_BOTTOM_LEFT, 35, -58);
    // lv_obj_add_event_cb(roller_num, mask_event_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    roller_num_sec = lv_roller_create(parent);
    lv_obj_add_style(roller_num_sec, &style, 0);
    lv_obj_set_style_bg_opa(roller_num_sec, LV_OPA_50, LV_PART_SELECTED);

    lv_roller_set_options(roller_num_sec, opts_num, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_num_sec, 3); // 显示3行
    lv_roller_set_selected(roller_num_sec, 1, LV_ANIM_OFF); // 默认选择
    lv_obj_set_width(roller_num_sec, 90);
    lv_obj_set_style_text_font(roller_num_sec, &lv_font_montserrat_20, 0);
    lv_obj_align(roller_num_sec, LV_ALIGN_BOTTOM_RIGHT, -35, -58);

    lv_obj_t *label_colon = lv_label_create(parent);
    lv_label_set_text(label_colon, ":");
    lv_obj_set_style_text_font(label_colon, &lv_font_montserrat_20, 0);
    lv_obj_align(label_colon, LV_ALIGN_BOTTOM_MID, 0, -95);

    lv_obj_t *label_min = lv_label_create(parent);
    lv_label_set_text(label_min, "min");
    lv_obj_set_style_text_font(label_min, &lv_font_montserrat_20, 0);
    lv_obj_align(label_min, LV_ALIGN_TOP_LEFT, 56, 30);

    lv_obj_t *label_sec = lv_label_create(parent);
    lv_label_set_text(label_sec, "sec");
    lv_obj_set_style_text_font(label_sec, &lv_font_montserrat_20, 0);
    lv_obj_align(label_sec, LV_ALIGN_TOP_RIGHT, -64, 30);

    // 创建的确认键
    lv_obj_t *btn_num_ok = lv_btn_create(parent);
    lv_obj_align(btn_num_ok, LV_ALIGN_BOTTOM_MID, 0, -10); // 位置
    lv_obj_set_width(btn_num_ok, 90); // 宽度
    lv_obj_add_event_cb(btn_num_ok, set_sleep_time_btn_cb, LV_EVENT_CLICKED, NULL); // 事件处理函数

    lv_obj_t *label_num_ok = lv_label_create(btn_num_ok);
    lv_label_set_text(label_num_ok, LV_SYMBOL_OK);
    lv_obj_set_style_text_font(label_num_ok, &lv_font_montserrat_20, 0);
    lv_obj_center(label_num_ok);

}


void set_slider_set_light(lv_obj_t *target, lv_event_t *e){
    char buf[20], value_str[15];
    int32_t value = lv_slider_get_value(target);
    if(lv_event_get_code(e) == LV_EVENT_RELEASED)
    {
        page_read(num_to_str(value, value_str));
        device_write(DEVICE_TTV_NUM, num_to_str(value, value_str), 0, 0);
    }
    sprintf(buf, "Brightness: %ld", value);
    lvgl_port_lock(0);
    lv_label_set_text(slider_label,buf);
    lvgl_port_unlock();
    bsp_display_brightness_set_defalut_light(value);
}

/// @brief 设置当前的音量
/// @param target 
void set_slider_set_sound(lv_obj_t *target, lv_event_t *e){
    char buf[20], value_str[15];
    int32_t value = lv_slider_get_value(target);
    if(lv_event_get_code(e) == LV_EVENT_RELEASED)
    {
        page_read(num_to_str(value, value_str));
        device_write(DEVICE_TTV_NUM, num_to_str(value, value_str), 0, 0);
    }
    sprintf(buf, "Sonnd: %ld", value);
    lvgl_port_lock(0);
    lv_label_set_text(slider_label,buf);
    lvgl_port_unlock();
    music_set_now_volume(value); // 把声音赋值给g_sys_volume保存
    bsp_codec_volume_set(value, NULL);
}

/// @brief 滚轮的回调函数
/// @param e 
static void event_cb_slider(lv_event_t * e){
    lv_obj_t *target=lv_event_get_target(e);
    
    if(target==slider_light){
        set_slider_set_light(target, e);
    }
    else if (target==slider_sound)
    {
        set_slider_set_sound(target, e);
    }
}

/// @brief 建立一个0-100的滚轮
/// @param base 
/// @return 
static lv_obj_t * create_slider(lv_obj_t * base)
{
    lv_obj_t *slider=lv_slider_create(base);
    lv_obj_center(slider);
    lv_obj_set_size(slider,200,10);
    lv_slider_set_range(slider,0,100);
    lv_slider_set_value(slider,50,LV_ANIM_ON);
    lv_obj_set_style_bg_opa(slider,LV_OPA_COVER,0);
    lv_obj_set_style_bg_color(slider,lv_color_hex(0xb3b3b3),LV_PART_MAIN);
    lv_obj_add_event_cb(slider,event_cb_slider,LV_EVENT_VALUE_CHANGED,base);

    slider_label = lv_label_create(base);
    lv_label_set_text(slider_label,"\0");
    lv_obj_set_style_text_color(slider_label,lv_color_hex(0xffffff),LV_STATE_DEFAULT);
    lv_obj_align(slider_label, LV_ALIGN_TOP_MID, 0, 40);

    // lv_obj_add_state(slider, LV_STATE_CHECKED);
    return slider;

}

/// @brief 界面朗读的按钮回调函数
/// @param e 
static void btn_event_cb_page_read(lv_event_t * e){
    if(strcmp(lv_label_get_text(label_page_read), "Page_Read")==0){
        char data[] = "翻页阅读关";
        if(page_read_flag)device_write(DEVICE_TTV_NUM, data, strlen(data), 0);
        lvgl_port_lock(0);
        lv_obj_set_style_bg_color(page_read_btn,lv_color_hex(0xb5b5b5),LV_STATE_DEFAULT);
        lv_label_set_text(label_page_read,"Page_Read_OFF");
        page_read("翻页阅读关");
        page_read_flag = 0;           
        lvgl_port_unlock();
    }else{
        char data[] = "翻页阅读开";
        if(page_read_flag)device_write(DEVICE_TTV_NUM, data, strlen(data), 0);
        lvgl_port_lock(0);
        lv_obj_set_style_bg_color(page_read_btn,lv_color_hex(0x2525c5),LV_STATE_DEFAULT);
        lv_label_set_text(label_page_read,"Page_Read");
        page_read_flag = 1;
        lvgl_port_unlock();
        page_read("翻页阅读开");
    }
}

/// @brief 界面朗读的回调函数
/// @param base 
static void create_page_read_btn(lv_obj_t * base)
{
    page_read_btn=lv_btn_create(base);
    lv_obj_center(page_read_btn);
    lv_obj_set_size(page_read_btn,150,60);
    lv_obj_set_style_bg_color(page_read_btn,lv_color_hex(0x2525b5),LV_STATE_DEFAULT);
    label_page_read=lv_label_create(page_read_btn);
    lv_label_set_text(label_page_read,"Page_Read");
    lv_obj_center(label_page_read);
    lv_obj_add_event_cb(page_read_btn,btn_event_cb_page_read,LV_EVENT_CLICKED,base);
    // lv_obj_add_state(page_read_btn, LV_STATE_CHECKED);
}

/// @brief 设置背景颜色的回调函数
/// @param e 
void color_cb(lv_event_t *e){
    lv_obj_t *target=lv_event_get_target(e);
    lvgl_port_lock(0);
    if(target==R_slider)
    {
        lv_label_set_text_fmt(R_slider_label,"%ld",lv_slider_get_value(target));
    }
    else if(target==G_slider)
    {
        lv_label_set_text_fmt(G_slider_label,"%ld",lv_slider_get_value(target));
    }
    else if(target==B_slider)
    {
        lv_label_set_text_fmt(B_slider_label,"%ld",lv_slider_get_value(target));
    }
    else if(target==T_slider)
    {
        lv_label_set_text_fmt(T_slider_label,"%ld",lv_slider_get_value(target));
    }
    uint32_t rgb_value = 0;
    rgb_value |= lv_slider_get_value(R_slider);
    rgb_value <<= 8;
    rgb_value |= lv_slider_get_value(G_slider);
    rgb_value <<= 8;
    rgb_value |= lv_slider_get_value(B_slider);
    defaule_color = rgb_value;
    lv_style_init(&main_style);
    lv_style_set_radius(&main_style, 10);  // 设置圆角半径
    lv_style_set_border_width(&main_style, 0);
    lv_style_set_pad_all(&main_style, 10);
    lv_style_set_width(&main_style, 320);  // 设置宽
    lv_style_set_height(&main_style, 240); // 设置高
    lv_style_set_text_color(&main_style, lv_color_hex(0xdfdfdf));
    lv_style_set_bg_color(&main_style,lv_color_hex(defaule_color));
    defaule_opa = lv_slider_get_value(T_slider);
    lv_style_set_bg_opa(&main_style, defaule_opa);
    lv_obj_refresh_style(lv_scr_act(), LV_PART_ANY, LV_STYLE_PROP_ANY);
    lvgl_port_unlock();
    // lv_obj_add_style(lv_scr_act(), &lv_style_main, 0);

}

/// @brief 颜色设置页面的滚轮实现
/// @param base 
static void create_color_slider(lv_obj_t *base){
    R_slider=lv_slider_create(base);
    lv_obj_align(R_slider,LV_ALIGN_LEFT_MID,45,-5);
    lv_obj_set_size(R_slider,165,15);
    lv_slider_set_range(R_slider,0,255);
    lv_slider_set_value(R_slider,defaule_color & 0xff0000, LV_ANIM_ON); 
    lv_obj_set_style_bg_color(R_slider,lv_color_hex(0xFF0000),LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(R_slider,lv_color_hex(0xFF0000),LV_PART_MAIN);
    lv_obj_set_style_bg_color(R_slider,lv_color_hex(0xFF0000),LV_PART_KNOB);
    // lv_obj_add_event_cb(R_slider,event_cb,LV_EVENT_VALUE_CHANGED,NULL);
    R_slider_label=lv_label_create(base);
    lv_obj_align(R_slider_label,LV_ALIGN_LEFT_MID,220,-5);
    lv_label_set_text_fmt(R_slider_label,"%ld",lv_slider_get_value(R_slider));

    G_slider=lv_slider_create(base);
    lv_obj_align(G_slider,LV_ALIGN_LEFT_MID,45,25);
    lv_obj_set_size(G_slider,165,15);
    lv_slider_set_range(G_slider,0,255);
    lv_slider_set_value(G_slider,defaule_color & 0xff00, LV_ANIM_ON); 
    lv_obj_set_style_bg_color(G_slider,lv_color_hex(0x00FF00),LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(G_slider,lv_color_hex(0x00FF00),LV_PART_MAIN);
    lv_obj_set_style_bg_color(G_slider,lv_color_hex(0x00FF00),LV_PART_KNOB);
    // lv_obj_add_event_cb(G_slider,event_cb,LV_EVENT_VALUE_CHANGED,NULL);
    G_slider_label=lv_label_create(base);
    lv_obj_align(G_slider_label,LV_ALIGN_LEFT_MID,220,25);
    lv_label_set_text_fmt(G_slider_label,"%ld",lv_slider_get_value(G_slider));

    B_slider=lv_slider_create(base);
    lv_obj_align(B_slider,LV_ALIGN_LEFT_MID,45,55);
    lv_obj_set_size(B_slider,165,15);
    lv_slider_set_range(B_slider,0,255);
    lv_slider_set_value(B_slider,defaule_color & 0xff, LV_ANIM_ON); 
    lv_obj_set_style_bg_color(B_slider,lv_color_hex(0x0000FF),LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(B_slider,lv_color_hex(0x0000FF),LV_PART_MAIN);
    lv_obj_set_style_bg_color(B_slider,lv_color_hex(0x0000FF),LV_PART_KNOB);
    // lv_obj_add_event_cb(B_slider,event_cb,LV_EVENT_VALUE_CHANGED,NULL);
    B_slider_label=lv_label_create(base);
    lv_obj_align(B_slider_label,LV_ALIGN_LEFT_MID,220,55);
    lv_label_set_text_fmt(B_slider_label,"%ld",lv_slider_get_value(B_slider));

    T_slider=lv_slider_create(base);
    lv_obj_align(T_slider,LV_ALIGN_LEFT_MID,45,85);
    lv_obj_set_size(T_slider,165,15);
    lv_slider_set_range(T_slider,0,255);
    lv_slider_set_value(T_slider, defaule_opa, LV_ANIM_ON); 
    lv_obj_set_style_bg_color(T_slider,lv_color_hex(0x0000FF),LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(T_slider,lv_color_hex(0x0000FF),LV_PART_MAIN);
    lv_obj_set_style_bg_color(T_slider,lv_color_hex(0x0000FF),LV_PART_KNOB);
    // lv_obj_add_event_cb(T_slider,event_cb,LV_EVENT_VALUE_CHANGED,NULL);
    T_slider_label=lv_label_create(base);
    lv_obj_align(T_slider_label,LV_ALIGN_LEFT_MID,220,85);
    lv_label_set_text_fmt(T_slider_label,"%ld",lv_slider_get_value(T_slider));

    lv_obj_add_event_cb(R_slider, color_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(G_slider, color_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(B_slider, color_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(T_slider, color_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
}

/// @brief 返回按钮的回调函数
/// @param e 
void back_event_cb(lv_event_t *e)
{
    lv_obj_t *base = lv_event_get_user_data(e);
    lvgl_port_lock(0);
    lv_obj_clean(base);
    lv_obj_add_style(base, &main_style, 0);
    lv_set_page(base);
    lvgl_port_unlock();
}



/*************************************
 * 
 * 各种不同的设置页面回调函数
 * 
 * ************************************/


set_item_t *set_item_handle = NULL;

static void set_menu_btn_event_cb(lv_event_t *e);
/// @brief 添加一个设置页面的额项
/// @param name 页面的名字
/// @param handle 页面的显示函数
/// @param setting 页面所在的列
/// @param base 页面生成的页面
/// @param item_log 图标
/// @return 是否成功
esp_err_t set_item_add(char *name, page_handle_cb_t handle, lv_obj_t * setting, lv_obj_t *base, 
                        char * item_log, void (*button_handle_cb)(lv_event_t * )){
#if USING_CHINESE
    LV_FONT_DECLARE(font_alipuhui20);
#endif
    set_item_t *now = (set_item_t *)malloc(sizeof(set_item_t));
    if(now == NULL){
        ESP_LOGE(TAG, "malloc set item err");
        return ESP_FAIL;
    }
    now->page_handle_cb = handle;
    now->item_log = item_log;
    now->next = set_item_handle;
    set_item_handle = now;
#if USING_CHINESE
    now->name_ch = name;
#else 
    now->name_en = name;
#endif
    now->btn = lv_list_add_btn(setting, item_log, "");  /* 添加按钮 */
    lv_obj_t *label = lv_label_create(now->btn);

#if USING_CHINESE
    lv_obj_set_style_text_font(label, &font_alipuhui20, 0);
    lv_label_set_text(label, now->name_ch);
#else
    lv_label_set_text(label, now->name_en "                            ");
#endif
    lv_obj_align(label, LV_ALIGN_CENTER, -30, 0);
    lv_obj_add_event_cb(now->btn,button_handle_cb,LV_EVENT_CLICKED,base);
    return ESP_OK;
}
/// @brief 获取某一个item的处理函数
/// @param obj 
/// @return 
page_handle_cb_t app_item_get_handler(lv_obj_t *obj){
    set_item_t *now = set_item_handle;
    while(now != 0){
        if(now->btn == obj){
            return now->page_handle_cb;
        }
        now = now->next;
    }
    return NULL;
}
/// @brief  释放
void app_item_free(){
    set_item_t *now = set_item_handle;
    while(now != 0){
        set_item_t *temp = now;
        now = now->next;
        free(temp);
    }
}

void set_Bright_page_btn_cb(lv_obj_t *base){
    page_read("亮度");
    char data[] = "亮度";
    device_write(DEVICE_TTV_NUM, data, strlen(data), 0);
    slider_light =  create_slider(base);
    int lingt = bsp_display_brightness_get_defalut_light();
    lv_slider_set_value(slider_light, lingt, LV_ANIM_ON); // 设置亮度
}
void set_Sound_page_btn_cb(lv_obj_t *base){
    page_read("音量");
    char data[] = "音量";
    device_write(DEVICE_TTV_NUM, data, strlen(data), 0);
    uint8_t g_sys_volume = music_get_now_volume();
    slider_sound = create_slider(base);
    lv_slider_set_value(slider_sound, g_sys_volume, LV_ANIM_ON);   // 设置音量
}
void set_Lock_time_page_btn_cb(lv_obj_t *base){
    page_read("锁屏时间");
    char data[] = "锁屏时间";
    device_write(DEVICE_TTV_NUM, data, strlen(data), 0);
    create_lock_time_page(base);
}
void set_Page_read_page_btn_cb(lv_obj_t *base){
    page_read("翻页阅读");
    char data[] = "翻页阅读";
    device_write(DEVICE_TTV_NUM, data, strlen(data), 0);
    create_page_read_btn(base);
}
void color_set_gui(lv_obj_t *base)
{
#if USING_CHINESE
    LV_FONT_DECLARE(font_alipuhui20);
#endif
    lv_style_init(&main_style);
    lv_style_set_bg_color(&main_style,lv_color_hex(defaule_color));
    lv_obj_t *label=lv_label_create(base);
    // lv_obj_set_style_bg_color(base,lv_color_hex(defaule_color),LV_STATE_DEFAULT);
    lv_obj_add_style(base, &main_style, 0);
    // lv_obj_add_style(label, &main_style, 0);
#if USING_CHINESE
    lv_label_set_text(label,"   欢迎您使用玉皇大帝点读机!!!!\n现在您可以选择一个您喜欢的主题背景颜色，然后开启您的奇幻之旅");
    lv_obj_set_style_text_font(label,&font_alipuhui20,LV_STATE_DEFAULT);
#else
    lv_label_set_text(label, "Welcome to use the Jade Emperor reading machine!!!!\nNow you can choose a theme background color you like, and then start your fantasy journey");
#endif
    lv_obj_set_width(label,300);
    lv_obj_align(label,LV_ALIGN_TOP_MID,0,0);

    
    lv_obj_t *r=lv_label_create(base);
    lv_label_set_text(r,"R");
    lv_obj_set_style_text_font(r,&lv_font_montserrat_20,LV_STATE_DEFAULT);
    lv_obj_align(r,LV_ALIGN_LEFT_MID,10,-5);
    lv_obj_t *g=lv_label_create(base);
    lv_label_set_text(g,"G");
    lv_obj_set_style_text_font(g,&lv_font_montserrat_20,LV_STATE_DEFAULT);
    lv_obj_align(g,LV_ALIGN_LEFT_MID,10,25);
    lv_obj_t *b=lv_label_create(base);
    lv_label_set_text(b,"B");
    lv_obj_set_style_text_font(b,&lv_font_montserrat_20,LV_STATE_DEFAULT);
    lv_obj_align(b,LV_ALIGN_LEFT_MID,10,55);
    lv_obj_t *t=lv_label_create(base);
    lv_label_set_text(t,"T");
    lv_obj_set_style_text_font(t,&lv_font_montserrat_20,LV_STATE_DEFAULT);
    lv_obj_align(t,LV_ALIGN_LEFT_MID,10,85);
    

    create_color_slider(base);
}
/// @brief 关于我们的界面的烧写
/// @param base 
static void create_us(lv_obj_t * base)
{
    // lv_obj_t *btn;
    LV_IMG_DECLARE(five);
    LV_IMG_DECLARE(grab);
#if USING_CHINESE
    LV_FONT_DECLARE(font_alipuhui20);
#endif
    
    lv_obj_t *grab_img=lv_img_create(base);
    lv_img_set_src(grab_img,&grab);
    lv_img_set_zoom(grab_img,135);
    lv_obj_align(grab_img,LV_ALIGN_TOP_LEFT,-35,-20);

    lv_obj_t *five_img=lv_img_create(base);
    lv_img_set_src(five_img,&five);
    lv_img_set_zoom(five_img,135);
    lv_obj_align(five_img,LV_ALIGN_TOP_RIGHT,35,-20);

    lv_obj_t *label=lv_label_create(base);
#if USING_CHINESE
    lv_label_set_text(label,"哈哈哈哈哈哈感谢您点进这\n关于玉皇大帝点读机，这是\n一个大佬和一个小白历时两\n个月做出来的，因为大佬主\n要做了所有，除了小白做的\n（简单来说是两个框架和各\n种驱动），而小白主要做的\n有lvgl的一部分外加情绪\n价值（大佬你太厉害了！)\n哈哈哈哈祝您使用愉快!");
    lv_obj_set_style_text_font(label,&font_alipuhui20,LV_STATE_DEFAULT);
#else
    lv_label_set_text(label, "HaHaHaHaHaHa Thank you \nfor clicking on this\nAbout the Jade Emperor reading machine, this is\nA big guy and a newbie took two\nMonths to make, because the big guy\nMainly did everything except the little white\n(Done (simply put, two frameworks and each\nKind of driver), and the little white mainly did\nSome of lvgl plus emotional\nValue (Big guy, you are amazing!)\nHaHaHaHaHaHa wish you a\n happy use!");
#endif
    lv_obj_set_size(label,240,50);
    lv_label_set_long_mode(label,LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_anim_speed(label,30,0);
    lv_obj_align(label,LV_ALIGN_BOTTOM_MID,0,0);
    lv_obj_set_scroll_snap_y(label,LV_SCROLL_SNAP_CENTER);
    
}

void set_power_off(lv_obj_t *base){
    esp_restart();
}

//btn事件回调
//菜单界面的处理函数
static void set_menu_btn_event_cb(lv_event_t *e)
{
    lvgl_port_lock(0);
    // 获取用户数据
    lv_obj_t *base = lv_event_get_user_data(e);
    //返回键
    lv_obj_clean(base);
    lv_obj_add_style(base, &main_style, 0);
    // lv_obj_set_style_bg_color(base,lv_color_hex(0xf5f5f5),LV_STATE_DEFAULT);
    lv_obj_t *back =lv_btn_create(base);
    lv_obj_set_style_bg_color(back,lv_color_hex(0xb5b5b5),LV_STATE_DEFAULT);
    // lv_list_add_btn(back,LV_SYMBOL_LEFT,"back");

    lv_obj_t *lable = lv_label_create(back);
    lv_label_set_text(lable,LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(lable,lv_color_hex(0x000000),LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(back,lv_color_hex(0xf5f5f5),LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(back, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(back,back_event_cb,LV_EVENT_CLICKED,base);
    // lv_obj_set_size(back,50,50);

    lv_obj_t *target=lv_event_get_target(e);
    page_handle_cb_t handle = app_item_get_handler(target);
    if(handle != NULL){
        handle(base);
    }else{
        ESP_LOGE(TAG, "find set item handler err");
    }
    lvgl_port_unlock();
    
}

/// @brief 设置页面
/// @param base 
void lv_set_page(lv_obj_t *base)
{
#if USING_CHINESE
    LV_FONT_DECLARE(font_alipuhui20);
#endif
    lv_obj_t *setting = lv_list_create(base);
    // lv_obj_set_style_text_font(setting, &font_alipuhui20, 0);
    lv_obj_set_size(setting, 300, 220);
    // lv_obj_set_style_bg_color(setting,lv_color_hex(0xb5b5b5),LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(setting, LV_OPA_50, 0);
    // 添加按钮
    lv_list_add_text(setting,"Setting");

    set_item_add(
#if USING_CHINESE
        "亮度                                    ", 
#else
        "Brightness",
#endif
        set_Bright_page_btn_cb,setting,base, LV_SYMBOL_SETTINGS, set_menu_btn_event_cb);

    set_item_add(
#if USING_CHINESE
        "音量                                    ", 
#else
        "Sound",
#endif
        set_Sound_page_btn_cb,setting,base, LV_SYMBOL_AUDIO, set_menu_btn_event_cb);

    set_item_add(
#if USING_CHINESE
        "锁屏时间                            ", 
#else
        "Lock time",
#endif
        set_Lock_time_page_btn_cb,setting,base, LV_SYMBOL_SETTINGS, set_menu_btn_event_cb);


    set_item_add(
#if USING_CHINESE
        "页面阅读                            ", 
#else
        "Page Read",
#endif
        set_Page_read_page_btn_cb,setting,base, LV_SYMBOL_VOLUME_MID, set_menu_btn_event_cb);

    set_item_add(
#if USING_CHINESE
        "背景颜色                            ", 
#else
        "Bg Color",
#endif
        color_set_gui,setting,base, LV_SYMBOL_SETTINGS, set_menu_btn_event_cb);


    set_item_add(
#if USING_CHINESE
        "关于我们                            ", 
#else
        "About us",
#endif
        create_us,setting,base, LV_SYMBOL_BELL, set_menu_btn_event_cb);

    set_item_add(
#if USING_CHINESE
        "关机                                    ", 
#else
        "Power OFF",
#endif
        set_power_off,setting,base, LV_SYMBOL_POWER, set_menu_btn_event_cb);
}


