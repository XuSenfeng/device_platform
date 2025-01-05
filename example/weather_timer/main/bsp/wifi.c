/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-12 23:31:21
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-11-24 23:06:48
 */
#include "wifi.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "string.h"
#include "cJSON.h"
#include "ttv.h"
#include "device_manage.h"
static char *TAG = "WIFI";
// extern SemaphoreHandle_t wifi_mutex;
void wifi_cb(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    uint8_t connect_num = 0;
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
        //wifi启动成功
        esp_wifi_connect();
    }
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
        //wifi连接失败
        while(1){
            connect_num ++;
            if(connect_num <= 5){
                esp_wifi_connect();
            }else{
                ESP_LOGE(TAG, "connected failed");
            }
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        ESP_LOGI("main", "connected successed");
        ip_event_got_ip_t *info = (ip_event_got_ip_t *)event_data;
        printf("get sta ip "IPSTR"\n", IP2STR(&info->ip_info.ip));
        // xSemaphoreGive(wifi_mutex);
    }


}

void wifi_sta_init(){
    // wifi_mutex = xSemaphoreCreateBinary();
    nvs_flash_init();
    esp_event_loop_create_default();
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, wifi_cb, NULL);
    //连接失败的回调
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_cb, NULL);
    //连接成功的事件
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_cb, NULL);
    //初始化网络接口
    esp_netif_init();
    

    //建立sta接口
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();

    //初始化Wifi底层配置
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_cfg);

    //设置wifi
    esp_wifi_set_mode(WIFI_MODE_STA);

    //配置STA的相关参数, 这两个是必须设置的
    wifi_config_t sta_cfg = {
        .sta = {
            .ssid = "jiao",
            .password = "1234567890",
        }
    };
    esp_wifi_set_config(WIFI_IF_STA, &sta_cfg);

    esp_wifi_start();
}



