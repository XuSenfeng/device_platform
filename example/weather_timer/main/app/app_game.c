/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2025-01-03 18:51:58
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-05 09:45:50
 */
#include "app_game.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "device_bsp.h"
#include "app_ui.h"

static lv_obj_t *Story_Text;
static lv_obj_t *Riddle;
static lv_obj_t *Poisonous_chicken_soup;
static lv_obj_t *Constellation;
static lv_obj_t *Birthday_character;
static lv_obj_t *Game;
static lv_obj_t *Photo;
static lv_obj_t *roller_month;
static lv_obj_t *roller_day;
static lv_obj_t *roller_day2;
lv_obj_t * text_context_windows;
lv_obj_t *roller_constellation;
lv_obj_t *canvsa;


set_item_t *game_manage = NULL;
/// @brief 各种翻页函数
/// @param e 
void story_next_page_btn_cb(lv_event_t *e){
    deal_story(0, 1);
}
void story_pre_page_btn_cb(lv_event_t *e){
    deal_story(0, 0);
}

void birthday_next_page_btn_cb(lv_event_t *e){
    deal_birthday_character(0, 1);
}
void birthday_pre_page_btn_cb(lv_event_t *e){
    deal_birthday_character(0, 0);
}
void riddle_next_page_btn_cb(lv_event_t *e){
    deal_riddle(0, 1);
}
void riddle_pre_page_btn_cb(lv_event_t *e){
    deal_riddle(0, 0);
}


/// @brief app页面返回按钮回调函数
/// @param e 
void app_back_event_cb(lv_event_t *e)
{
    lv_obj_t *base = (lv_obj_t *)lv_event_get_user_data(e);
    text_context_windows = NULL;
    lvgl_port_lock(0);
    // lv_obj_clean(base);
    // lv_obj_add_style(base, &main_style, 0);
    // lv_app_page(base, );
    extern lv_obj_t *main_tileview;
    extern int init_mode;
    if(init_mode)
        lv_obj_set_tile_id(wifi_ui->tileview, 2, 1, LV_ANIM_ON);
    else
        lv_obj_set_tile_id(main_tileview, 2, 1, LV_ANIM_ON);
    if_camer_show = 0;
    lvgl_port_unlock();
}

