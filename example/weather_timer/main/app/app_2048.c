#include "app_2048.h"
#include "lvgl.h"
#include "time.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
uint8_t p=0,flag=0,cnt=2;
uint8_t sz[WIDTH][WIDTH] = { 0 };
lv_obj_t* sz_del[WIDTH][WIDTH]={0};
lv_obj_t* p_score = NULL;
uint16_t score=0;
lv_obj_t* end_main = NULL;

lv_obj_t *game_page = NULL;

void game_main(lv_obj_t * base) {
    p=0;
    flag=0;
    cnt=2;
    memset(sz, 0, sizeof(sz));
    memset(sz_del, 0, sizeof(sz_del));
    p_score = NULL;
    score = 0;
    end_main = NULL;
    game_page = base;
    lv_obj_t* bg = lv_obj_create(game_page);
    lv_obj_set_style_bg_color(game_page,lv_color_hex(0xf8f8f0), LV_STATE_DEFAULT);
    lv_obj_set_size(bg, 200, 200);
    lv_obj_align(bg, LV_ALIGN_RIGHT_MID,-30,0);
    lv_obj_set_style_bg_color(bg, lv_color_hex(0xb2a296), LV_STATE_DEFAULT);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(bg, event_cb, LV_EVENT_GESTURE, bg);
 
    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_font(&font_style, &lv_font_montserrat_14);
    lv_obj_t* label_2048 = lv_label_create(game_page);
    lv_obj_set_size(label_2048,80, 50);
    lv_label_set_recolor(label_2048, true);
    lv_label_set_text(label_2048, "#655949 2048#");
    lv_obj_add_style(label_2048, &font_style, LV_STATE_DEFAULT);
    lv_obj_align(label_2048, LV_ALIGN_TOP_LEFT,25,20);
 
    lv_obj_t* btn = lv_obj_create(game_page);
    lv_obj_t* label1 = lv_label_create(btn);
    lv_obj_set_size(btn, 50, 50);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xb8afa0), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(btn,0, LV_STATE_DEFAULT);
    lv_label_set_recolor(label1, true);
    lv_label_set_text(label1, "#f0e4d8 New\n#f0e4d8 Game#");
    lv_obj_align(label1, LV_ALIGN_TOP_LEFT, -10, -10);
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, bg);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 22, -20);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
 
    rands(2, 1);
    show_mb(bg);
}
 
