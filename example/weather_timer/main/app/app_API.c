/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-16 11:07:51
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-04 16:07:20
 */
#include "app_API.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"
#include "device_manage.h"
#include "ttv.h"
#include "freertos/event_groups.h"
#include "esp_sntp.h"

#include "time.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "esp_netif_sntp.h"
#include "zlib.h"
#include "esp_tls.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "user_config.h"
static char *TAG = "API";

#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1
#define WIFI_START_BIT        BIT2
#define WIFI_GET_SNTP_BIT     BIT3
#define CALENT_CAN_BE_USED_BIT    BIT4
#define WIFI_BUFFER_LENGTH          (8 * 1024)

#define MAX_HTTP_OUTPUT_BUFFER WIFI_BUFFER_LENGTH
char wifi_read_buf[WIFI_BUFFER_LENGTH];
int read_pos = 0, cannot_read = 0;

int buf_type = 0;



//事件回调
static esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR://错误事件
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED://连接成功事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT://发送头事件
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER://接收头事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA://接收数据事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // if (esp_http_client_is_chunked_response(evt->client))
            // {
            if(read_pos + evt->data_len > WIFI_BUFFER_LENGTH){
                ESP_LOGE(TAG, "buffer overflow");
                cannot_read = 1;
                break;
            }
            memcpy(wifi_read_buf + read_pos, evt->data, evt->data_len);
            read_pos += evt->data_len;
            // }else{
            //     if(read_pos + evt->data_len > WIFI_BUFFER_LENGTH){
            //         ESP_LOGE(TAG, "buffer overflow");
            //         cannot_read = 1;
            //         break;
            //     }
            //     memcpy(wifi_read_buf + read_pos, evt->data, evt->data_len);
            //     read_pos += evt->data_len;
            // }
            break;
        case HTTP_EVENT_ON_FINISH://会话完成事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED://断开事件
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
        default:
            break;
        
    }
    return ESP_OK;
}
extern EventGroupHandle_t s_wifi_event_group;
extern QueueHandle_t   s_client_queue;

void get_client(void){
    ESP_LOGI(TAG, "get client");
    xQueueSemaphoreTake(s_client_queue, portMAX_DELAY);
    ESP_LOGI(TAG, "get client success");
}

void relesae_client(void){
    xSemaphoreGive(s_client_queue);
}
extern lv_obj_t *text_context_windows;
/*获取一个故事*/
void get_http_data_story_task(void* parameter){
    example_wifi_reconnect();
    int i = 0;
    while(get_target_url(STORY_API_URL) != ESP_OK && i < 3){
        vTaskDelay(100 / portTICK_PERIOD_MS);
        i++;
    }
    if(i >= 3){
        if(text_context_windows != NULL){
            lvgl_port_lock(0);
            lv_textarea_set_text(text_context_windows, "获取故事失败");
            lvgl_port_unlock();
        }
        vTaskDelete(NULL);
    }
    buf_type = BUF_TYPE_STORY;
    deal_story(1, 1);
    example_wifi_shutdown();
    vTaskDelete(NULL);
}

void get_http_data_birthday_character_task(void* parameter){
    example_wifi_reconnect();
    int i = 0;
    const char *data = (const char *)parameter;
    char temp_buf[180];
    sprintf(temp_buf, "%s%s", BIRTHDAY_CHARACTRE_API_URL, data);
    ESP_LOGI("API", "url: %s", temp_buf);
    while(get_target_url(temp_buf) != ESP_OK && i < 3){
        vTaskDelay(100 / portTICK_PERIOD_MS);
        i++;
    }
    if(i >= 3){
        if(text_context_windows != NULL){
            lvgl_port_lock(0);
            lv_textarea_set_text(text_context_windows, "获取性格失败");
            lvgl_port_unlock();
        }
        vTaskDelete(NULL);
    }
    buf_type = BUF_TYPE_BIRTHDAY_CHARACTRE;
    deal_birthday_character(1, 1);
    example_wifi_shutdown();
    vTaskDelete(NULL);
}

