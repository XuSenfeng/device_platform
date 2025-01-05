#include "app_dir.h"
#include "app_ui.h"
#include "lvgl.h"
#include "stdio.h"
#include "user_config.h"
#include "esp_lvgl_port.h"
#include "device_bsp.h"


const char *TAG = "app_dir";
extern lv_obj_t *text_context_windows;
extern lv_style_t main_style;

/// @brief 文件页面返回按钮回调函数
void dir_back_event_cb(lv_event_t *e)
{
    lv_dile_back_data_t *lv_dile_back_data = lv_event_get_user_data(e);
    ESP_LOGI(TAG, " back to file list");
    lvgl_port_lock(0);
    lv_obj_clean(lv_dile_back_data->base);
    lv_obj_add_style(lv_dile_back_data->base, &main_style, 0);
    lv_dir_page(lv_dile_back_data->base);
    lvgl_port_unlock();
    if(lv_dile_back_data->fp != 0)
        fclose(lv_dile_back_data->fp);
}


/// @brief 文件下一页回调
/// @param e 对这一个文件之后的部分进行读取, 把读取的位置显示
void file_nextpage(lv_event_t *e){
    FILE *fp = (FILE *)lv_event_get_user_data(e);
    char buf[FILE_READ_BUF_SIZE + 3] = {0};
    if (fp) {
        if (fread(buf, 1, FILE_READ_BUF_SIZE, fp) != NULL) {
            // 判断最后一个utf编码是否读取完整
            if((buf[FILE_READ_BUF_SIZE - 1] & 0xc0) == 0x80) {
                // 不完整, 重新读取(UTF-8 的第一个字节的高位为1, 说明这个字节是一个utf编码的开始)
                int i = FILE_READ_BUF_SIZE;
                while(fread(buf + i, 1, 1, fp) == 1) {
                    if((buf[i] & 0xc0) != 0x80) {
                        // 读取到了一个完整的utf编码
                        buf[i] = 0;
                        fseek(fp, -1, SEEK_CUR); // 回退一个字节
                        break;
                    }
                    i++;
                }
            }
            lvgl_port_lock(0);
            lv_textarea_set_text(text_context_windows, buf);
            lv_textarea_set_cursor_pos(text_context_windows, 0);
            lvgl_port_unlock();
        }
    }
}

