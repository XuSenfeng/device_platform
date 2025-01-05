/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-10 10:07:33
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-11-27 19:00:33
 */
#ifndef LCD_H
#define LCD_H
#include "stdint.h"
#include "esp_err.h"

/***********************************************************/
/****************    LCD显示屏 ↓   *************************/
#define BSP_LCD_PIXEL_CLOCK_HZ     (80 * 1000 * 1000)
#define BSP_LCD_SPI_NUM            (SPI2_HOST)
#define LCD_CMD_BITS               (8)
#define LCD_PARAM_BITS             (8)
#define BSP_LCD_BITS_PER_PIXEL     (16)
#define LCD_LEDC_CH          LEDC_CHANNEL_0

#define BSP_LCD_H_RES              (320)
#define BSP_LCD_V_RES              (240)

#define BSP_LCD_SPI_MOSI      (GPIO_NUM_40)
#define BSP_LCD_SPI_CLK       (GPIO_NUM_41)
#define BSP_LCD_SPI_CS        (GPIO_NUM_NC)
#define BSP_LCD_DC            (GPIO_NUM_39)
#define BSP_LCD_RST           (GPIO_NUM_NC)
#define BSP_LCD_BACKLIGHT     (GPIO_NUM_42)  

#define BSP_LCD_DRAW_BUF_HEIGHT    (20)

#define LCD_REGISTER    {                                                   \
                            device = (device_t *)malloc(sizeof(device_t));  \
                            memset(device, 0, sizeof(device_t));            \
                            device->name = "lcd";                           \
                            device->init = bsp_lvgl_start;                    \
                            device->read = NULL;                            \
                            device->id = DEVICE_LCD_NUM;                    \
                            device->state = NULL;                           \
                            device_register(device);                        \
                        }while(0);

esp_err_t bsp_display_brightness_init(void);
void bsp_display_brightness_set_defalut_light(int light);
void bsp_display_brightness_set_defalut_time(int time);
int bsp_display_brightness_get_defalut_light(void);
int bsp_display_brightness_get_defalut_time(void);
void bsp_display_brightness_reset(void);

esp_err_t bsp_display_backlight_off(void);
esp_err_t bsp_display_backlight_on(void);
esp_err_t bsp_lcd_init(void);
void lcd_set_color(uint16_t color);
// void lcd_draw_pictrue(int x_start, int y_start, int x_end, int y_end, const unsigned char *gImage);
void bsp_lvgl_start(void);
/***************    LCD显示屏 ↑   *************************/
/***********************************************************/


#endif // LCD_H