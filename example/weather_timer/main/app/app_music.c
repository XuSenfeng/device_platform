/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-09 11:41:07
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-05 09:12:16
 */
#include "app_music.h"
#include "audio_player.h"
#include "app_ui.h"
// #include "esp32_s3_szp.h"
#include "sd_card.h"
#include "device_bsp.h"
#include "file_iterator.h"
#include "string.h"
#include <dirent.h>
#include "lvgl.h"

#include "esp_lvgl_port.h"

static const char *TAG = "app_ui";

static audio_player_config_t player_config = {0};
uint8_t g_sys_volume = DEFAULT_SOUND_VALUSE;
extern lv_style_t main_style;

lv_obj_t *music_list;
lv_obj_t *label_play_pause;
lv_obj_t *btn_play_pause;
lv_obj_t *volume_slider;
lv_obj_t *music_ui_page;
lv_obj_t *main_tileview;


uint8_t music_get_now_volume(void){
    return g_sys_volume;
}

void music_set_now_volume(uint8_t volume){
    g_sys_volume = volume;
}

// 播放指定序号的音乐
static void play_index(int index)
{
    ESP_LOGI(TAG, "play_index(%d)", index);

    char filename[128];
    int retval = file_iterator_get_full_path_from_index(file_iterator_sd, index, filename, sizeof(filename));
    if (retval == 0) {
        ESP_LOGE(TAG, "unable to retrieve filename");
        return;
    }

    FILE *fp = fopen(filename, "rb");
    if (fp) {
        ESP_LOGI(TAG, "Playing '%s'", filename);
        audio_player_play(fp);
    } else {
        ESP_LOGE(TAG, "unable to open index %d, filename '%s'", index, filename);
    }
}

// 设置声音处理函数
static esp_err_t _audio_player_mute_fn(AUDIO_PLAYER_MUTE_SETTING setting)
{
    esp_err_t ret = ESP_OK;
    // 判断是否需要静音
    bsp_codec_mute_set(setting == AUDIO_PLAYER_MUTE ? true : false);
    // 如果不是静音 设置音量
    if (setting == AUDIO_PLAYER_UNMUTE) {
        bsp_codec_volume_set(g_sys_volume, NULL);
    }
    ret = ESP_OK;

    return ret;
}

// 播放音乐函数 播放音乐的时候 会不断进入
static esp_err_t _audio_player_write_fn(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;

    ret = bsp_i2s_write(audio_buffer, len, bytes_written, timeout_ms);

    return ret;
}