esp_err_t get_target_url(char *target_url){
    xEventGroupWaitBits(s_wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);
    get_client();
    read_pos = 0;
    memset(wifi_read_buf, 0, WIFI_BUFFER_LENGTH * sizeof(char));
    cannot_read = 0;
    //http client配置
    esp_http_client_config_t config = {
        .url = target_url,
        .method = HTTP_METHOD_GET,
        .event_handler = _http_event_handle,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    ESP_LOGI(TAG, "start get data");
	esp_http_client_handle_t client = esp_http_client_init(&config);//初始化配置
    ESP_LOGD(TAG, "esp_http_client_init");
	esp_err_t err = esp_http_client_perform(client);//执行请求
	esp_http_client_cleanup(client);//断开并释放资源
    relesae_client();
	if(err == ESP_OK && cannot_read == 0)
	{
        return ESP_OK;
	}
    return ESP_FAIL;
}

void deal_story(int new_story, int next_page){
    if(buf_type != BUF_TYPE_STORY){
        return;
    }
    static int story_pos = 0;
    if(new_story == 1){
        story_pos = 0;
    }

    if(next_page == 0){
        story_pos -= FILE_READ_BUF_SIZE * 2;
        if(story_pos < 0){
            story_pos = 0;
        }
    }
    cJSON *cjson_outmost_item = NULL;
    cjson_outmost_item = cJSON_Parse(wifi_read_buf);
    if(cjson_outmost_item == NULL){
        ESP_LOGE(TAG, "parse json failed");
        goto err;
    }
    //读取最外面的数据
    cJSON *cjson_msg_item = cJSON_GetObjectItem(cjson_outmost_item, "msg");
    if(strcmp(cjson_msg_item->valuestring, "success") == 0)
    {
        ESP_LOGI(TAG, "get data success");
    }else{
        ESP_LOGE(TAG, "get data failed");
        goto err;
    }
    // 解析实际的数据
    cJSON *cjson_result_item = cJSON_GetObjectItem(cjson_outmost_item, "result");
    //获取到数据数组list
    int skill_array_size = 0;
    cJSON *cjson_list_item = cJSON_GetObjectItem(cjson_result_item, "list");
    skill_array_size = cJSON_GetArraySize(cjson_list_item);
    ESP_LOGI(TAG, "get array size: %d", skill_array_size);
    for(int i = 0; i < skill_array_size; i++)
    {
        cJSON *cjson_skill_item = cJSON_GetArrayItem(cjson_list_item, i);
        cJSON *cjson_title_item = cJSON_GetObjectItem(cjson_skill_item, "title");
        cJSON *cjson_content_item = cJSON_GetObjectItem(cjson_skill_item, "content");
        char *temp_buf = (char *)malloc(FILE_READ_BUF_SIZE + 3);
        int left_length = strlen(cjson_content_item->valuestring) - story_pos;
        int copy_long =  left_length > FILE_READ_BUF_SIZE ? FILE_READ_BUF_SIZE : left_length; // 读取的长度
        memcpy(temp_buf, cjson_content_item->valuestring + story_pos, copy_long);

        // 判断最后一个utf编码是否读取完整
        if(copy_long == FILE_READ_BUF_SIZE && (temp_buf[FILE_READ_BUF_SIZE - 1] & 0xc0) == 0x80) {
            // 不完整, 重新读取(UTF-8 的第一个字节的高位为1, 说明这个字节是一个utf编码的开始)
            int i = FILE_READ_BUF_SIZE;
            while((temp_buf[i] = cjson_content_item->valuestring[story_pos + i]) != 0){
                if((temp_buf[i] & 0xc0) != 0x80) {
                    // 读取到了一个完整的utf编码
                    temp_buf[i] = 0;
                    story_pos += i;
                    break;
                }
                i++;
            }
        }else{
            temp_buf[copy_long] = 0;
            story_pos += copy_long;
        }
        if(text_context_windows != NULL){
            lvgl_port_lock(0);
            lv_textarea_set_text(text_context_windows, temp_buf);
            lvgl_port_unlock();
        }
    }
err:
    cJSON_Delete(cjson_outmost_item);
}
void deal_birthday_character(int new_story, int next_page){
    if(buf_type != BUF_TYPE_BIRTHDAY_CHARACTRE){
        return;
    }
    static int story_pos = 0;
    if(new_story == 1){
        story_pos = 0;
    }

    if(next_page == 0){
        story_pos -= FILE_READ_BUF_SIZE * 2;
        if(story_pos < 0){
            story_pos = 0;
        }
    }
    cJSON *cjson_outmost_item = NULL;
    cjson_outmost_item = cJSON_Parse(wifi_read_buf);
    if(cjson_outmost_item == NULL){
        ESP_LOGE(TAG, "parse json failed");
        goto err;
    }
    //读取最外面的数据
    cJSON *cjson_msg_item = cJSON_GetObjectItem(cjson_outmost_item, "msg");
    if(strcmp(cjson_msg_item->valuestring, "success") == 0)
    {
        ESP_LOGI(TAG, "get data success");
    }else{
        ESP_LOGE(TAG, "get data failed");
        goto err;
    }
    // 解析实际的数据
    cJSON *cjson_result_item = cJSON_GetObjectItem(cjson_outmost_item, "result");

    cJSON *cjson_title_item = cJSON_GetObjectItem(cjson_result_item, "title");
    cJSON *cjson_content_item = cJSON_GetObjectItem(cjson_result_item, "content");
    char *temp_buf = (char *)malloc(FILE_READ_BUF_SIZE + 3);
    int left_length = strlen(cjson_content_item->valuestring) - story_pos;
    int copy_long =  left_length > FILE_READ_BUF_SIZE ? FILE_READ_BUF_SIZE : left_length; // 读取的长度
    memcpy(temp_buf, cjson_content_item->valuestring + story_pos, copy_long);

    // 判断最后一个utf编码是否读取完整
    if(copy_long == FILE_READ_BUF_SIZE && (temp_buf[FILE_READ_BUF_SIZE - 1] & 0xc0) == 0x80) {
        // 不完整, 重新读取(UTF-8 的第一个字节的高位为1, 说明这个字节是一个utf编码的开始)
        int i = FILE_READ_BUF_SIZE;
        while((temp_buf[i] = cjson_content_item->valuestring[story_pos + i]) != 0){
            if((temp_buf[i] & 0xc0) != 0x80) {
                // 读取到了一个完整的utf编码
                temp_buf[i] = 0;
                story_pos += i;
                break;
            }
            i++;
        }
    }else{
        temp_buf[copy_long] = 0;
        story_pos += copy_long;
    }
    if(text_context_windows != NULL){
        lvgl_port_lock(0);
        lv_textarea_set_text(text_context_windows, temp_buf);
        lvgl_port_unlock();
    }
    
err:
    cJSON_Delete(cjson_outmost_item);
}

void deal_constellation(void){
    if(buf_type != BUF_TYPE_CONSTELLATION){
        return;
    }
    cJSON *cjson_outmost_item = NULL;
    cjson_outmost_item = cJSON_Parse(wifi_read_buf);
    if(cjson_outmost_item == NULL){
        ESP_LOGE(TAG, "parse json failed");
        goto err;
    }
    //读取最外面的数据
    cJSON *cjson_msg_item = cJSON_GetObjectItem(cjson_outmost_item, "msg");
    if(strcmp(cjson_msg_item->valuestring, "success") == 0)
    {
        ESP_LOGI(TAG, "get data success");
    }else{
        ESP_LOGE(TAG, "get data failed");
        goto err;
    }
    // 解析实际的数据
    cJSON *cjson_result_item = cJSON_GetObjectItem(cjson_outmost_item, "result");
    //获取到数据数组list
    int skill_array_size = 0;
    cJSON *cjson_list_item = cJSON_GetObjectItem(cjson_result_item, "list");
    skill_array_size = cJSON_GetArraySize(cjson_list_item);
    ESP_LOGI(TAG, "get array size: %d", skill_array_size);
    for(int i = 0; i < skill_array_size; i++)
    {
        cJSON *cjson_skill_item = cJSON_GetArrayItem(cjson_list_item, i);
        cJSON *cjson_type_item = cJSON_GetObjectItem(cjson_skill_item, "type");
        cJSON *cjson_content_item = cJSON_GetObjectItem(cjson_skill_item, "content");
        ESP_LOGI(TAG, "type: %s", cjson_type_item->valuestring);
        ESP_LOGI(TAG, "content: %s", cjson_content_item->valuestring);
        if(text_context_windows != NULL){
            lvgl_port_lock(0);
            lv_textarea_add_text(text_context_windows, cjson_type_item->valuestring);
            lv_textarea_add_text(text_context_windows, cjson_content_item->valuestring);
            lv_textarea_add_char(text_context_windows, '\n');
            lvgl_port_unlock();
        }
    }
err:
    cJSON_Delete(cjson_outmost_item);
}

void deal_riddle(int new_story, int next_page){
    if(buf_type != BUF_TYPE_RIDDLE){
        return;
    }
    static int riddle_pos = 0;
    if(new_story == 1){
        riddle_pos = 0;
    }

    if(next_page == 0){
        riddle_pos -= 2;
        if(riddle_pos < 0){
            riddle_pos = 0;
        }
    }
    cJSON *cjson_outmost_item = NULL;
    cjson_outmost_item = cJSON_Parse(wifi_read_buf);
    if(cjson_outmost_item == NULL){
        ESP_LOGE(TAG, "parse json failed");
        goto err;
    }
    //读取最外面的数据
    cJSON *cjson_msg_item = cJSON_GetObjectItem(cjson_outmost_item, "msg");
    if(strcmp(cjson_msg_item->valuestring, "success") == 0)
    {
        ESP_LOGI(TAG, "get data success");
    }else{
        ESP_LOGE(TAG, "get data failed");
        goto err;
    }
    // 解析实际的数据
    cJSON *cjson_result_item = cJSON_GetObjectItem(cjson_outmost_item, "result");
    //获取到数据数组list
    int skill_array_size = 0;
    cJSON *cjson_list_item = cJSON_GetObjectItem(cjson_result_item, "list");
    skill_array_size = cJSON_GetArraySize(cjson_list_item);
    ESP_LOGI(TAG, "get array size: %d", skill_array_size);
    if(riddle_pos >= skill_array_size *2){
        riddle_pos = skill_array_size *2 - 1;
    }

    cJSON *cjson_skill_item = cJSON_GetArrayItem(cjson_list_item, riddle_pos / 2);
    char *temp_buf = (char *)malloc(FILE_READ_BUF_SIZE);
    if(riddle_pos % 2 == 0){
        cJSON *cjson_title_item = cJSON_GetObjectItem(cjson_skill_item, "quest");
        ESP_LOGI(TAG, "title: %s", cjson_title_item->valuestring);
        snprintf(temp_buf, FILE_READ_BUF_SIZE, "question:%s", cjson_title_item->valuestring);
    }else{
        cJSON *cjson_content_item = cJSON_GetObjectItem(cjson_skill_item, "result");
        ESP_LOGI(TAG, "content: %s", cjson_content_item->valuestring);
        snprintf(temp_buf, FILE_READ_BUF_SIZE, "question:%s", cjson_content_item->valuestring);
    }

    if(text_context_windows != NULL){
        lvgl_port_lock(0);
        lv_textarea_set_text(text_context_windows, temp_buf);
        lvgl_port_unlock();
    }
    free(temp_buf);
    
    riddle_pos++;

err:
    cJSON_Delete(cjson_outmost_item);
}
void deal_poisonous_chicken_soup(void){
    if(buf_type != BUF_TYPE_POISONOUS_CHICKEN_SOUP){
        ESP_LOGE(TAG, "buf_type error");
        return;
    }
    cJSON *cjson_outmost_item = NULL;
    cjson_outmost_item = cJSON_Parse(wifi_read_buf);
    if(cjson_outmost_item == NULL){
        ESP_LOGE(TAG, "parse json failed");
        goto err;
    }
    //读取最外面的数据
    cJSON *cjson_msg_item = cJSON_GetObjectItem(cjson_outmost_item, "msg");
    if(strcmp(cjson_msg_item->valuestring, "success") == 0)
    {
        ESP_LOGI(TAG, "get data success");
    }else{
        ESP_LOGE(TAG, "get data failed");
        goto err;
    }
    // 解析实际的数据
    cJSON *cjson_result_item = cJSON_GetObjectItem(cjson_outmost_item, "result");
    cJSON *cjson_context_item = cJSON_GetObjectItem(cjson_result_item, "content");
    if(text_context_windows != NULL){
        lvgl_port_lock(0);
        lv_textarea_set_text(text_context_windows, cjson_context_item->valuestring);
        lvgl_port_unlock();
    }
err:
    cJSON_Delete(cjson_outmost_item);
}

void get_http_data_riddle_task(void* parameter){
    example_wifi_reconnect();
    ESP_LOGI("API", "get_http_data_riddle_task");
    int i = 0;
    while(get_target_url(RIDDLE_API_URL) != ESP_OK && i < 3){
        vTaskDelay(100 / portTICK_PERIOD_MS);
        i++;
    }
    if(i >= 3){
        if(text_context_windows != NULL){
            lvgl_port_lock(0);
            lv_textarea_set_text(text_context_windows, "获取脑筋急转弯失败");
            lvgl_port_unlock();
        }
        vTaskDelete(NULL);
    }
    buf_type = BUF_TYPE_RIDDLE;
    deal_riddle(1, 1);
    example_wifi_shutdown();
    vTaskDelete(NULL);
}

void get_http_data_poisonous_chicken_soup_task(void* parameter){
    example_wifi_reconnect();
    ESP_LOGI("API", "get_http_data_riddle_task");
    int i = 0;
    while(get_target_url(POISONOUS_CHICKEN_SOUP_API_URL) != ESP_OK && i < 3){
        vTaskDelay(100 / portTICK_PERIOD_MS);
        i++;
    }
    if(i >= 3){
        if(text_context_windows != NULL){
            lvgl_port_lock(0);
            lv_textarea_set_text(text_context_windows, "获取毒鸡汤失败");
            lvgl_port_unlock();
        }
        vTaskDelete(NULL);
    }
    buf_type = BUF_TYPE_POISONOUS_CHICKEN_SOUP;
    deal_poisonous_chicken_soup();
    example_wifi_shutdown();
    vTaskDelete(NULL);

}


void get_http_data_constellation_task(void* parameter){
    example_wifi_reconnect();
    const char *data = (const char *)parameter;
    ESP_LOGI("API", "get_http_data_riddle_task");
    int i = 0;
    char temp_buf[200];
    sprintf(temp_buf, "%s%s", CONSTELLATION_API_URL, data);
    ESP_LOGI("API", "url: %s", temp_buf);
    while(get_target_url(temp_buf) != ESP_OK && i < 3){
        vTaskDelay(100 / portTICK_PERIOD_MS);
        i++;
    }
    if(i >= 3){
        if(text_context_windows != NULL){
            lvgl_port_lock(0);
            lv_textarea_set_text(text_context_windows, "获取星座运势失败");
            lvgl_port_unlock();
        }
        vTaskDelete(NULL);
    }
    buf_type = BUF_TYPE_CONSTELLATION;
    deal_constellation();
    example_wifi_shutdown();
    vTaskDelete(NULL);
}

void get_http_data_riddle(void){
    xTaskCreatePinnedToCore(get_http_data_riddle_task, "get_http_data_riddle_task", 4 * 1024, NULL, 5, NULL, 0);
}

void get_http_data_story(void){
    xTaskCreatePinnedToCore(get_http_data_story_task, "get_http_data_story_task", 4 * 1024, NULL, 5, NULL, 0);
}

void get_http_data_poisonous_chicken_soup(void){
    xTaskCreatePinnedToCore(get_http_data_poisonous_chicken_soup_task, "get_http_data_poisonous_chicken_soup_task", 4 * 1024, NULL, 5, NULL, 0);
}

void get_http_data_constellation(const char *data){
    xTaskCreatePinnedToCore(get_http_data_constellation_task, "get_http_data_constellation_task", 4 * 1024, data, 5, NULL, 0);
}

void get_http_data_birthday_character(const char *data){
    xTaskCreatePinnedToCore(get_http_data_birthday_character_task, "get_http_data_birthday_character_task", 4 * 1024, data, 5, NULL, 0);
}

/*获取现在的时间*/
time_t now;
struct tm timeinfo;

// 获得日期时间 任务函数
void get_time(void)
{
    get_client();
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
    // wait for time to be set
    int retry = 0;
    const int retry_count = 6;
    lv_obj_t*Label = lv_label_create(lv_scr_act());
    lv_obj_set_size(Label, 200, 40);
    lv_obj_align(Label, LV_ALIGN_CENTER, 0, 0);
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        lv_label_set_text_fmt(Label, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    }

    if(retry>5)
    {
        
        lv_label_set_text(Label, "get time fail, will restart");
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        esp_restart(); // 没有获取到时间的话 重启ESP32
    }

    esp_netif_sntp_deinit();
    // 设置时区
    setenv("TZ", "CST-8", 1); 
    tzset();
    // 获取系统时间
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", asctime(&timeinfo));

    xEventGroupSetBits(s_wifi_event_group, WIFI_GET_SNTP_BIT);
    relesae_client();
    lv_obj_del(Label);
}


int qwdaily_tempMax;       // 当天最高温度
int qwdaily_tempMin;       // 当天最低温度
char qwdaily_sunrise[10];  // 当天日出时间
char qwdaily_sunset[10];   // 当天日落时间

int qwnow_temp; // 实时天气温度
int qwnow_humi; // 实时天气湿度
int qwnow_icon; // 实时天气图标
char qwnow_text[32]; // 实时天气状态

int qanow_level;       // 实时空气质量等级
// GZIP解压函数
int gzDecompress(char *src, int srcLen, char *dst, int* dstLen)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    strm.avail_in = srcLen;
    strm.avail_out = *dstLen;
    strm.next_in = (Bytef *)src;
    strm.next_out = (Bytef *)dst;

    int err = -1;
    err = inflateInit2(&strm, 31); // 初始化
    if (err == Z_OK)
    {
        printf("inflateInit2 err=Z_OK\n");
        err = inflate(&strm, Z_FINISH); // 解压gzip数据
        if (err == Z_STREAM_END) // 解压成功
        { 
            printf("inflate err=Z_OK\n");
            *dstLen = strm.total_out; 
        } 
        else // 解压失败
        {
            printf("inflate err=!Z_OK\n");
        }
        inflateEnd(&strm);
    } 
    else
    {
        printf("inflateInit2 err! err=%d\n", err);
    }

    return err;
}