bool detect_end() {
    const short a[4][2] = { {0,1},{0,-1},{1,0},{-1,0} };
    for (short i = 0; i < WIDTH; i++) {
        for (short j = 0; j < WIDTH; j++) {
            for (short z = 0; z < 4; z++) {
                if (i + a[z][0] < WIDTH && j + a[z][1] < WIDTH && i + a[z][0] >= 0 && j + a[z][1] >= 0) {
                    if (sz[i][j] == sz[i + a[z][0]][j + a[z][1]]) {
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}
 
 
void event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* bg = (lv_obj_t*)e->user_data;
    if (code == LV_EVENT_GESTURE) {
        ESP_LOGI("event_cb", "gesture event");
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_TOP) {
            bg_hd(UP, bg);
        }
        else if (dir == LV_DIR_BOTTOM) {
            bg_hd(DOWN, bg);
        }
        else if (dir == LV_DIR_LEFT) {
            bg_hd(LEFT, bg);
        }
        else if (dir == LV_DIR_RIGHT) {
            bg_hd(RIGHT, bg);
        }
    }
    if (code == LV_EVENT_CLICKED) {
        game_init(bg);
            if(end_main!=NULL){
                lv_obj_del(end_main);
                end_main=NULL;
                lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志
                lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度
            }
		}
}
 
void game_init(lv_obj_t* bg) {
 
    for (uint8_t i = 0; i < WIDTH; i++) {
        for (uint8_t j = 0; j < WIDTH; j++) {
            sz[i][j] = 0;
        }
    }
    score = 0;
    rands(2, score);
    cnt = 2;
    show_mb(bg);
}
 
void show_mb(lv_obj_t* bg) {
    uint8_t margin_x = 0, margin_y = 0;
 
    for (uint8_t i = 0; i < WIDTH; i++) {
        margin_x = 0;
        for (uint8_t j = 0; j < WIDTH; j++) {
            //printf("%d ", sz[i][j]);
            show_color_block(sz[i][j], margin_x + j * BLOCKH, margin_y + i * BLOCKH, bg, i, j);
            margin_x += 5;
        }
        margin_y += 5;
        //printf("\n");
    }
}

void show_color_block(uint8_t n, uint16_t px, uint16_t py, lv_obj_t* bg, uint8_t i, uint8_t j) {
    lvgl_port_lock(0);
    if (sz_del[i][j] != NULL)lv_obj_del(sz_del[i][j]);
    if(p_score!=NULL)lv_obj_del(p_score);
    lv_obj_t* block = lv_obj_create(bg);
    sz_del[i][j] = block;
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(block, 40, 40);
    lv_obj_set_style_border_width(block, LV_BORDER_SIDE_NONE, LV_STATE_DEFAULT);
 
 
    lv_obj_t* score_block = lv_obj_create(game_page);
    lv_obj_set_size(score_block, 50, 50);
    lv_obj_align(score_block,LV_ALIGN_BOTTOM_LEFT,22,-30);
    lv_obj_set_style_bg_color(score_block,lv_color_hex(0xb8afa0), LV_STATE_DEFAULT);
 
 
    lv_obj_t* score_l = lv_label_create(score_block);
    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_font(&font_style, &lv_font_montserrat_14);
    lv_obj_add_style(score_l, &font_style, LV_STATE_DEFAULT);
    p_score = score_block;
    lv_label_set_recolor(score_l, true);
    lv_label_set_text(score_l, "#f0e4d8 SCORE#");
    lv_obj_clear_flag(score_block, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(score_l, LV_ALIGN_TOP_MID, 0, -10);
 
    lv_obj_t* score_s = lv_label_create(score_block);
    static lv_style_t font_style1;
    lv_style_init(&font_style1);
    lv_style_set_text_font(&font_style1, &lv_font_montserrat_14);
    lv_label_set_recolor(score_s, true);
    lv_label_set_text_fmt(score_s, "#ffffff %d#",score);
    lv_obj_add_style(score_s, &font_style1, LV_STATE_DEFAULT);
    lv_obj_align(score_s, LV_ALIGN_BOTTOM_MID, 0, 5);
    if (n != 0) {
        lv_obj_t* num_l = lv_label_create(block);
        lv_obj_clear_flag(block, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_width(num_l, LV_BORDER_SIDE_NONE, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(num_l, "%d", (uint16_t)pow(2, n));
        lv_obj_center(num_l);
    }
    switch (n) {
    case 0:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xc6b8ab), LV_STATE_DEFAULT); break;
    case 1:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xece2d8), LV_STATE_DEFAULT); break;
    case 2:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xeadec6), LV_STATE_DEFAULT); break;
    case 3:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xf0af76), LV_STATE_DEFAULT); break;
    case 4:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xec8d54), LV_STATE_DEFAULT); break;
    case 5:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xf67c5f), LV_STATE_DEFAULT); break;
    case 6:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xea5937), LV_STATE_DEFAULT); break;
    case 7:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xf2d76b), LV_STATE_DEFAULT); break;
    case 8:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xf1d04b), LV_STATE_DEFAULT); break;
    case 9:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xe4c02a), LV_STATE_DEFAULT); break;
    case 10:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xdcb82a), LV_STATE_DEFAULT); break;
    case 11:
        lv_obj_set_style_bg_color(block, lv_color_hex(0xd4b02a), LV_STATE_DEFAULT); break;
    }
    lv_obj_align(block, LV_ALIGN_TOP_LEFT, px, py);
    lvgl_port_unlock();
}
 
