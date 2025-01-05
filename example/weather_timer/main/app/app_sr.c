
#include "app_sr.h"
#include "esp_log.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_afe_sr_iface.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "esp_mn_speech_commands.h"
#include "model_path.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_process_sdkconfig.h"
#include "audio_player.h"
#include "app_music.h"

#include "device_bsp.h"
#include "app_ui.h"


static const char *TAG = "app_sr";

srmodel_list_t *models = NULL;
static esp_afe_sr_iface_t *afe_handle = NULL;
static esp_afe_sr_data_t *afe_data = NULL;

int detect_flag = 0;
static volatile int task_flag = 0;

void feed_Task(void *arg)
{
    esp_afe_sr_data_t *afe_data = arg;  // 获取参数
    int audio_chunksize = afe_handle->get_feed_chunksize(afe_data); // 获取帧长度
    int nch = afe_handle->get_channel_num(afe_data); // 获取声道数
    int feed_channel = bsp_get_feed_channel(); // 获取ADC输入通道数
    assert(nch <= feed_channel);
    int16_t *i2s_buff = heap_caps_malloc(audio_chunksize * sizeof(int16_t) * feed_channel, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM); // 分配获取I2S数据的缓存大小
    assert(i2s_buff);

    while (task_flag) {
        bsp_get_feed_data(false, i2s_buff, audio_chunksize * sizeof(int16_t) * feed_channel);  // 获取I2S数据

        afe_handle->feed(afe_data, i2s_buff); // 把获取到的I2S数据输入给afe_data
    }
    if (i2s_buff) {
        free(i2s_buff);
        i2s_buff = NULL;
    }
    vTaskDelete(NULL);
}