// 获取每日天气预报, 每日的日出日落以及最高最低气温
void get_daily_weather(void)
{

    int client_code = 0;
    int64_t gzip_len = 0;
    get_client();
    memset(wifi_read_buf, 0, MAX_HTTP_OUTPUT_BUFFER);
    read_pos = 0;
    cannot_read = 0;
    esp_http_client_config_t config = {
        .url = QWEATHER_DAILY_URL,
        .event_handler = _http_event_handle,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data = wifi_read_buf,        // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        client_code = esp_http_client_get_status_code(client);
        gzip_len = read_pos;
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %"PRIu64, client_code, gzip_len);
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }

    if (client_code == 200 && cannot_read == 0)
    {
        int buffSize = MAX_HTTP_OUTPUT_BUFFER;
        char* buffData = (char*)malloc(MAX_HTTP_OUTPUT_BUFFER);
        memset(buffData, 0, MAX_HTTP_OUTPUT_BUFFER);

        int ret = gzDecompress(wifi_read_buf, gzip_len, buffData, &buffSize);

        if (Z_STREAM_END == ret) { /* 解压成功 */
            printf("daily_weather decompress success\n");
            printf("buffSize = %d\n", buffSize);
            // lv_label_set_text(label_weather, "√ 天气信息获取成功");

            cJSON *root = cJSON_Parse(buffData);
            cJSON *daily = cJSON_GetObjectItem(root,"daily");

            cJSON *daily1 = cJSON_GetArrayItem(daily, 0);

            char *temp_max = cJSON_GetObjectItem(daily1,"tempMax")->valuestring;
            char *temp_min = cJSON_GetObjectItem(daily1,"tempMin")->valuestring;
            char *sunset = cJSON_GetObjectItem(daily1,"sunset")->valuestring;
            char *sunrise = cJSON_GetObjectItem(daily1,"sunrise")->valuestring;

            qwdaily_tempMax = atoi(temp_max);
            qwdaily_tempMin = atoi(temp_min);
            strcpy(qwdaily_sunrise, sunrise);
            strcpy(qwdaily_sunset, sunset);

            ESP_LOGI(TAG, "最高气温：%d", qwdaily_tempMax);
            ESP_LOGI(TAG, "最低气温：%d", qwdaily_tempMin);
            ESP_LOGI(TAG, "日出时间：%s", qwdaily_sunrise);
            ESP_LOGI(TAG, "日落时间：%s", qwdaily_sunset);

            cJSON_Delete(root);
        }
        else {
            printf("decompress failed:%d\n", ret);
        }
        free(buffData);
    }
    // esp_http_client_close(client);
    esp_http_client_cleanup(client);
    relesae_client();
    
}
// 获取实时天气信息
void get_now_weather(void)
{
    get_client();
    int client_code = 0;
    int64_t gzip_len = 0;

    memset(wifi_read_buf, 0, MAX_HTTP_OUTPUT_BUFFER);
    read_pos = 0;
    cannot_read = 0;
    esp_http_client_config_t config = {
        .url = QWEATHER_NOW_URL,
        .event_handler = _http_event_handle,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data = wifi_read_buf,        // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK ) {
        client_code = esp_http_client_get_status_code(client);
        gzip_len = read_pos;
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %"PRIu64, client_code, gzip_len);
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }

    if(client_code == 200 && cannot_read == 0)
    {
        int buffSize = MAX_HTTP_OUTPUT_BUFFER;
        char* buffData = (char*)malloc(MAX_HTTP_OUTPUT_BUFFER);
        memset(buffData, 0, MAX_HTTP_OUTPUT_BUFFER);

        int ret = gzDecompress(wifi_read_buf, gzip_len, buffData, &buffSize);

        if (Z_STREAM_END == ret) { /* 解压成功 */
            printf("now weather decompress success\n");
            printf("buffSize = %d\n", buffSize);

            cJSON *root = cJSON_Parse(buffData);
            cJSON *now = cJSON_GetObjectItem(root,"now");

            char *temp = cJSON_GetObjectItem(now,"temp")->valuestring;
            char *icon = cJSON_GetObjectItem(now,"icon")->valuestring;
            char *humidity = cJSON_GetObjectItem(now,"humidity")->valuestring;

            qwnow_temp = atoi(temp);
            qwnow_humi = atoi(humidity);
            qwnow_icon = atoi(icon);

            ESP_LOGI(TAG, "地区：北京区");
            ESP_LOGI(TAG, "温度：%d", qwnow_temp);
            ESP_LOGI(TAG, "湿度：%d", qwnow_humi);
            ESP_LOGI(TAG, "图标：%d", qwnow_icon);

            cJSON_Delete(root);
        }
        else {
            printf("decompress failed:%d\n", ret);
        }
        free(buffData);
    }
    // esp_http_client_close(client);
    esp_http_client_cleanup(client);
    relesae_client();
}

