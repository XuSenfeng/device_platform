/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-10 12:14:57
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-12-08 13:03:18
 */
#include "music.h"
#include "device_bsp.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "esp_check.h"
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static char *TAG = "music";

/***********************************************************/
/*********************    音频 ↓   *************************/
static esp_codec_dev_handle_t play_dev_handle;    // speaker句柄
static esp_codec_dev_handle_t record_dev_handle;  // microphone句柄

static i2s_chan_handle_t i2s_tx_chan = NULL; // 发送通道
static i2s_chan_handle_t i2s_rx_chan = NULL; // 接收通道
static const audio_codec_data_if_t *i2s_data_if = NULL;  /* Codec data interface */

extern int init_mode;
// I2S总线初始化
esp_err_t bsp_audio_init(void)
{
    esp_err_t ret = ESP_FAIL;
    if (i2s_tx_chan && i2s_rx_chan) {
        /* Audio was initialized before */
        return ESP_OK;
    }

    /* Setup I2S peripheral */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(BSP_I2S_NUM, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true; // Auto clear the legacy data in the DMA buffer
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &i2s_tx_chan, &i2s_rx_chan));

    /* Setup I2S channels */
    const i2s_std_config_t std_cfg_default = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),   // 采样率16000
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(32, I2S_SLOT_MODE_STEREO),  // 32位 2通道
        .gpio_cfg = { 
            .mclk = GPIO_I2S_MCLK, 
            .bclk = GPIO_I2S_SCLK, 
            .ws   = GPIO_I2S_LRCK, 
            .dout = GPIO_I2S_DOUT, 
            .din  = GPIO_I2S_SDIN,
        },
    };

    if (i2s_tx_chan != NULL) {
        ESP_GOTO_ON_ERROR(i2s_channel_init_std_mode(i2s_tx_chan, &std_cfg_default), err, TAG, "I2S channel initialization failed");
        ESP_GOTO_ON_ERROR(i2s_channel_enable(i2s_tx_chan), err, TAG, "I2S enabling failed");
    }
    if (i2s_rx_chan != NULL) {
        ESP_GOTO_ON_ERROR(i2s_channel_init_std_mode(i2s_rx_chan, &std_cfg_default), err, TAG, "I2S channel initialization failed");
        ESP_GOTO_ON_ERROR(i2s_channel_enable(i2s_rx_chan), err, TAG, "I2S enabling failed");
    }

    audio_codec_i2s_cfg_t i2s_cfg = {
        .port = BSP_I2S_NUM,
        .rx_handle = i2s_rx_chan,
        .tx_handle = i2s_tx_chan,
    };
    i2s_data_if = audio_codec_new_i2s_data(&i2s_cfg);
    if (i2s_data_if == NULL) {   
        goto err;
    }   

    return ESP_OK;

err:
    if (i2s_tx_chan) {
        i2s_del_channel(i2s_tx_chan);
    }
    if (i2s_rx_chan) {
        i2s_del_channel(i2s_rx_chan);
    }

    return ret;
}

// 初始化音频输出芯片
esp_codec_dev_handle_t bsp_audio_codec_speaker_init(void)
{
    if (i2s_data_if == NULL) {
        /* Configure I2S peripheral and Power Amplifier */
        ESP_ERROR_CHECK(bsp_audio_init());
    }
    assert(i2s_data_if);

    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();

    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = BSP_I2C_NUM,
        .addr = ES8311_CODEC_DEFAULT_ADDR,
    };
    const audio_codec_ctrl_if_t *i2c_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    assert(i2c_ctrl_if);

    esp_codec_dev_hw_gain_t gain = {
        .pa_voltage = 5.0,
        .codec_dac_voltage = 3.3,
    };

    es8311_codec_cfg_t es8311_cfg = {
        .ctrl_if = i2c_ctrl_if,
        .gpio_if = gpio_if,
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC,
        .pa_pin = GPIO_PWR_CTRL,
        .pa_reverted = false,
        .master_mode = false,
        .use_mclk = true,
        .digital_mic = false,
        .invert_mclk = false,
        .invert_sclk = false,
        .hw_gain = gain,
    };
    const audio_codec_if_t *es8311_dev = es8311_codec_new(&es8311_cfg);
    assert(es8311_dev);

    esp_codec_dev_cfg_t codec_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_OUT,
        .codec_if = es8311_dev,
        .data_if = i2s_data_if,
    };
    return esp_codec_dev_new(&codec_dev_cfg);
}