/// @brief 上一页按钮的回调函数, 计算一下上一页的位置, 进行显示
/// @param e 
void file_prepage(lv_event_t *e){
    FILE *fp = (FILE *)lv_event_get_user_data(e);
    // 获取当前位置
    long pos = ftell(fp);
    // 移动到上一页
    long pos_new = pos - FILE_READ_BUF_SIZE;
    if (pos_new < 0) {
        pos_new = 0;
    }
    fseek(fp, pos_new, SEEK_SET);
    
    char buf[FILE_READ_BUF_SIZE + 3] = {0};
    if (fp) {
        if (fread(buf, 1, FILE_READ_BUF_SIZE, fp) != NULL) {
            // 判断最后一个utf编码是否读取完整
            if((buf[FILE_READ_BUF_SIZE - 1] & 0xc0) == 0x80) {
                // 不完整, 重新读取(UTF-8 的第一个字节的高位为1, 说明这个字节是一个utf编码的开始)
                int i = FILE_READ_BUF_SIZE;
                while(fread(buf + i, 1, 1, fp) == 1) {
                    if((buf[i] & 0xc0) != 0x80) {
                        // 读取到了一个完整的utf编码
                        buf[i] = 0;
                        fseek(fp, -1, SEEK_CUR); // 回退一个字节
                        break;
                    }
                    i++;
                }
            }
            lvgl_port_lock(0);
            lv_textarea_set_text(text_context_windows, buf);
            lv_textarea_set_cursor_pos(text_context_windows, 0);
            lvgl_port_unlock();
        }
    }
}
char *canvsa_buf; 
/// @brief 文件链表的回调函数, 选择文件后, 进行文件的读取
/// @param e 
void file_list_event_cb(lv_event_t *e){
    lv_dir_data_t* dir_data = lv_event_get_user_data(e);
    char path[128] = {0};
    int retval = file_iterator_get_full_path_from_index(file_iterator_sd, dir_data->file_idx, path, sizeof(path));
    if (retval == 0) {
        ESP_LOGE(TAG, "unable to retrieve filename");
        return;
    }
    ESP_LOGI(TAG, "file selected: %s", path);
    FILE *fp = fopen(path, "rb");
    if (fp) {
        //返回键
        lv_obj_clean(dir_data->base);
        lv_obj_add_style(dir_data->base, &main_style, 0);
        lv_obj_t *back =lv_btn_create(dir_data->base);
        lv_obj_t *lable = lv_label_create(back);
        lv_label_set_text(lable,LV_SYMBOL_LEFT);
        lv_obj_set_style_text_color(lable,lv_color_hex(0x000000),LV_STATE_DEFAULT);

        lv_obj_set_style_bg_color(back,lv_color_hex(0x7c7c7c),LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(back, LV_OPA_50, 0);
        lv_obj_set_style_shadow_width(back, 0, LV_PART_MAIN);
        lv_dile_back_data_t *dir_data_back = malloc(sizeof(lv_dile_back_data_t));
        dir_data_back->fp = fp;
        dir_data_back->base = dir_data->base;
        lv_obj_add_event_cb(back,dir_back_event_cb,LV_EVENT_CLICKED, dir_data_back);
        lv_obj_set_align(lable, LV_ALIGN_CENTER);
        lv_obj_set_size(back,25,25);
        if(strstr(path, ".pic") != NULL || strstr(path, ".PIC") != NULL){
            lv_obj_t *canvsa = lv_canvas_create(dir_data->base);
            lv_obj_set_size(canvsa, 240, 176);
            lv_obj_align(canvsa, LV_ALIGN_CENTER, 0, 0);
            if(!canvsa_buf){
                canvsa_buf = (char *)malloc(240 * 176 * 2);
            }
            lv_canvas_set_buffer(canvsa, canvsa_buf, 240, 176, LV_IMG_CF_TRUE_COLOR);
            
            lvgl_port_lock(0);
            fread(canvsa_buf, 1, 240 * 176 *2, fp);
            //刷新
            lv_obj_invalidate(canvsa);
            lvgl_port_unlock();

        }else{

            ESP_LOGI(TAG, "Playing '%s'", path);
            text_context_windows = create_text(dir_data->base, file_nextpage, file_prepage, fp);
        }
    } else {
        ESP_LOGE(TAG, "unable to open index %d, filename '%s'", dir_data->file_idx, path);
    }
}
#if USING_CHINESE
LV_FONT_DECLARE(font_alipuhui20);
#endif
/// @brief 文件页面的配置
/// @param obj3 
void lv_dir_page(lv_obj_t *obj3){
    lv_obj_clean(obj3);
    lv_obj_add_style(obj3, &main_style, 0);
    lv_obj_t *file_list=lv_list_create(obj3);
    lv_obj_set_size(file_list,lv_pct(100),lv_pct(85));
    lv_obj_align(file_list,LV_ALIGN_CENTER,0,20);
    lv_obj_set_scrollbar_mode(file_list, LV_SCROLLBAR_MODE_OFF); // 隐藏file_list滚动条
    lv_obj_set_style_bg_opa(file_list, LV_OPA_50, 0);
    
    lv_obj_t *title = lv_label_create(obj3);
    lv_label_set_text(title,"       File");
    lv_obj_set_style_bg_color(title,lv_color_hex(0xFFFFFF),LV_STATE_DEFAULT);
    

    for(size_t i = 0; i<file_iterator_sd->count; i++) {
            // 添加file列表
            const char *file_name = file_iterator_get_name_from_index(file_iterator_sd, i);
            lv_obj_t *btn = lv_list_add_btn(file_list, LV_SYMBOL_FILE, ""); 
            lv_obj_set_style_radius(btn, 0, LV_PART_MAIN);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xbababa), LV_STATE_DEFAULT); 
            lv_obj_set_style_text_color(btn, lv_color_hex(0x0044BB), LV_STATE_DEFAULT); 
            lv_obj_set_style_height(btn, 40, LV_STATE_DEFAULT);
            lv_obj_set_style_width(btn, lv_pct(100), LV_STATE_DEFAULT);
            lv_obj_t * label_file = lv_label_create(btn);
#if USING_CHINESE
            lv_obj_set_style_text_font(label_file, &font_alipuhui20, 0);
#endif
            lv_label_set_text(label_file, file_name);
            lv_dir_data_t *data = malloc(sizeof(lv_dir_data_t));
            data->file_idx = i;
            data->base = obj3;
            lv_obj_add_event_cb(btn, file_list_event_cb, LV_EVENT_CLICKED, data);
        }
}