void detect_Task(void *arg)
{
    esp_afe_sr_data_t *afe_data = arg;  // 接收参数
    int afe_chunksize = afe_handle->get_fetch_chunksize(afe_data);  // 获取fetch帧长度
    char *mn_name = esp_srmodel_filter(models, ESP_MN_PREFIX, ESP_MN_CHINESE); // 初始化命令词模型
    printf("multinet:%s\n", mn_name); // 打印命令词模型名称
    esp_mn_iface_t *multinet = esp_mn_handle_from_name(mn_name);
    model_iface_data_t *model_data = multinet->create(mn_name, 6000);  // 设置唤醒后等待事件 6000代表6000毫秒
    esp_mn_commands_clear(); // 清除当前的命令词列表
    int light = bsp_display_brightness_get_defalut_light(); // 获取默认亮度
    #if APP_REGISTER & UI_ID_MUSIC
    esp_mn_commands_add(1, "bo fang yin yue"); // 播放音乐
    esp_mn_commands_add(2, "zan ting"); // 暂停
    esp_mn_commands_add(3, "ji xu"); // 继续
    esp_mn_commands_add(4, "shang yi shou"); // 上一首
    esp_mn_commands_add(5, "xia yi shou"); // 下一首
    esp_mn_commands_add(6, "sheng yin da yi dian"); // 声音大一点
    esp_mn_commands_add(7, "sheng yin xiao yi dian"); // 声音小一点
    #elif APP_REGISTER & UI_ID_TIMER
    esp_mn_commands_add(1, "ting gu shi"); // 听故事
    esp_mn_commands_add(2, "liang du jiang di"); // 亮度++
    esp_mn_commands_add(3, "liang du ti sheng"); // 亮度--
    esp_mn_commands_add(4, "dang qian shi jian"); // 时间
    esp_mn_commands_add(5, "dang qian tian qi"); // 天气
    esp_mn_commands_add(6, "sheng yin da yi dian"); // 声音大一点
    esp_mn_commands_add(7, "sheng yin xiao yi dian"); // 声音小一点
    #endif 
    esp_mn_commands_update(); // 更新命令词
    int mu_chunksize = multinet->get_samp_chunksize(model_data);  // 获取samp帧长度
    assert(mu_chunksize == afe_chunksize);

    // 打印所有的命令
    multinet->print_active_speech_commands(model_data);
    printf("------------detect start------------\n");

    while (task_flag) {
        afe_fetch_result_t* res = afe_handle->fetch(afe_data); // 获取模型输出结果
        if (!res || res->ret_value == ESP_FAIL) {
            printf("fetch error!\n");
            break;
        }
        if (res->wakeup_state == WAKENET_DETECTED) {
            printf("WAKEWORD DETECTED\n");
	        multinet->clean(model_data);  // clean all status of multinet
        } else if (res->wakeup_state == WAKENET_CHANNEL_VERIFIED) {  // 检测到唤醒词
            // play_voice = -1;
            afe_handle->disable_wakenet(afe_data);  // 关闭唤醒词识别
            detect_flag = 1; // 标记已检测到唤醒词
            ai_gui_in(); // AI人出现
            printf("AFE_FETCH_CHANNEL_VERIFIED, channel index: %d\n", res->trigger_channel_id);
        }

        if (detect_flag == 1) {
            esp_mn_state_t mn_state = multinet->detect(model_data, res->data); // 检测命令词

            if (mn_state == ESP_MN_STATE_DETECTING) {
                continue;
            }

            if (mn_state == ESP_MN_STATE_DETECTED) { // 已检测到命令词
                esp_mn_results_t *mn_result = multinet->get_results(model_data); // 获取检测词结果
                for (int i = 0; i < mn_result->num; i++) { // 打印获取到的命令词
                    printf("TOP %d, command_id: %d, phrase_id: %d, string:%s prob: %f\n", 
                    i+1, mn_result->command_id[i], mn_result->phrase_id[i], mn_result->string, mn_result->prob[i]);
                }
                // 根据命令词 执行相应动作
                switch (mn_result->command_id[0])
                {
#if APP_REGISTER & UI_ID_MUSIC
                    case 1: // bo fang yin yue 播放音乐
                        ai_play();
                        break;
                    case 2: // zan ting 暂停
                        ai_pause();
                        break;
                    case 3: // ji xu 继续
                        ai_resume();
                        break;
                    case 4: // shang yi shou 上一首
                        ai_prev_music();
                        break;
                    case 5: // xia yi shou 下一首
                        ai_next_music();
                        break;
                    case 6: // sheng yin da yi dian 声音大一点
                        ai_volume_up();
                        break;
                    case 7: // sheng yin xiao yi dian 声音小一点
                        ai_volume_down();
                        break;
#elif APP_REGISTER & UI_ID_TIMER
                    case 1: // ting gu shi 听故事
                        get_http_data_story();
                        break;
                    case 2: // liang du jiang di 亮度++
                        light += 10;
                        if(light > 100) light = 100;
                        bsp_display_brightness_set_defalut_light(light);
                        break;
                    case 3: // liang du ti sheng 亮度--
                        light -= 10;
                        if(light < 0) light = 0;
                        bsp_display_brightness_set_defalut_light(light);
                        break;
                    case 4: // dang qian shi jian 时间
                        timer_read_timer();
                        break;
                    case 5: // dang qian tian qi 天气
                        timer_read_weather();
                        break;
                    case 6: // sheng yin da yi dian 声音大一点
                        ai_volume_up();
                        break;
                    case 7: // sheng yin xiao yi dian 声音小一点
                        ai_volume_down();
                        break;
#endif 
                    default:
                        break;
                }
                printf("\n-----------listening-----------\n");
            }

            if (mn_state == ESP_MN_STATE_TIMEOUT) { // 达到最大检测命令词时间
                esp_mn_results_t *mn_result = multinet->get_results(model_data);
                printf("timeout, string:%s\n", mn_result->string);
                afe_handle->enable_wakenet(afe_data);  // 重新打开唤醒词识别
                detect_flag = 0; // 清除标记
                printf("\n-----------awaits to be waken up-----------\n");
                ai_gui_out(); // AI人退出
                continue;
            }
        }
    }
    if (model_data) {
        multinet->destroy(model_data);
        model_data = NULL;
    }
    printf("detect exit\n");
    vTaskDelete(NULL);
}

void app_sr_init(void)
{
    models = esp_srmodel_init("model"); // 获取模型 名称“model”和分区表中装载模型的名称一致

    afe_handle = (esp_afe_sr_iface_t *)&ESP_AFE_SR_HANDLE;  // 先配置afe句柄 随后才可以调用afe接口
    afe_config_t afe_config = AFE_CONFIG_DEFAULT(); // 配置afe

    afe_config.wakenet_model_name = esp_srmodel_filter(models, ESP_WN_PREFIX, NULL); // 配置唤醒模型 必须在create_from_config之前配置
    afe_data = afe_handle->create_from_config(&afe_config); // 创建afe_data
    ESP_LOGI(TAG, "wakenet:%s", afe_config.wakenet_model_name); // 打印唤醒名称

    task_flag = 1;
    xTaskCreatePinnedToCore(&detect_Task, "detect", 6 * 1024, (void*)afe_data, 5, NULL, 1); 
    xTaskCreatePinnedToCore(&feed_Task, "feed", 6 * 1024, (void*)afe_data, 5, NULL, 0);
    
    // esp_tts_parse_chinese
}