/// @brief 获取选择的日期进行处理
/// @param e 
void btn_birthday_cb(lv_event_t *e){
    lv_obj_t *base = lv_event_get_user_data(e);
    int month = lv_roller_get_selected(roller_month);
    int day = lv_roller_get_selected(roller_day);
    int day2 = lv_roller_get_selected(roller_day2);
    lvgl_port_lock(0);
    //返回键
    lv_obj_clean(base);
    lv_obj_add_style(base, &main_style, 0);
    lv_obj_t *back =lv_btn_create(base);
    lv_obj_t *lable = lv_label_create(back);
    lv_label_set_text(lable,LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(lable,lv_color_hex(0x000000),LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(back,lv_color_hex(0x7c7c7c),LV_STATE_DEFAULT);
    lv_obj_set_style_opa(back, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(back, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(back,app_back_event_cb,LV_EVENT_CLICKED,base);
    lv_obj_set_align(lable, LV_ALIGN_CENTER);
    lv_obj_set_size(back,25,25);

    text_context_windows = create_text(base, birthday_next_page_btn_cb, birthday_pre_page_btn_cb, 0);
    lvgl_port_unlock();
    
    ESP_LOGI("btn_birthday_cb", "month = %d, day = %d, day2 = %d", month, day, day2);
    static char birthday[30];
    if(day==0){
        sprintf(birthday, "&m=%d&d=%d", month+1, day2);
    }else if(day==0 && day2==0){
        ESP_LOGE("btn_birthday", "err date");
    }else{
        sprintf(birthday, "&m=%d&d=%1d%1d", month+1, day, day2);
    }
    get_http_data_birthday_character(birthday);
}
//获取生日界面
void lv_birthday_page(lv_obj_t * base){
    static lv_style_t style;
    const char * month = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12";
    const char * day = "0\n1\n2\n3";
    const char * day2 = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9";
    ESP_LOGI("lv_birthday_page", "lv_birthday_page");
    lvgl_port_lock(0);
#if USING_CHINESE
    LV_FONT_DECLARE(font_alipuhui20);
#endif
    lv_obj_t *label=lv_label_create(base);
#if USING_CHINESE
    lv_label_set_text(label,"请输入您的生日");
    lv_obj_set_style_text_font(label,&font_alipuhui20,LV_STATE_DEFAULT);
#else
    lv_label_set_text(label, "Please enter your birthday");
#endif
    lv_obj_align(label,LV_ALIGN_TOP_MID,0,20);

    lv_obj_t *label_month=lv_label_create(base);
#if USING_CHINESE
    lv_label_set_text(label_month,"月");
    lv_obj_set_style_text_font(label_month,&font_alipuhui20,LV_STATE_DEFAULT);
#else
    lv_label_set_text(label_month, "month");
#endif
    lv_obj_align(label_month,LV_ALIGN_LEFT_MID,103,0);

    lv_obj_t *label_day=lv_label_create(base);
#if USING_CHINESE
    lv_label_set_text(label_day,"号");
    lv_obj_set_style_text_font(label_day,&font_alipuhui20,LV_STATE_DEFAULT);
#else
    lv_label_set_text(label_day, "day");
#endif

    lv_obj_align(label_day,LV_ALIGN_RIGHT_MID,-25,0);

    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_white());
    lv_style_set_text_color(&style, lv_color_black());
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_style_set_radius(&style, 0);
    // 创建"数字"roller
    
    roller_month = lv_roller_create(base);
    lv_obj_add_style(roller_month, &style, 0);
    lv_obj_set_style_bg_opa(roller_month, LV_OPA_50, LV_PART_SELECTED);

    lv_roller_set_options(roller_month, month, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_month, 3); // 显示3行
    lv_roller_set_selected(roller_month, 6, LV_ANIM_OFF); // 默认选择
    lv_obj_set_width(roller_month, 60);
    lv_obj_set_style_text_font(roller_month, &lv_font_montserrat_20, 0);
    lv_obj_align(roller_month, LV_ALIGN_BOTTOM_LEFT, 35, -70);
    // lv_obj_add_event_cb(roller_num, mask_event_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    roller_day = lv_roller_create(base);
    lv_obj_add_style(roller_day, &style, 0);
    lv_obj_set_style_bg_opa(roller_day, LV_OPA_50, LV_PART_SELECTED);

    lv_roller_set_options(roller_day, day, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_day, 3); // 显示3行
    lv_roller_set_selected(roller_day, 1, LV_ANIM_OFF); // 默认选择
    lv_obj_set_width(roller_day, 60);
    lv_obj_set_style_text_font(roller_day, &lv_font_montserrat_20, 0);
    lv_obj_align(roller_day, LV_ALIGN_BOTTOM_RIGHT, -120, -70);

    roller_day2 = lv_roller_create(base);
    lv_obj_add_style(roller_day2, &style, 0);
    lv_obj_set_style_bg_opa(roller_day2, LV_OPA_50, LV_PART_SELECTED);

    lv_roller_set_options(roller_day2, day2, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_day2, 3); // 显示3行
    lv_roller_set_selected(roller_day2, 5, LV_ANIM_OFF); // 默认选择
    lv_obj_set_width(roller_day2, 60);
    lv_obj_set_style_text_font(roller_day2, &lv_font_montserrat_20, 0);
    lv_obj_align(roller_day2, LV_ALIGN_BOTTOM_RIGHT, -50, -70);

    //确认键
    lv_obj_t *btn_num_ok = lv_btn_create(base);
    lv_obj_align(btn_num_ok, LV_ALIGN_BOTTOM_MID, 0, -10); // 位置
    lv_obj_set_width(btn_num_ok, 90); // 宽度
    lv_obj_add_event_cb(btn_num_ok, btn_birthday_cb, LV_EVENT_CLICKED, base); // 事件处理函数

    lv_obj_t *label_num_ok = lv_label_create(btn_num_ok);
    lv_label_set_text(label_num_ok, LV_SYMBOL_OK);
    lv_obj_set_style_text_font(label_num_ok, &lv_font_montserrat_20, 0);
    lv_obj_center(label_num_ok);
    lvgl_port_unlock();
    ESP_LOGI("lv_birthday_page", "lv_birthday_page end");
}


// 星座
const char *start_buf[] = {
    "aries",
    "taurus",
    "gemini",
    "cancer",
    "leo",
    "virgo",
    "libra",
    "scorpio",
    "sagittarius",
    "capricorn",
    "aquarius",
    "pisces",
};
void btn_constellation_cb(lv_event_t *e){
    lv_obj_t *base = lv_event_get_user_data(e);
    int str_num = lv_roller_get_selected(roller_constellation);
    lvgl_port_lock(0);
    //返回键
    lv_obj_clean(base);
    lv_obj_add_style(base, &main_style, 0);
    lv_obj_t *back =lv_btn_create(base);
    lv_obj_t *lable = lv_label_create(back);
    lv_label_set_text(lable,LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(lable,lv_color_hex(0x000000),LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(back,lv_color_hex(0x7c7c7c),LV_STATE_DEFAULT);
    lv_obj_set_style_opa(back, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(back, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(back,app_back_event_cb,LV_EVENT_CLICKED,base);
    lv_obj_set_align(lable, LV_ALIGN_CENTER);
    lv_obj_set_size(back,25,25);

    text_context_windows = create_text(base, NULL, NULL, 0);
    lvgl_port_unlock();
    
    ESP_LOGI("btn_constellation_cb", "str_num = %d", str_num);
    if(str_num > 11){
        str_num = 0;
    }
    const char * constellation = start_buf[str_num];
    get_http_data_constellation(constellation);
}
/// @brief 星座选择界面
/// @param base 
void lv_constellation_page(lv_obj_t * base){
    ESP_LOGI("lv_constellation_page", "lv_constellation_page");
    lvgl_port_lock(0);
#if USING_CHINESE
    char * constellation = "白羊座\n金牛座\n双子座\n巨蟹座\n狮子座\n处女座\n天秤座\n天蝎座\n射手座\n摩羯座\n水瓶座\n双鱼座";
    LV_FONT_DECLARE(font_alipuhui20);
#else
    char * constellation = "Aries\nTaurus\nGemini\nCancer\nLeo\nVirgo\nLibra\nScorpio\nSagittarius\nCapricorn\nAquarius\nPisces";
#endif
    lv_obj_t *label=lv_label_create(base);
#if USING_CHINESE
    lv_label_set_text(label,"请输入您的星座");
    lv_obj_set_style_text_font(label,&font_alipuhui20,LV_STATE_DEFAULT);
#else
    lv_label_set_text(label, "Please enter your constellation");
#endif
    lv_obj_align(label,LV_ALIGN_TOP_MID,0,20);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_white());
    lv_style_set_text_color(&style, lv_color_black());
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_style_set_radius(&style, 0);
    
    // roller
    roller_constellation = lv_roller_create(base);
    lv_obj_add_style(roller_constellation, &style, 0);
    lv_obj_set_style_bg_opa(roller_constellation, LV_OPA_50, LV_PART_SELECTED);

    lv_roller_set_options(roller_constellation, constellation, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_constellation, 3); // 显示3行
    lv_roller_set_selected(roller_constellation, 6, LV_ANIM_OFF); // 默认选择
    lv_obj_set_width(roller_constellation, 110);
#if USING_CHINESE
    lv_obj_set_style_text_font(roller_constellation, &font_alipuhui20, 0);
#endif
    lv_obj_align(roller_constellation, LV_ALIGN_CENTER, 0, -3);
    // lv_obj_add_event_cb(roller_num, mask_event_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    //确认键
    lv_obj_t *btn_num_ok = lv_btn_create(base);
    lv_obj_align(btn_num_ok, LV_ALIGN_BOTTOM_MID, 0, -10); // 位置
    lv_obj_set_width(btn_num_ok, 90); // 宽度
    lv_obj_add_event_cb(btn_num_ok, btn_constellation_cb, LV_EVENT_CLICKED, base); // 事件处理函数

    lv_obj_t *label_num_ok = lv_label_create(btn_num_ok);
    lv_label_set_text(label_num_ok, LV_SYMBOL_OK);
    lv_obj_set_style_text_font(label_num_ok, &lv_font_montserrat_20, 0);
    lv_obj_center(label_num_ok);
    lvgl_port_unlock();
    ESP_LOGI("lv_constellation_page", "lv_constellation_page end");
}




/// @brief app页面
/// @param e 
void app_btn_event_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // 获取用户数据
        lvgl_port_lock(0);
        lv_obj_t *base = lv_event_get_user_data(e);
        //返回键
        lv_obj_clean(base);
        lv_obj_add_style(base, &main_style, 0);
        lv_obj_t *back =lv_btn_create(base);
        lv_obj_t *lable = lv_label_create(back);
        lv_label_set_text(lable,LV_SYMBOL_LEFT);
        lv_obj_set_style_text_color(lable,lv_color_hex(0x000000),LV_STATE_DEFAULT);

        lv_obj_set_style_bg_color(back,lv_color_hex(0x7c7c7c),LV_STATE_DEFAULT);
        lv_obj_set_style_opa(back, LV_OPA_50, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(back, 0, LV_PART_MAIN);
        lv_obj_add_event_cb(back,app_back_event_cb,LV_EVENT_CLICKED,base);
        lv_obj_set_align(lable, LV_ALIGN_CENTER);
        lv_obj_set_size(back,25,25);
        // lv_obj_set_size(back,50,50);

        lv_obj_t *target=lv_event_get_target(e);
        if(target==Story_Text){
            get_http_data_story();
            text_context_windows = create_text(base, story_next_page_btn_cb, story_pre_page_btn_cb, 0);
        }else if(target == Riddle){
            text_context_windows = create_text(base, riddle_next_page_btn_cb, riddle_pre_page_btn_cb, 0);
            get_http_data_riddle();
        }else if(target == Game){
            game_main(base);
        }else if(target == Poisonous_chicken_soup){
            text_context_windows = create_text(base, NULL, NULL, 0);
            get_http_data_poisonous_chicken_soup();
        }else if(target == Constellation){
            lv_constellation_page(base);
        }else if(target == Birthday_character){
            ESP_LOGI("app_btn_event_cb", "Birthday_character");
            lv_birthday_page(base);
        }else if(target == Photo){
            canvsa = lv_canvas_create(base);
            lv_obj_set_size(canvsa, 240, 176);
            lv_obj_align(canvsa, LV_ALIGN_CENTER, 0, 0);
            extern char *canvsa_buf;
            if(!canvsa_buf){
                canvsa_buf = malloc(240 *176 *2);
            }
            lv_canvas_set_buffer(canvsa, canvsa_buf, 240, 176, LV_IMG_CF_TRUE_COLOR);
            if_camer_show = 1;
        }
        extern lv_obj_t *main_tileview;
        extern int init_mode;
        if(init_mode)
            lv_obj_set_tile_id(wifi_ui->tileview, 3, 1, LV_ANIM_ON);
        else
            lv_obj_set_tile_id(main_tileview, 3, 1, LV_ANIM_ON);
        lvgl_port_unlock();
    }
}

/// @brief APP页面的设置
/// @param base 
void lv_app_page(lv_obj_t *base, lv_obj_t *app_page){
    //Program
    extern int init_mode;
#if USING_CHINESE
    LV_FONT_DECLARE(font_alipuhui20);
#endif
    lv_obj_t *Program = lv_list_create(base);
    // lv_obj_set_style_text_font(Program, &font_alipuhui20, 0);
    lv_obj_set_size(Program, 300, 220);
    lv_obj_align(Program,LV_ALIGN_TOP_RIGHT,0,0);
    lv_obj_set_style_bg_opa(Program, LV_OPA_50, 0);
    lv_list_add_text(Program,"Program");
    lv_obj_t *lable;
    if(init_mode){
        Story_Text = lv_list_add_btn(Program, LV_SYMBOL_FILE, "");  /* 添加按钮 */
        lable = lv_label_create(Story_Text);
#if USING_CHINESE
        lv_obj_set_style_text_font(lable, &font_alipuhui20, 0);
        lv_label_set_text(lable, "故事                                     ");
#else
        lv_label_set_text(lable, "Story                                     ");
#endif
        lv_obj_align(lable, LV_ALIGN_CENTER, -30, 0);
        lv_obj_add_event_cb(Story_Text,app_btn_event_cb,LV_EVENT_CLICKED,app_page);

        Riddle = lv_list_add_btn(Program, LV_SYMBOL_SETTINGS, "");  /* 添加按钮 */
        lable = lv_label_create(Riddle);
#if USING_CHINESE
        lv_obj_set_style_text_font(lable, &font_alipuhui20, 0);
        lv_label_set_text(lable, "脑筋急转弯                          ");
#else
        lv_label_set_text(lable, "Riddle                                     ");
#endif
        lv_obj_align(lable, LV_ALIGN_CENTER, -30, 0);
        lv_obj_add_event_cb(Riddle,app_btn_event_cb,LV_EVENT_CLICKED,app_page);

        Poisonous_chicken_soup = lv_list_add_btn(Program, LV_SYMBOL_FILE, "");  /* 添加按钮 */
        lable = lv_label_create(Poisonous_chicken_soup);
#if USING_CHINESE
        lv_obj_set_style_text_font(lable, &font_alipuhui20, 0);
        lv_label_set_text(lable, "毒鸡汤                                    ");
#else
        lv_label_set_text(lable, "Poisonous chicken soup                                     ");
#endif
        lv_obj_align(lable, LV_ALIGN_CENTER, -30, 0);
        lv_obj_add_event_cb(Poisonous_chicken_soup,app_btn_event_cb,LV_EVENT_CLICKED,app_page);

        Constellation = lv_list_add_btn(Program, LV_SYMBOL_FILE, "");  /* 添加按钮 */
        lable = lv_label_create(Constellation);
#if USING_CHINESE
        lv_obj_set_style_text_font(lable, &font_alipuhui20, 0);
        lv_label_set_text(lable, "星座运势                             ");
#else
        lv_label_set_text(lable, "Constellation                                     ");
#endif
        lv_obj_align(lable, LV_ALIGN_CENTER, -30, 0);
        lv_obj_add_event_cb(Constellation,app_btn_event_cb,LV_EVENT_CLICKED,app_page);
        
        Birthday_character = lv_list_add_btn(Program, LV_SYMBOL_FILE, "Birthday");  /* 添加按钮 */

        lv_obj_add_event_cb(Birthday_character,app_btn_event_cb,LV_EVENT_CLICKED,app_page);
    }else{
        Photo = lv_list_add_btn(Program, LV_SYMBOL_PLAY, "");  /* 添加按钮 */
        lable = lv_label_create(Photo);
#if USING_CHINESE
        lv_obj_set_style_text_font(lable, &font_alipuhui20, 0);
        lv_label_set_text(lable, "照相机                            ");
#else
        lv_label_set_text(lable, "Photo                            ");
#endif
        lv_obj_align(lable, LV_ALIGN_CENTER, -30, 0);
        lv_obj_add_event_cb(Photo,app_btn_event_cb,LV_EVENT_CLICKED,app_page);
    }

    Game = lv_list_add_btn(Program, LV_SYMBOL_PLAY, "");  /* 添加按钮 */
    lable = lv_label_create(Game);
#if USING_CHINESE
    lv_obj_set_style_text_font(lable, &font_alipuhui20, 0);
    lv_label_set_text(lable, "2048游戏                            ");
#else
    lv_label_set_text(lable, "Game                            ");
#endif
    lv_obj_align(lable, LV_ALIGN_CENTER, -30, 0);
    lv_obj_add_event_cb(Game,app_btn_event_cb,LV_EVENT_CLICKED,app_page);

    ESP_LOGI("lv_app_page", "lv_app_page end");
}