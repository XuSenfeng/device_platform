#ifndef MUSIC_H
#define MUSIC_H

#include "esp_err.h"
#include "esp_log.h"
#include "stdint.h"
#include "stdbool.h"
#include "driver/i2s_std.h"
#include "app_ui.h"

/******************************************************************************/
/***************************   I2S  ↓    **************************************/

/* Example configurations */
#define EXAMPLE_RECV_BUF_SIZE   (2400)
#define EXAMPLE_SAMPLE_RATE     (16000)
#define EXAMPLE_MCLK_MULTIPLE   (384) // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ    (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)
#define EXAMPLE_VOICE_VOLUME    (70)

/* I2S port and GPIOs */
#define I2S_NUM         (0)
#define I2S_MCK_IO      (GPIO_NUM_38)
#define I2S_BCK_IO      (GPIO_NUM_14)
#define I2S_WS_IO       (GPIO_NUM_13)
#define I2S_DO_IO       (GPIO_NUM_45)
#define I2S_DI_IO       (-1)

/***********************************************************/
/*********************    音频 ↓   *************************/
#define ADC_I2S_CHANNEL 4

#define VOLUME_DEFAULT    70

#if APP_REGISTER  & APP_MUSIC
#define CODEC_DEFAULT_SAMPLE_RATE          (48000)
#else 
#define CODEC_DEFAULT_SAMPLE_RATE          (8000)
#endif 
#define CODEC_DEFAULT_BIT_WIDTH            (16)
#define CODEC_DEFAULT_ADC_VOLUME           (24.0)
#define CODEC_DEFAULT_CHANNEL              (2)

#define BSP_I2S_NUM                  I2S_NUM_1

#define GPIO_I2S_LRCK       (GPIO_NUM_13)
#define GPIO_I2S_MCLK       (GPIO_NUM_38)
#define GPIO_I2S_SCLK       (GPIO_NUM_14)
#define GPIO_I2S_SDIN       (GPIO_NUM_12)
#define GPIO_I2S_DOUT       (GPIO_NUM_45)
#define GPIO_PWR_CTRL       (GPIO_NUM_NC)


esp_err_t bsp_codec_set_fs(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch);
esp_err_t bsp_speaker_set_fs(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch);

#define MUSIC_REGISTER  {                                                   \
                            device = (device_t *)malloc(sizeof(device_t));  \
                            memset(device, 0, sizeof(device_t));            \
                            device->name = "music";                         \
                            device->init = bsp_codec_init;                  \
                            device->read = NULL;                            \
                            device->id = DEVICE_MUSIC_NUM;                  \
                            device->state = NULL;                           \
                            device_register(device);                        \
                        }while(0);

void bsp_codec_init(void *data);
esp_err_t bsp_i2s_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms);
esp_err_t bsp_codec_mute_set(bool enable);
esp_err_t bsp_codec_volume_set(int volume, int *volume_set);

int bsp_get_feed_channel(void);
esp_err_t bsp_get_feed_data(bool is_get_raw_channel, int16_t *buffer, int buffer_len);

/*********************    音频 ↑   *************************/
/***********************************************************/

#endif