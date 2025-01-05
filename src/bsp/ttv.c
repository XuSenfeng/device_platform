/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-09 12:13:47
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-04 18:47:59
 */


#include "esp_tts_voice_xiaole.h"
#include "esp_partition.h"
#include "esp_idf_version.h"
#include "freertos/ringbuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_tts_voice_xiaole.h"
#include "esp_tts_voice_template.h"
#include "esp_tts_player.h"

#include "device_bsp.h"
#include "ttv.h"
#include "esp_err.h"
#include "esp_log.h"
QueueHandle_t ttv_queue = NULL;
SemaphoreHandle_t ttv_finish_mutex = NULL;
static char *TAG = "TTV";

void ttv_init(void *data){
    //获取一下声音的数据
    const esp_partition_t* part=esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "voice_data");
    if (part==NULL) { 
        ESP_LOGE(TAG, "Couldn't find voice data partition!\n"); 
        return;
    } else {
        ESP_LOGI(TAG, "voice_data paration size:%d\n", (int)part->size);
    }
    void* voicedata;

    esp_partition_mmap_handle_t mmap;
    // 将分区的一部分映射至 CPU 指令空间或数据空间
    esp_err_t err=esp_partition_mmap(part, 0, part->size, ESP_PARTITION_MMAP_DATA, (const void **)&voicedata, &mmap);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Couldn't map voice data partition!\n"); 
        return;
    }
    esp_tts_voice_t *voice=esp_tts_voice_set_init(&esp_tts_voice_template, (int16_t*)voicedata); 
    

    esp_tts_handle_t *tts_handle=esp_tts_create(voice);
    if (tts_handle==NULL) {
        ESP_LOGE(TAG, "Couldn't create TTS handle!\n"); 
        return;
    }

    ttv_queue = xQueueGenericCreate(5, sizeof(char *), queueQUEUE_TYPE_BASE);
    ttv_finish_mutex = xSemaphoreCreateCounting(5, 0);
    if(ttv_queue == NULL || ttv_finish_mutex == NULL){
        ESP_LOGE(TAG, "Create queue failed");
        return;
    }

    //建立处理任务
    xTaskCreatePinnedToCore(ttv_task, "ttv_task", 4096, (void *)tts_handle, 5, NULL, 1);
    return;
}

void ttv_task(void *arg){
    esp_tts_handle_t* tts_handle = (esp_tts_handle_t *)arg;
    char *prompt = NULL;
    
    while(1){
        xQueueReceive(ttv_queue, &prompt, portMAX_DELAY);
        if(prompt == NULL){
            ESP_LOGE(TAG, "Receive message a empty message");
            continue;
        }
        printf("%s\n", prompt);
        if (esp_tts_parse_chinese(tts_handle, prompt)) {
            int len[1]={0};
            pa_en(1); // 打开音频功放
            do {
                short *pcm_data=esp_tts_stream_play(tts_handle, len, 3);
                size_t i;
                bsp_i2s_write(pcm_data, len[0]*2, &i , portMAX_DELAY);
                vTaskDelay((len[0] + 20) / (512 * portTICK_PERIOD_MS));
                // vTaskDelay(1000);
                
            } while(len[0]>0);
            pa_en(0); // 打开音频功放
        }
        esp_tts_stream_reset(tts_handle);
        ESP_LOGI(TAG, "Finish");
        xSemaphoreGive(ttv_finish_mutex);
    }
}
//int (*read)(void * data, uint32_t len); //设备读取函数
esp_err_t ttv_send_message(char *message, int len){
    int err = xQueueGenericSend(ttv_queue, &message, 1000,queueSEND_TO_BACK);
    if(err != pdTRUE){
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t ttv_wait_to_end(){
    //begin to wait
    ESP_LOGI(TAG, "Wait to end");
    xSemaphoreTake(ttv_finish_mutex, portMAX_DELAY);
    ESP_LOGI(TAG, "End");
    return ESP_OK;
}