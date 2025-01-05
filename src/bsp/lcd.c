#include "lcd.h"
#include "extend_io.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "esp_check.h"
#include "driver/spi_master.h"
#include "esp_lcd_types.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch_ft5x06.h"
#include "esp_lvgl_port.h"
#include "qmi8658c.h"

#include "device_bsp.h"
#include "esp_lvgl_port.h"

static char * TAG = "LCD";
#define BSP_I2C_NUM           (1)            // I2C外设
/***********************************************************/
/****************    LCD显示屏 ↓   *************************/

// 背光PWM初始化
esp_err_t bsp_display_brightness_init(void)
{
    // Setup LEDC peripheral for PWM backlight control
    const ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num = BSP_LCD_BACKLIGHT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LCD_LEDC_CH,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = 0,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = true
    };
    const ledc_timer_config_t LCD_backlight_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = 0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&LCD_backlight_timer));
    ESP_ERROR_CHECK(ledc_channel_config(&LCD_backlight_channel));

    return ESP_OK;
}

// 背光亮度设置
esp_err_t bsp_display_brightness_set(int brightness_percent)
{
    if (brightness_percent > 100) {
        brightness_percent = 100;
    } else if (brightness_percent < 0) {
        brightness_percent = 0;
    }

    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness_percent);
    // LEDC resolution set to 10bits, thus: 100% = 1023
    uint32_t duty_cycle = (1023 * brightness_percent) / 100;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH, duty_cycle));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH));

    return ESP_OK;
}

// 关闭背光
esp_err_t bsp_display_backlight_off(void)
{
    return bsp_display_brightness_set(0);
}

// 打开背光 最亮
esp_err_t bsp_display_backlight_on(void)
{
    return bsp_display_brightness_set(100);
}

// 定义液晶屏句柄
esp_lcd_panel_handle_t panel_handle = NULL;
esp_lcd_panel_io_handle_t io_handle = NULL; 
static esp_lcd_touch_handle_t tp;   // 触摸屏句柄
static lv_disp_t *disp;      // 指向液晶屏
static lv_indev_t *disp_indev = NULL; // 指向触摸屏
static lv_indev_drv_t indev_drv_keypad; //按键驱动
lv_indev_t * indev_keypad;

// 液晶屏初始化
esp_err_t bsp_display_new(void)
{
    esp_err_t ret = ESP_OK;
    // 背光初始化
    ESP_RETURN_ON_ERROR(bsp_display_brightness_init(), TAG, "Brightness init failed");
    // 初始化SPI总线
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = BSP_LCD_SPI_CLK,
        .mosi_io_num = BSP_LCD_SPI_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");
    // 液晶屏控制IO初始化
    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_LCD_DC,
        .cs_gpio_num = BSP_LCD_SPI_CS,
        .pclk_hz = BSP_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 2,
        .trans_queue_depth = 10,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_NUM, &io_config, &io_handle), err, TAG, "New panel IO failed");
    // 初始化液晶屏驱动芯片ST7789
    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = BSP_LCD_BITS_PER_PIXEL,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle), err, TAG, "New panel failed");
    
    esp_lcd_panel_reset(panel_handle);  // 液晶屏复位
    lcd_cs(0);  // 拉低CS引脚
    esp_lcd_panel_init(panel_handle);  // 初始化配置寄存器
    esp_lcd_panel_invert_color(panel_handle, true); // 颜色反转
    esp_lcd_panel_swap_xy(panel_handle, true);  // 显示翻转 
    esp_lcd_panel_mirror(panel_handle, true, false); // 镜像

    return ret;

err:
    if (panel_handle) {
        esp_lcd_panel_del(panel_handle);
    }
    if (io_handle) {
        esp_lcd_panel_io_del(io_handle);
    }
    spi_bus_free(BSP_LCD_SPI_NUM);
    return ret;
}

// LCD显示初始化
esp_err_t bsp_lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    
    ret = bsp_display_new(); // 液晶屏驱动初始化
    lcd_set_color(0x0000); // 设置整屏背景黑色
    ret = esp_lcd_panel_disp_on_off(panel_handle, true); // 打开液晶屏显示
    ret = bsp_display_backlight_on(); // 打开背光显示

    return  ret;
}

// 液晶屏初始化+添加LVGL接口
static lv_disp_t *bsp_display_lcd_init(void)
{
    /* 初始化液晶屏 */
    bsp_display_new(); // 液晶屏驱动初始化
    lcd_set_color(0xffff); // 设置整屏背景白色
    esp_lcd_panel_disp_on_off(panel_handle, true); // 打开液晶屏显示

    /* 液晶屏添加LVGL接口 */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = BSP_LCD_H_RES * BSP_LCD_DRAW_BUF_HEIGHT,   // LVGL缓存大小 
        .double_buffer = false, // 是否开启双缓存
        .hres = BSP_LCD_H_RES, // 液晶屏的宽
        .vres = BSP_LCD_V_RES, // 液晶屏的高
        .monochrome = false,  // 是否单色显示器
        /* Rotation的值必须和液晶屏初始化里面设置的 翻转 和 镜像 一样 */
        .rotation = {
            .swap_xy = true,  // 是否翻转
            .mirror_x = true, // x方向是否镜像
            .mirror_y = false, // y方向是否镜像
        },
        .flags = {
            .buff_dma = false,  // 是否使用DMA 注意：dma与spiram不能同时为true
            .buff_spiram = true, // 是否使用PSRAM 注意：dma与spiram不能同时为true
        }
    };

    return lvgl_port_add_disp(&disp_cfg);
}

