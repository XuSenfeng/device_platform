#include "app_ui.h"
#include <stddef.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"

ui_contorl_t ui_control;

void app_ui_init(int register_ui)
{
    ui_control.current = NULL;
    ui_control.head = NULL;

// #if APP_REGISTER & UI_ID_WIFI
    if(register_ui & UI_ID_WIFI){
        ESP_LOGI("app_ui", "register wifi ui");
        //wifi页面注册
        WIFI_UI_REGISTER_CODE
    }
    // /*wifi页面注册*/
    // WIFI_UI_REGISTER_CODE
// #endif

// #if APP_REGISTER & UI_ID_TIMER
    //weather 界面注册
    if(register_ui & UI_ID_TIMER){
        ESP_LOGI("app_ui", "register weather ui");
        WEATHER_TIMER_UI_REGISTER_CODE
    }
    // WEATHER_TIMER_UI_REGISTER_CODE
// #endif

// #if APP_REGISTER & UI_ID_MUSIC
    //music 界面注册
    if(register_ui & UI_ID_MUSIC){
        ESP_LOGI("app_ui", "register music ui");
        MUSIC_UI_REGISTER_CODE
    }
    // MUSIC_UI_REGISTER_CODE
// #endif
}

/// @brief 注册一个页面
/// @param ui 要注册的页面
/// @return 
esp_err_t app_ui_add(ui_config_t *ui)
{
    if(ui_control.head == NULL){
        ui_control.head = ui;
        ui_control.current = ui;
    }else{
        ui->next = ui_control.head;
        ui_control.head = ui;
    }
    return ESP_OK;
}
/// @brief 给一个页面添加一个相邻页面
/// @param ui 要添加的页面
/// @param now_pos_ui 基准页面
/// @param pos 位置, l左 r右 u上 d下
/// @return 
esp_err_t app_ui_add_pos(ui_config_t *ui, ui_config_t *now_pos_ui, char pos)
{
    if(now_pos_ui){
        switch (pos)
        {
        case 'l':
            now_pos_ui->next_l = ui;
            ui->next_r = now_pos_ui;
            break;
        case 'r':
            now_pos_ui->next_r = ui;
            ui->next_l = now_pos_ui;
            break;
        case 'u':
            now_pos_ui->next_u = ui;
            ui->next_d = now_pos_ui;
            break;
        case 'd':
            now_pos_ui->next_d = ui;
            ui->next_u = now_pos_ui;
            break;
        default:
            ESP_LOGE("app_ui", "add ui error pos");
            goto err;
            break;
        }
    }else{
        ESP_LOGE("app_ui", "add ui error now_pos_ui");
        goto err;
    }
    return ESP_OK;
err:
    return ESP_FAIL;
}

/// @brief 显示一个页面
/// @param pos 显示的页面位置, l左 r右 u上 d下, NULL再次显示当前页面
/// @return 
esp_err_t app_ui_show_pos(char pos)
{
    if(!pos){
        ui_control.current->init(ui_control.current->user_data);
        return ESP_OK;
    }
    ui_config_t *ui = ui_control.current;
    switch (pos)
    {
    case 'l':
        if(ui_control.current->next_l == NULL){
            ESP_LOGE("app_ui", "show ui error pos no ui");
            goto err;
        }
        ui_control.current = ui->next_l;
        break;
    case 'r':
        if(ui_control.current->next_r == NULL){
            ESP_LOGE("app_ui", "show ui error pos no ui");
            goto err;
        }
        ui_control.current = ui->next_r;
        break;
    case 'u':
        if(ui_control.current->next_u == NULL){
            ESP_LOGE("app_ui", "show ui error pos no ui");
            goto err;
        }
        ui_control.current = ui->next_u;
        break;
    case 'd':
        if(ui_control.current->next_d == NULL){
            ESP_LOGE("app_ui", "show ui error pos no ui");
            goto err;
        }
        ui_control.current = ui->next_d;
        break;
    default:
        ESP_LOGE("app_ui", "show ui error pos");
        goto err;
        break;
    }
    if(ui->deinit){
        ui->deinit();
    }
    ui_control.current->init(ui_control.current->user_data);
    return ESP_OK;
err:
    return ESP_FAIL;
}
/// @brief 获取当前页面的id
/// @param id 
/// @return 成功ESP_OK, 失败ESP_FAIL
esp_err_t app_get_ui_id(uint32_t *id)
{
    if(ui_control.current){
        *id = ui_control.current->ID;
        return ESP_OK;
    }
    return ESP_FAIL;
}
/// @brief 通过页面的ID获取一个页面
/// @param id 页面ID
/// @param ui 返回的结构体
/// @return 成功ESP_OK, 失败ESP_FAIL
esp_err_t app_get_ui_by_id(uint32_t id, ui_config_t **ui)
{
    ui_config_t *ui_temp = ui_control.head;
    while (ui_temp)
    {
        if(ui_temp->ID == id){
            *ui = ui_temp;
            return ESP_OK;
        }
        ui_temp = ui_temp->next;
    }
    return ESP_FAIL;
}
/// @brief 通过页面的ID进行显示
/// @param id 页面的ID
/// @return 成功ESP_OK, 失败ESP_FAIL
esp_err_t app_ui_show_id(uint32_t id)
{
    ui_config_t *ui = ui_control.head;
    while (ui)
    {
        if(ui->ID == id){
            if(ui_control.current && ui_control.current != ui){
                if(ui_control.current->deinit){
                    ui_control.current->deinit();
                }
            }
            ESP_LOGI("app_ui", "show ui id %d", (int)id);
            ui_control.current = ui;
            ui->init(ui->user_data);
            return ESP_OK;
        }
        ui = ui->next;
    }
    ESP_LOGI("app_ui", "show ui id %d error", (int)id);
    return ESP_FAIL;
}