// 设置采样率 播放的时候进入一次
static esp_err_t _audio_player_std_clock(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
{
    esp_err_t ret = ESP_OK;

    // ret = bsp_speaker_set_fs(rate, bits_cfg, ch);
    // ret = bsp_codec_set_fs(rate, bits_cfg, ch);
    return ret;
}

// 回调函数 播放器每次动作都会进入
static void _audio_player_callback(audio_player_cb_ctx_t *ctx)
{
    ESP_LOGI(TAG, "ctx->audio_event = %d", ctx->audio_event);
    switch (ctx->audio_event) {
    case AUDIO_PLAYER_CALLBACK_EVENT_IDLE: {  // 播放完一首歌 进入这个case
        ESP_LOGI(TAG, "AUDIO_PLAYER_REQUEST_IDLE");
        // 指向下一首歌
        file_iterator_next(file_iterator_sd);
        int index = file_iterator_get_index(file_iterator_sd);
        ESP_LOGI(TAG, "playing index '%d'", index);
        play_index(index);
        // 修改当前播放的音乐名称
        lvgl_port_lock(0);
        lv_dropdown_set_selected(music_list, index);
        lv_obj_t *label_title = (lv_obj_t *) music_list->user_data;
        lv_label_set_text_static(label_title, file_iterator_get_name_from_index(file_iterator_sd, index));
        lvgl_port_unlock();
        break;
    }
    case AUDIO_PLAYER_CALLBACK_EVENT_PLAYING: // 正在播放音乐
        ESP_LOGI(TAG, "AUDIO_PLAYER_REQUEST_PLAY");
        pa_en(1); // 打开音频功放
        break;
    case AUDIO_PLAYER_CALLBACK_EVENT_PAUSE: // 正在暂停音乐
        ESP_LOGI(TAG, "AUDIO_PLAYER_REQUEST_PAUSE");
        pa_en(0); // 关闭音频功放
        break;
    default:
        break;
    }
}

// mp3播放器初始化
void mp3_player_init(void *data)
{
    // 获取文件信息
    // file_iterator_sd = file_iterator_new(MOUNT_POINT); // 记录一下文件的迭代器
    assert(file_iterator_sd != NULL);

    // 初始化音频播放
    player_config.mute_fn = _audio_player_mute_fn;
    player_config.write_fn = _audio_player_write_fn;
    player_config.clk_set_fn = _audio_player_std_clock;
    player_config.priority = 6;
    player_config.coreID = 1;

    ESP_ERROR_CHECK(audio_player_new(player_config));
    ESP_ERROR_CHECK(audio_player_callback_register(_audio_player_callback, NULL));

    // 显示界面
    music_main_page();
}


// 按钮样式相关定义
typedef struct {
    lv_style_t style_bg;
    lv_style_t style_focus_no_outline;
} button_style_t;

static button_style_t g_btn_styles;

button_style_t *ui_button_styles(void)
{
    return &g_btn_styles;
}

// 按钮样式初始化
static void ui_button_style_init(void)
{
    /*Init the style for the default state*/
    lv_style_init(&g_btn_styles.style_focus_no_outline);
    lv_style_set_outline_width(&g_btn_styles.style_focus_no_outline, 0);

    lv_style_init(&g_btn_styles.style_bg);
    lv_style_set_bg_opa(&g_btn_styles.style_bg, LV_OPA_100);
    lv_style_set_bg_color(&g_btn_styles.style_bg, lv_color_make(255, 255, 255));
    lv_style_set_shadow_width(&g_btn_styles.style_bg, 0);
}

// 播放暂停按钮 事件处理函数
static void btn_play_pause_cb(lv_event_t *event)
{
    lv_obj_t *btn = lv_event_get_target(event);
    lv_obj_t *lab = (lv_obj_t *) btn->user_data;

    audio_player_state_t state = audio_player_get_state();
    printf("state=%d\n", state);
    if(state == AUDIO_PLAYER_STATE_IDLE){
        lvgl_port_lock(0);
        lv_label_set_text_static(lab, LV_SYMBOL_PAUSE);
        lvgl_port_unlock();
        int index = file_iterator_get_index(file_iterator_sd);     //获取当前播放的音乐序号
        ESP_LOGI(TAG, "playing index '%d'", index);
        play_index(index);
    }else if (state == AUDIO_PLAYER_STATE_PAUSE) {
        lvgl_port_lock(0);
        lv_label_set_text_static(lab, LV_SYMBOL_PAUSE);
        lvgl_port_unlock();
        audio_player_resume();
    } else if (state == AUDIO_PLAYER_STATE_PLAYING) {
        lvgl_port_lock(0);
        lv_label_set_text_static(lab, LV_SYMBOL_PLAY);
        lvgl_port_unlock();
        audio_player_pause();
    }
}

// 上一首 下一首 按键事件处理函数
static void btn_prev_next_cb(lv_event_t *event)
{
    bool is_next = (bool) event->user_data;

    if (is_next) {
        ESP_LOGI(TAG, "btn next");
        file_iterator_next(file_iterator_sd);  // 指向下一首歌
    } else {
        ESP_LOGI(TAG, "btn prev");
        file_iterator_prev(file_iterator_sd);
    }
    // 修改当前的音乐名称
    int index = file_iterator_get_index(file_iterator_sd); // 获取当前播放的音乐序号
    lvgl_port_lock(0);
    lv_dropdown_set_selected(music_list, index);
    lv_obj_t *label_title = (lv_obj_t *) music_list->user_data;
    lv_label_set_text_static(label_title, file_iterator_get_name_from_index(file_iterator_sd, index));
    lvgl_port_unlock();
    // 执行音乐事件
    audio_player_state_t state = audio_player_get_state();
    printf("prev_next_state=%d\n", state);
    if (state == AUDIO_PLAYER_STATE_IDLE) { 
        // Nothing to do
    }else if (state == AUDIO_PLAYER_STATE_PAUSE){ // 如果当前正在暂停歌曲
        ESP_LOGI(TAG, "playing index '%d'", index);
        play_index(index);
        audio_player_pause();
    } else if (state == AUDIO_PLAYER_STATE_PLAYING) { // 如果当前正在播放歌曲
        // 播放歌曲
        ESP_LOGI(TAG, "playing index '%d'", index);
        play_index(index);
    }
}

// 音量调节滑动条 事件处理函数
static void volume_slider_cb(lv_event_t *event)
{
    lv_obj_t *slider = lv_event_get_target(event);
    int volume = lv_slider_get_value(slider); // 获取slider的值
    bsp_codec_volume_set(volume, NULL); // 设置声音大小
    g_sys_volume = volume; // 把声音赋值给g_sys_volume保存
    ESP_LOGI(TAG, "volume '%d'", volume);
}

// 音乐列表 点击事件处理函数
static void music_list_cb(lv_event_t *event)
{
    lv_obj_t *label_title = (lv_obj_t *) music_list->user_data;
    
    uint16_t index = lv_dropdown_get_selected(music_list);
    ESP_LOGI(TAG, "switching index to '%d'", index);
    file_iterator_set_index(file_iterator_sd, index);
    lvgl_port_lock(0);
    lv_label_set_text_static(label_title, file_iterator_get_name_from_index(file_iterator_sd, index));
    lvgl_port_unlock();
    
    audio_player_state_t state = audio_player_get_state();
    if (state == AUDIO_PLAYER_STATE_PAUSE){ // 如果当前正在暂停歌曲
        play_index(index);
        audio_player_pause();
    } else if (state == AUDIO_PLAYER_STATE_PLAYING) { // 如果当前正在播放歌曲
        play_index(index);
    }
}

// 音乐名称加入列表
static void build_file_list(lv_obj_t *music_list)
{
    lv_obj_t *label_title = (lv_obj_t *) music_list->user_data;

    lvgl_port_lock(0);
    lv_dropdown_clear_options(music_list);
    lvgl_port_unlock();

    for(size_t i = 0; i<file_iterator_sd->count; i++)
    {
        const char *file_name = file_iterator_get_name_from_index(file_iterator_sd, i);
        if (NULL != file_name) {
            lvgl_port_lock(0);
            lv_dropdown_add_option(music_list, file_name, i); // 添加音乐名称到列表中
            lvgl_port_unlock();
        }
    }
    lvgl_port_lock(0);
    lv_dropdown_set_selected(music_list, 0); // 选择列表中的第一个
    lv_label_set_text_static(label_title, file_iterator_get_name_from_index(file_iterator_sd, 0)); // 显示list中第一个音乐的名称
    lvgl_port_unlock();
}
void lv_set_page(lv_obj_t *base);
void music_main_page(void){
    main_tileview = lv_tileview_create(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0); // 修改背景为黑色

    /*Create an object with the new style*/
    lv_obj_t * obj = lv_tileview_add_tile(main_tileview, 1, 1, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP);
    lv_obj_t * obj2 = lv_tileview_add_tile(main_tileview, 0, 1, LV_DIR_RIGHT);
    lv_obj_t * obj3 = lv_tileview_add_tile(main_tileview, 1, 0, LV_DIR_BOTTOM);
    lv_obj_t * obj4 = lv_tileview_add_tile(main_tileview, 2, 1, LV_DIR_LEFT);
    lv_obj_t * obj5 = lv_tileview_add_tile(main_tileview, 3, 1, LV_DIR_NONE);
    // my_list(obj2); //初始化一下左侧的列表

    // 设置初始位置
    lv_obj_set_tile_id(main_tileview, 1, 1, LV_ANIM_OFF);
    lv_obj_remove_style(main_tileview, NULL, LV_PART_SCROLLBAR);

    lv_obj_add_style(obj, &main_style, 0);
    lv_obj_add_style(obj2, &main_style, 0);
    lv_obj_add_style(obj3, &main_style, 0);
    lv_obj_add_style(obj4, &main_style, 0);
    lv_obj_add_style(obj5, &main_style, 0);
    lvgl_port_unlock();

    lvgl_port_lock(0);
    music_ui(obj);
    lvgl_port_unlock();

    lvgl_port_lock(0);
    lv_set_page(obj2);
    lvgl_port_unlock();

    lvgl_port_lock(0);
    lv_dir_page(obj3);
    lvgl_port_unlock();

    lvgl_port_lock(0);
    lv_app_page(obj4, obj5);
    lvgl_port_unlock();
}

// 播放器界面初始化
void music_ui(lv_obj_t *base)
{

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_opa( &style, LV_OPA_50 ); // 背景透明度
    lv_style_set_border_width(&style, 0); // 边框宽度
    lv_style_set_pad_all(&style, 0);  // 内间距
    lv_style_set_radius(&style, 0);   // 圆角半径
    lv_style_set_width(&style, 300);  // 宽
    lv_style_set_height(&style, 230); // 高
    music_ui_page = lv_obj_create(base);  
    // lv_obj_set_style_bg_opa(music_ui_page, LV_OPA_50, 0);
    lv_obj_add_style(music_ui_page, &style, 0);

    ui_button_style_init();// 初始化按键风格

    /* 创建播放暂停控制按键 */
    btn_play_pause = lv_btn_create(music_ui_page);
    lv_obj_align(btn_play_pause, LV_ALIGN_CENTER, 0, 40);
    lv_obj_set_size(btn_play_pause, 50, 50);
    lv_obj_set_style_radius(btn_play_pause, 25, LV_STATE_DEFAULT);
    lv_obj_add_flag(btn_play_pause, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_add_style(btn_play_pause, &ui_button_styles()->style_focus_no_outline, LV_STATE_FOCUS_KEY);
    lv_obj_add_style(btn_play_pause, &ui_button_styles()->style_focus_no_outline, LV_STATE_FOCUSED);

    label_play_pause = lv_label_create(btn_play_pause);
    lv_label_set_text_static(label_play_pause, LV_SYMBOL_PLAY);
    lv_obj_center(label_play_pause);

    lv_obj_set_user_data(btn_play_pause, (void *) label_play_pause);
    lv_obj_add_event_cb(btn_play_pause, btn_play_pause_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* 创建上一首控制按键 */
    lv_obj_t *btn_play_prev = lv_btn_create(music_ui_page);
    lv_obj_set_size(btn_play_prev, 50, 50);
    lv_obj_set_style_radius(btn_play_prev, 25, LV_STATE_DEFAULT);
    lv_obj_clear_flag(btn_play_prev, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_align_to(btn_play_prev, btn_play_pause, LV_ALIGN_OUT_LEFT_MID, -40, 0); 

    lv_obj_add_style(btn_play_prev, &ui_button_styles()->style_focus_no_outline, LV_STATE_FOCUS_KEY);
    lv_obj_add_style(btn_play_prev, &ui_button_styles()->style_focus_no_outline, LV_STATE_FOCUSED);
    lv_obj_add_style(btn_play_prev, &ui_button_styles()->style_bg, LV_STATE_FOCUS_KEY);
    lv_obj_add_style(btn_play_prev, &ui_button_styles()->style_bg, LV_STATE_FOCUSED);
    lv_obj_add_style(btn_play_prev, &ui_button_styles()->style_bg, LV_STATE_DEFAULT);

    lv_obj_t *label_prev = lv_label_create(btn_play_prev);
    lv_label_set_text_static(label_prev, LV_SYMBOL_PREV);
    lv_obj_set_style_text_font(label_prev, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_prev, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_center(label_prev);
    lv_obj_set_user_data(btn_play_prev, (void *) label_prev);
    lv_obj_add_event_cb(btn_play_prev, btn_prev_next_cb, LV_EVENT_CLICKED, (void *) false);

    /* 创建下一首控制按键 */
    lv_obj_t *btn_play_next = lv_btn_create(music_ui_page);
    lv_obj_set_size(btn_play_next, 50, 50);
    lv_obj_set_style_radius(btn_play_next, 25, LV_STATE_DEFAULT);
    lv_obj_clear_flag(btn_play_next, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_align_to(btn_play_next, btn_play_pause, LV_ALIGN_OUT_RIGHT_MID, 40, 0);

    lv_obj_add_style(btn_play_next, &ui_button_styles()->style_focus_no_outline, LV_STATE_FOCUS_KEY);
    lv_obj_add_style(btn_play_next, &ui_button_styles()->style_focus_no_outline, LV_STATE_FOCUSED);
    lv_obj_add_style(btn_play_next, &ui_button_styles()->style_bg, LV_STATE_FOCUS_KEY);
    lv_obj_add_style(btn_play_next, &ui_button_styles()->style_bg, LV_STATE_FOCUSED);
    lv_obj_add_style(btn_play_next, &ui_button_styles()->style_bg, LV_STATE_DEFAULT);

    lv_obj_t *label_next = lv_label_create(btn_play_next);
    lv_label_set_text_static(label_next, LV_SYMBOL_NEXT);
    lv_obj_set_style_text_font(label_next, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_next, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_center(label_next);
    lv_obj_set_user_data(btn_play_next, (void *) label_next);
    lv_obj_add_event_cb(btn_play_next, btn_prev_next_cb, LV_EVENT_CLICKED, (void *) true);

    /* 创建声音调节滑动条 */
    volume_slider = lv_slider_create(music_ui_page);
    lv_obj_set_size(volume_slider, 200, 10);
    lv_obj_set_ext_click_area(volume_slider, 15);
    lv_obj_align(volume_slider, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_slider_set_range(volume_slider, 0, 100);
    lv_slider_set_value(volume_slider, g_sys_volume, LV_ANIM_ON);
    lv_obj_add_event_cb(volume_slider, volume_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *lab_vol_min = lv_label_create(music_ui_page);
    lv_label_set_text_static(lab_vol_min, LV_SYMBOL_VOLUME_MID);
    lv_obj_set_style_text_font(lab_vol_min, &lv_font_montserrat_20, LV_STATE_DEFAULT);
    lv_obj_align_to(lab_vol_min, volume_slider, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    lv_obj_t *lab_vol_max = lv_label_create(music_ui_page);
    lv_label_set_text_static(lab_vol_max, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_style_text_font(lab_vol_max, &lv_font_montserrat_20, LV_STATE_DEFAULT);
    lv_obj_align_to(lab_vol_max, volume_slider, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    /* 创建音乐标题 */
    lv_obj_t *lab_title = lv_label_create(music_ui_page);
    lv_obj_set_user_data(lab_title, (void *) btn_play_pause);
    lv_label_set_text_static(lab_title, "Scanning Files...");
    lv_obj_set_style_text_font(lab_title, &lv_font_montserrat_32, LV_STATE_DEFAULT);
    lv_obj_align(lab_title, LV_ALIGN_TOP_MID, 0, 20);

    /* 创建音乐列表 */ 
    music_list = lv_dropdown_create(music_ui_page);
    lv_dropdown_clear_options(music_list);
    lv_dropdown_set_options_static(music_list, "Scanning...");
    lv_obj_set_style_text_font(music_list, &lv_font_montserrat_20, LV_STATE_ANY);
    lv_obj_set_width(music_list, 200);
    lv_obj_align_to(music_list, lab_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_set_user_data(music_list, (void *) lab_title);
    lv_obj_add_event_cb(music_list, music_list_cb, LV_EVENT_VALUE_CHANGED, NULL);

    build_file_list(music_list);

    lvgl_port_unlock();
}


// AI人动画
LV_IMG_DECLARE(img_bilibili120);

// lv_obj_t *gif_start;

// // AI人出现在屏幕
// void ai_gui_in(void)
// {   
//     lvgl_port_lock(0);
//     gif_start = lv_gif_create(music_ui_page);
//     lv_gif_set_src(gif_start, &img_bilibili120);
//     lv_obj_align(gif_start, LV_ALIGN_CENTER, 0, 0);
//     lvgl_port_unlock();
// }

// // AI人退出屏幕
// void ai_gui_out(void)
// {
//     lvgl_port_lock(0);
//     lv_obj_del(gif_start);
//     lvgl_port_unlock();
// }


// AI播放音乐
void ai_play(void)
{
    int index = file_iterator_get_index(file_iterator_sd);
    ESP_LOGI(TAG, "playing index '%d'", index);
    play_index(index);
    lvgl_port_lock(0);
    lv_label_set_text_static(label_play_pause, LV_SYMBOL_PAUSE); // 显示图标
    lv_obj_add_state(btn_play_pause, LV_STATE_CHECKED); // 按键设置为选中状态
    lvgl_port_unlock();
}

// AI暂停
void ai_pause(void)
{
    audio_player_pause();
    lvgl_port_lock(0);
    lv_label_set_text_static(label_play_pause, LV_SYMBOL_PLAY); // 显示图标
    lv_obj_clear_state(btn_play_pause, LV_STATE_CHECKED); // 清除按键的选中状态
    lvgl_port_unlock();
}

// AI继续
void ai_resume(void)
{
    audio_player_resume(); 
    lvgl_port_lock(0);
    lv_label_set_text_static(label_play_pause, LV_SYMBOL_PAUSE); // 显示图标
    lv_obj_add_state(btn_play_pause, LV_STATE_CHECKED); // 按键设置为选中状态
    lvgl_port_unlock();
}

// AI上一首
void ai_prev_music(void)
{
    // 指向上一首音乐
    file_iterator_prev(file_iterator_sd);
    // 修改当前的音乐名称
    int index = file_iterator_get_index(file_iterator_sd);
    lvgl_port_lock(0);
    lv_dropdown_set_selected(music_list, index);
    lv_obj_t *label_title = (lv_obj_t *) music_list->user_data;
    lv_label_set_text_static(label_title, file_iterator_get_name_from_index(file_iterator_sd, index));
    lvgl_port_unlock();
    // 执行音乐事件
    audio_player_state_t state = audio_player_get_state();
    printf("prev_next_state=%d\n", state);
    if (state == AUDIO_PLAYER_STATE_IDLE) { 
        // Nothing to do
    }else if (state == AUDIO_PLAYER_STATE_PAUSE){ // 如果当前正在暂停歌曲
        ESP_LOGI(TAG, "playing index '%d'", index);
        play_index(index);
        audio_player_pause();
    } else if (state == AUDIO_PLAYER_STATE_PLAYING) { // 如果当前正在播放歌曲
        // 播放歌曲
        ESP_LOGI(TAG, "playing index '%d'", index);
        play_index(index);
    }
}

// AI下一首
void ai_next_music(void)
{
    // 指向上一首音乐
    file_iterator_next(file_iterator_sd);
    // 修改当前的音乐名称
    int index = file_iterator_get_index(file_iterator_sd);
    lvgl_port_lock(0);
    lv_dropdown_set_selected(music_list, index);
    lv_obj_t *label_title = (lv_obj_t *) music_list->user_data;
    lv_label_set_text_static(label_title, file_iterator_get_name_from_index(file_iterator_sd, index));
    lvgl_port_unlock();
    // 执行音乐事件
    audio_player_state_t state = audio_player_get_state();
    printf("prev_next_state=%d\n", state);
    if (state == AUDIO_PLAYER_STATE_IDLE) { 
        // Nothing to do
    }else if (state == AUDIO_PLAYER_STATE_PAUSE){ // 如果当前正在暂停歌曲
        ESP_LOGI(TAG, "playing index '%d'", index);
        play_index(index);
        audio_player_pause();
    } else if (state == AUDIO_PLAYER_STATE_PLAYING) { // 如果当前正在播放歌曲
        // 播放歌曲
        ESP_LOGI(TAG, "playing index '%d'", index);
        play_index(index);
    }
}

// AI声音大一点
void ai_volume_up(void)
{
    if (g_sys_volume < 100){
        g_sys_volume = g_sys_volume + 5;
        if (g_sys_volume > 100){
            g_sys_volume = 100;
        }
        bsp_codec_volume_set(g_sys_volume, NULL); // 设置声音大小
        lvgl_port_lock(0);
        lv_slider_set_value(volume_slider, g_sys_volume, LV_ANIM_ON); // 设置slider
        lvgl_port_unlock();
    }
    ESP_LOGI(TAG, "volume '%d'", g_sys_volume);
}

// AI声音小一点
void ai_volume_down(void)
{
    if (g_sys_volume > 0){
        if (g_sys_volume < 5){
            g_sys_volume = 0;
        }else{
            g_sys_volume = g_sys_volume - 5;
        }
        bsp_codec_volume_set(g_sys_volume, NULL); // 设置声音大小
        lvgl_port_lock(0);
        lv_slider_set_value(volume_slider, g_sys_volume, LV_ANIM_ON); // 设置slider
        lvgl_port_unlock();
    }
    ESP_LOGI(TAG, "volume '%d'", g_sys_volume);
}