// 触摸屏初始化
esp_err_t bsp_touch_new(esp_lcd_touch_handle_t *ret_touch)
{
    /* Initialize touch */
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = BSP_LCD_V_RES,
        .y_max = BSP_LCD_H_RES,
        .rst_gpio_num = GPIO_NUM_NC, // Shared with LCD reset
        .int_gpio_num = GPIO_NUM_NC, 
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 1,
            .mirror_x = 1,
            .mirror_y = 0,
        },
    };
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();

    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)BSP_I2C_NUM, &tp_io_config, &tp_io_handle), TAG, "");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, ret_touch));

    return ESP_OK;
}

// 触摸屏初始化+添加LVGL接口
static lv_indev_t *bsp_display_indev_init(lv_disp_t *disp)
{
    /* 初始化触摸屏 */
    ESP_ERROR_CHECK(bsp_touch_new(&tp));
    assert(tp);

    /* 添加LVGL接口 */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = tp,
    };

    return lvgl_port_add_touch(&touch_cfg);

}
static int brightness_time = DEFOULT_LCD_LIGHT_TIME;  // 初识亮屏时间
static int default_sleep_time = DEFOULT_LCD_LIGHT_TIME;    // 默认休眠时间
static int defalut_light = DEFOULT_LCD_LIGHT_VALUE;    // 默认亮度

// 背光管理任务
void bsp_display_brightness_task(void *arg)
{
        if (brightness_time == -1)
        {
            // 时间到关灯
            return;
        }
        else if(brightness_time == 0)
        {
            // 时间到关灯
            ESP_LOGI(TAG, "Turn off the backlight");
            lvgl_port_lock(0);
            bsp_display_brightness_set(0);
            lvgl_port_unlock();
        }
        brightness_time --;
}
void bsp_display_brightness_reset(void)
{
    // ESP_LOGI(TAG, "Reset the backlight time");
    if(brightness_time == -1)
    {
        // 亮屏
        // ESP_LOGI(TAG, "Turn on the backlight");
        lvgl_port_lock(0);
        bsp_display_brightness_set(defalut_light);
        lvgl_port_unlock();
    }
    brightness_time = default_sleep_time;
}

void bsp_display_brightness_set_defalut_time(int time)
{
    ESP_LOGI(TAG, "Set the default sleep time to %d seconds", time);
    default_sleep_time = time;
}

void bsp_display_brightness_set_defalut_light(int light)
{
    defalut_light = light;
    bsp_display_brightness_set(defalut_light);
}

int bsp_display_brightness_get_defalut_light(void)
{
    return defalut_light;
}

int bsp_display_brightness_get_defalut_time(void)
{
    return default_sleep_time;
}
lv_group_t * g;
// 开发板显示初始化
void bsp_lvgl_start(void)
{
    ESP_LOGI(TAG, "Start LVGL LCD initialization");
    /* 初始化LVGL */
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    /* 初始化液晶屏 并添加LVGL接口 */
    disp = bsp_display_lcd_init();

    /* 初始化触摸屏 并添加LVGL接口 */
    disp_indev = bsp_display_indev_init(disp);

    // 注册一下模拟按键
    lv_indev_drv_init(&indev_drv_keypad);
    indev_drv_keypad.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_keypad.read_cb = Analog_keying_task;
    indev_keypad = lv_indev_drv_register(&indev_drv_keypad);

    // 把这一个设备添加到默认组里面
    g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(indev_keypad, g);

    /* 打开液晶屏背光 */
    bsp_display_backlight_on();
    // 背光管理任务
    lv_timer_create(bsp_display_brightness_task, 1000, NULL);
    // xTaskCreatePinnedToCore(bsp_display_brightness_task, "bsp_display_brightness_task", 1024, NULL, 4, NULL, 1);

}


// 设置液晶屏颜色
void lcd_set_color(uint16_t color)
{
    // 分配内存 这里分配了液晶屏一行数据需要的大小
    uint16_t *buffer = (uint16_t *)heap_caps_malloc(BSP_LCD_H_RES * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    
    if (NULL == buffer)
    {
        ESP_LOGE(TAG, "Memory for bitmap is not enough");
    }
    else
    {
        for (size_t i = 0; i < BSP_LCD_H_RES; i++) // 给缓存中放入颜色数据
        {
            buffer[i] = color;
        }
        for (int y = 0; y < 240; y++) // 显示整屏颜色
        {
            esp_lcd_panel_draw_bitmap(panel_handle, 0, y, 320, y+1, buffer);
        }
        free(buffer); // 释放内存
    }
}
/***************    LCD显示屏 ↑   *************************/
/***********************************************************/