// 获取实时空气质量
void get_air_quality(void)
{
    get_client();
    int client_code = 0;
    int64_t gzip_len = 0;
    memset(wifi_read_buf, 0, MAX_HTTP_OUTPUT_BUFFER);
    read_pos = 0;
    cannot_read = 0;
    esp_http_client_config_t config = {
        .url = QAIR_NOW_URL,
        .event_handler = _http_event_handle,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data = wifi_read_buf,        // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        client_code = esp_http_client_get_status_code(client);
        gzip_len = read_pos;
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %"PRIu64, client_code, gzip_len);
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }

    if(client_code == 200  && cannot_read == 0)
    {
        int buffSize = MAX_HTTP_OUTPUT_BUFFER;
        char* buffData = (char*)malloc(MAX_HTTP_OUTPUT_BUFFER);
        memset(buffData, 0, MAX_HTTP_OUTPUT_BUFFER);

        int ret = gzDecompress(wifi_read_buf, gzip_len, buffData, &buffSize);

        if (Z_STREAM_END == ret) { /* 解压成功 */
            printf("decompress success\n");
            printf("buffSize = %d\n", buffSize);

            cJSON *root = cJSON_Parse(buffData);
            cJSON *now = cJSON_GetObjectItem(root,"now");

            char *level = cJSON_GetObjectItem(now,"level")->valuestring;

            qanow_level = atoi(level);

            ESP_LOGI(TAG, "空气质量：%d", qanow_level);

            cJSON_Delete(root);
        }
        else {
            printf("decompress failed:%d\n", ret);
        }
        free(buffData);
    }
    // esp_http_client_close(client);
    esp_http_client_cleanup(client);
    relesae_client();
}