// 初始化音频输入芯片
esp_codec_dev_handle_t bsp_audio_codec_microphone_init(void)
{
    if (i2s_data_if == NULL) {
        /* Configure I2S peripheral and Power Amplifier */
        ESP_ERROR_CHECK(bsp_audio_init());
    }
    assert(i2s_data_if);

    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = BSP_I2C_NUM,
        .addr = 0x82,
    };
    const audio_codec_ctrl_if_t *i2c_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    assert(i2c_ctrl_if);

    es7210_codec_cfg_t es7210_cfg = {
        .ctrl_if = i2c_ctrl_if,
        .mic_selected = ES7120_SEL_MIC1 | ES7120_SEL_MIC2 | ES7120_SEL_MIC3 | ES7120_SEL_MIC4,
    };
    const audio_codec_if_t *es7210_dev = es7210_codec_new(&es7210_cfg);
    assert(es7210_dev);

    esp_codec_dev_cfg_t codec_es7210_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_IN,
        .codec_if = es7210_dev,
        .data_if = i2s_data_if,
    };
    return esp_codec_dev_new(&codec_es7210_dev_cfg);
}

// 设置采样率
esp_err_t bsp_codec_set_fs(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
{
    esp_err_t ret = ESP_OK;

    esp_codec_dev_sample_info_t fs = {
        .sample_rate = rate,
        .channel = ch,
        .bits_per_sample = bits_cfg,
    };
    
    if (play_dev_handle) {
        ret = esp_codec_dev_close(play_dev_handle);
    }
    if (record_dev_handle) {
        ret |= esp_codec_dev_close(record_dev_handle);
        ret |= esp_codec_dev_set_in_gain(record_dev_handle, CODEC_DEFAULT_ADC_VOLUME);
    }

    if (play_dev_handle) {
        ret |= esp_codec_dev_open(play_dev_handle, &fs);
    }
    if (record_dev_handle) {
        ret |= esp_codec_dev_open(record_dev_handle, &fs);
    }
    return ret;
}
#define KEY_GPIO_NUM 0
void start_codec_task(void *arg)
{
    play_dev_handle = bsp_audio_codec_speaker_init();
    assert((play_dev_handle) && "play_dev_handle not initialized");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    record_dev_handle = bsp_audio_codec_microphone_init();
    assert((record_dev_handle) && "record_dev_handle not initialized");
    if(gpio_get_level(KEY_GPIO_NUM)){
        bsp_codec_set_fs(CODEC_DEFAULT_SAMPLE_RATE, CODEC_DEFAULT_BIT_WIDTH, CODEC_DEFAULT_CHANNEL);
        ESP_LOGI(TAG, "codec rate %d", CODEC_DEFAULT_SAMPLE_RATE);
    }else{
        bsp_codec_set_fs(48000, CODEC_DEFAULT_BIT_WIDTH, CODEC_DEFAULT_CHANNEL);
        ESP_LOGI(TAG, "codec rate %d", 48000);
    }
    
    bsp_codec_mute_set(false);
    bsp_codec_volume_set(DEFAULT_SOUND_VALUSE, NULL); // 设置音量

    vTaskDelete(NULL);
}

// 音频芯片初始化
void bsp_codec_init(void *data)
{
    xTaskCreatePinnedToCore(start_codec_task, "start_codec_task",  4096, NULL, 22, NULL, 1);
}

// 播放音乐
esp_err_t bsp_i2s_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;
    ret = esp_codec_dev_write(play_dev_handle, audio_buffer, len);
    *bytes_written = len;
    return ret;
}

// 设置静音与否
esp_err_t bsp_codec_mute_set(bool enable)
{
    esp_err_t ret = ESP_OK;
    ret = esp_codec_dev_set_out_mute(play_dev_handle, enable);
    return ret;
}

// 设置喇叭音量
esp_err_t bsp_codec_volume_set(int volume, int *volume_set)
{
    esp_err_t ret = ESP_OK;
    float v = volume;
    ret = esp_codec_dev_set_out_vol(play_dev_handle, (int)v);
    return ret;
}

int bsp_get_feed_channel(void)
{
    return ADC_I2S_CHANNEL;
}

esp_err_t bsp_get_feed_data(bool is_get_raw_channel, int16_t *buffer, int buffer_len)
{
    esp_err_t ret = ESP_OK;
    
    int audio_chunksize = buffer_len / (sizeof(int16_t) * ADC_I2S_CHANNEL);

    ret = esp_codec_dev_read(record_dev_handle, (void *)buffer, buffer_len);
    
    if (!is_get_raw_channel) {
        for (int i = 0; i < audio_chunksize; i++) {
            int16_t ref = buffer[4 * i + 0];
            buffer[3 * i + 0] = buffer[4 * i + 1];
            buffer[3 * i + 1] = buffer[4 * i + 3];
            buffer[3 * i + 2] = ref;
        }
    }

    return ret;
}

/*********************    音频 ↑   *************************/
/***********************************************************/