void rands(int n, uint16_t s) {
    uint8_t x, y;
    time_t t;
    while(n--){
        uint8_t flag_buf[WIDTH * WIDTH];
        memset(flag_buf, 0, sizeof(flag_buf));
        int k = 0;
        for(int i = 0;i<WIDTH;i++){
            for(int j = 0;j<WIDTH;j++){
                if(sz[i][j] == 0)flag_buf[k++] = i*WIDTH + j;
            }
        }
        srand((unsigned) time(&t));
        x = rand() % k;
        if (s < LIMIT1) {
            sz[flag_buf[x] / WIDTH][flag_buf[x] % WIDTH] = 1;
        }
        else if (s >= LIMIT1 && s < LIMIT2) {
            srand((unsigned) time(&t));
            uint8_t c = rand() % 4;
            if (c == 0) {
                sz[flag_buf[x] / WIDTH][flag_buf[x] % WIDTH] = 1;
            }
            else{
                sz[flag_buf[x] / WIDTH][flag_buf[x] % WIDTH] = 2;
            }
        }
        else if (s >= LIMIT2 && s < LIMIT3) {
            srand((unsigned) time(&t));
            uint8_t c = rand() % 3;
            if (c == 0) {
                sz[flag_buf[x] / WIDTH][flag_buf[x] % WIDTH] = 1;
            }
            else {
                sz[flag_buf[x] / WIDTH][flag_buf[x] % WIDTH] = 2;
            }
        }
        else if (s >= LIMIT3) {
            srand((unsigned) time(&t));
            uint8_t c = rand() % 2;
            if (c == 0) {
                sz[flag_buf[x] / WIDTH][flag_buf[x] % WIDTH] = 1;
            }
            else {
                sz[flag_buf[x] / WIDTH][flag_buf[x] % WIDTH] = 2;
            }
        }
    }
    // sz[flag_buf[x] / WIDTH][flag_buf[x] % WIDTH] = 1;
#if 0
    while (n > 0) {
        srand((unsigned) time(&t));
        x = rand() % WIDTH;
        srand((unsigned) time(&t));
        y = rand() % WIDTH;
        if (sz[x][y] == 0) {
            if (s < LIMIT1) {
                sz[x][y] = 1;
            }
            else if (s >= LIMIT1 && s < LIMIT2) {
                srand((unsigned) time(&t));
                uint8_t c = rand() % 4;
                if (c == 0) {
                    sz[x][y] = 1;
                }
                else{
                    sz[x][y] = 2;
                }
            }
            else if (s >= LIMIT2 && s < LIMIT3) {
                srand((unsigned) time(&t));
                uint8_t c = rand() % 3;
                if (c == 0) {
                      sz[x][y] = 1;
                }
                else {
                      sz[x][y] = 2;
                }
            }
            else if (s >= LIMIT3) {
                srand((unsigned) time(&t));
                uint8_t c = rand() % 2;
                if (c == 0) {
                    sz[x][y] = 1;
                }
                else {
                    sz[x][y] = 2;
                }
            }
            n -= 1;
        }
    }
#endif
}
void bg_hd(uint8_t toward, lv_obj_t* bg) {
    uint8_t flag, flag_t = 0;     //不连续合并
    if (toward == UP) {
        for (short j = 0; j < WIDTH; j++) {
            flag = 1;
            for (short i = 1; i < WIDTH; i++) {
                for (short z = i; z > 0; z--) {
                    if (sz[z - 1][j] == sz[z][j] && sz[z][j] != 0 && flag) {
                        sz[z - 1][j] += 1;
                        cnt--;
                        score += (uint16_t)pow(2, sz[z - 1][j]);
                        sz[z][j] = 0;
                        flag = 0;
                        flag_t = 1;
                        if (flag_t != 1)flag_t = 1;
                    }
                    else if (sz[z - 1][j] == 0 && sz[z][j] != 0) {
                        sz[z - 1][j] = sz[z][j];
                        sz[z][j] = 0;
                        if (flag_t != 1)flag_t = 1;
                    }
                }
            }
        }
    }
    if (toward == DOWN) {
        for (short j = 0; j < WIDTH; j++) {
            flag = 1;
            for (short i = WIDTH - 2; i >= 0; i--) {
                for (short z = i; z < WIDTH - 1; z++) {
                    if (sz[z + 1][j] == sz[z][j] && sz[z][j] != 0 && flag) {
                        sz[z + 1][j] += 1;
                        cnt--;
                        score += (uint16_t)pow(2, sz[z + 1][j]);
                        sz[z][j] = 0;
                        flag = 0;
                        if (flag_t != 1)flag_t = 1;
                    }
                    else if (sz[z + 1][j] == 0 && sz[z][j] != 0) {
                        sz[z + 1][j] = sz[z][j];
                        sz[z][j] = 0;
                        if (flag_t != 1)flag_t = 1;
                    }
                }
            }
        }
    }
    if (toward == LEFT) {
        for (short j = 0; j < WIDTH; j++) {
            flag = 1;
            for (short i = 1; i < WIDTH; i++) {
                for (short z = i; z > 0; z--) {
                    if (sz[j][z - 1] == sz[j][z] && sz[j][z] != 0 && flag) {
                        sz[j][z - 1] += 1;
                        cnt--;
                        score += (uint16_t)pow(2, sz[j][z - 1]);
                        sz[j][z] = 0;
                        flag = 0;
                        if (flag_t != 1)flag_t = 1;
                    }
                    else if (sz[j][z - 1] == 0 && sz[j][z] != 0) {
                        sz[j][z - 1] = sz[j][z];
                        sz[j][z] = 0;
                        if (flag_t != 1)flag_t = 1;
                    }
                }
            }
        }
    }
    if (toward == RIGHT) {
        for (short j = 0; j < WIDTH; j++) {
            flag = 1;
            for (short i = WIDTH - 2; i >= 0; i--) {
                for (short z = i; z < WIDTH - 1; z++) {
                    if (sz[j][z + 1] == sz[j][z] && sz[j][z] != 0 && flag) {
                        sz[j][z + 1] += 1;
                        cnt--;
                        score += (uint16_t)pow(2, sz[j][z + 1]);
                        sz[j][z] = 0;
                        flag = 0;
                        if (flag_t != 1)flag_t = 1;
                    }
                    else if (sz[j][z + 1] == 0 && sz[j][z] != 0) {
                        sz[j][z + 1] = sz[j][z];
                        sz[j][z] = 0;
                        if (flag_t != 1)flag_t = 1;
                    }
                }
            }
        }
    }
    if (flag_t == 1) {
        rands(NEW_NUM, score);
        cnt += NEW_NUM;
        show_mb(bg);
    }
    if (cnt == WIDTH * WIDTH) {
        if (detect_end()) {						
            end_ami(bg);
        }
    }
}
 
 
void end_ami(lv_obj_t* bg) {
    lvgl_port_lock(0);
    lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);
    end_main = lv_obj_create(lv_layer_top());
    lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_50, 0);                          // 设置对象透明度
    lv_obj_set_style_bg_color(lv_layer_top(), lv_palette_main(LV_PALETTE_GREY), 0); //灰色背景
    lv_obj_set_style_bg_color(end_main, lv_color_hex(0xb2a296), LV_STATE_DEFAULT);
    lv_obj_set_size(end_main, 140, 150);
    lv_obj_clear_flag(end_main, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_align(end_main, LV_ALIGN_CENTER);
    lv_obj_t* score_l = lv_label_create(end_main);
    lv_label_set_recolor(score_l, true);
    lv_label_set_text(score_l, "#000000 SCORE#");
    lv_obj_align(score_l, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_t* score_s = lv_label_create(end_main);
    static lv_style_t font_style1;
    lv_style_init(&font_style1);
    lv_label_set_recolor(score_s, true);
    lv_label_set_text_fmt(score_s, "#000000 %d#",score);
    lv_obj_add_style(score_s, &font_style1, LV_STATE_DEFAULT);
    lv_obj_align(score_s, LV_ALIGN_BOTTOM_MID, 0, -70);
    // 创建重新开始按钮
    lv_obj_t* regame = lv_obj_create(end_main);
    lv_obj_set_size(regame, 100, 40);
    lv_obj_set_style_bg_color(regame,lv_color_hex(0x1E90FF), LV_STATE_DEFAULT);
    lv_obj_align(regame,LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_t* retry = lv_label_create(regame);
    lv_label_set_recolor(retry, true);
    lv_label_set_text(retry, "#000000 RETRY#");
    lv_obj_clear_flag(regame, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(retry, LV_ALIGN_CENTER,0,0);
    lv_obj_add_event_cb(regame, event_cb, LV_EVENT_CLICKED, bg);
    lv_obj_clear_flag(regame, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lvgl_port_unlock();
}
 