#include "camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "extend_io.h"
// #include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"
#include "esp_jpeg_common.h"
#include "esp_jpeg_enc.h"
// 定义lcd显示队列句柄
static QueueHandle_t xQueueLCDFrame = NULL;
static QueueHandle_t xQueuePhotoFrame = NULL;
static const char *TAG = "camera";
int if_camer_show = 0;
int to_photo_flag = 0;
// 摄像头硬件初始化
void bsp_camera_init(void)
{
    dvp_pwdn(0); // 打开摄像头

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_1;  // LEDC通道选择  用于生成XCLK时钟 但是S3不用
    config.ledc_timer = LEDC_TIMER_1; // LEDC timer选择  用于生成XCLK时钟 但是S3不用
    config.pin_d0 = CAMERA_PIN_D0;
    config.pin_d1 = CAMERA_PIN_D1;
    config.pin_d2 = CAMERA_PIN_D2;
    config.pin_d3 = CAMERA_PIN_D3;
    config.pin_d4 = CAMERA_PIN_D4;
    config.pin_d5 = CAMERA_PIN_D5;
    config.pin_d6 = CAMERA_PIN_D6;
    config.pin_d7 = CAMERA_PIN_D7;
    config.pin_xclk = CAMERA_PIN_XCLK;
    config.pin_pclk = CAMERA_PIN_PCLK;
    config.pin_vsync = CAMERA_PIN_VSYNC;
    config.pin_href = CAMERA_PIN_HREF;
    config.pin_sccb_sda = -1;   // 这里写-1 表示使用已经初始化的I2C接口
    config.pin_sccb_scl = CAMERA_PIN_SIOC;
    config.sccb_i2c_port = 1;
    config.pin_pwdn = CAMERA_PIN_PWDN;
    config.pin_reset = CAMERA_PIN_RESET;
    config.xclk_freq_hz = XCLK_FREQ_HZ;
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_HQVGA;//240 176
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    // camera init
    esp_err_t err = esp_camera_init(&config); // 配置上面定义的参数
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get(); // 获取摄像头型号

    if (s->id.PID == GC0308_PID) {
        s->set_hmirror(s, 1);  // 这里控制摄像头镜像 写1镜像 写0不镜像
    }
    app_camera_lcd();
}
extern esp_lcd_panel_handle_t panel_handle;
extern lv_obj_t *canvsa;
// lcd处理任务
static void task_process_lcd(void *arg)
{
    camera_fb_t *frame = NULL;

    while (true)
    {
        if (xQueueReceive(xQueueLCDFrame, &frame, portMAX_DELAY))
        {
            if(if_camer_show){
                
                // for(int i = 32;i<208;i++){
                lvgl_port_lock(0);

                // ESP_LOGI(TAG, "frame->width: %d, frame->height: %d", frame->width, frame->height);
                lv_canvas_copy_buf(canvsa, frame->buf, 0, 0, 240, 176);
                //刷新
                lv_obj_invalidate(canvsa);

                lvgl_port_unlock();
                // }
                
            }
            // esp_lcd_panel_draw_bitmap(panel_handle, 32, 206, frame->width, frame->height, (uint16_t *)frame->buf + frame->width * frame->height/2);
            if(!to_photo_flag)
                esp_camera_fb_return(frame);
            else{
                to_photo_flag = 0;
                xQueueSend(xQueuePhotoFrame, &frame, portMAX_DELAY);
                
            }
                // esp_camera_fb_return(frame);
                // break;
            
        }
    }
}




jpeg_error_t esp_jpeg_encode_one_picture(camera_fb_t *frame, uint8_t *buf)
{
    // configure encoder
    jpeg_enc_config_t jpeg_enc_cfg = DEFAULT_JPEG_ENC_CONFIG();
    jpeg_enc_cfg.width = frame->width;
    jpeg_enc_cfg.height = frame->height;
    jpeg_enc_cfg.src_type = JPEG_PIXEL_FORMAT_RGB888;
    jpeg_enc_cfg.subsampling = JPEG_SUBSAMPLE_420;
    jpeg_enc_cfg.quality = 60;
    jpeg_enc_cfg.rotate = JPEG_ROTATE_0D;
    jpeg_enc_cfg.task_enable = false;
    jpeg_enc_cfg.hfm_task_priority = 13;
    jpeg_enc_cfg.hfm_task_core = 1;

    jpeg_error_t ret = JPEG_ERR_OK;
    uint8_t *inbuf = buf;
    int image_size = frame->height * frame->width * 3;
    uint8_t *outbuf = NULL;
    int outbuf_size = 100 * 1024;
    int out_len = 0;
    jpeg_enc_handle_t jpeg_enc = NULL;
    FILE *out = NULL;

    // open
    ret = jpeg_enc_open(&jpeg_enc_cfg, &jpeg_enc);
    if (ret != JPEG_ERR_OK) {
        return ret;
    }

    // allocate output buffer to fill encoded image stream
    outbuf = (uint8_t *)calloc(1, outbuf_size);
    if (outbuf == NULL) {
        ret = JPEG_ERR_NO_MEM;
        goto jpeg_example_exit;
    }

    // process
    ret = jpeg_enc_process(jpeg_enc, inbuf, image_size, outbuf, outbuf_size, &out_len);
    
    if (ret != JPEG_ERR_OK) {
        goto jpeg_example_exit;
    }

    char path[64];
    sprintf(path, "/sdcard/%ld.jpg", esp_log_timestamp());
    out = fopen(path, "wb");
    if (out == NULL) {
        goto jpeg_example_exit;
    }
    fwrite(outbuf, 1, out_len, out);
    fclose(out);

jpeg_example_exit:
    // close
    jpeg_enc_close(jpeg_enc);
    if (outbuf) {
        free(outbuf);
    }
    return ret;
}


// 摄像头处理任务
static void task_process_camera(void *arg)
{
    while (true)
    {
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame)
            xQueueSend(xQueueLCDFrame, &frame, portMAX_DELAY);
    }
}

void rgb5652888(uint8_t *src, uint8_t *out, int len){
    uint16_t *src_buf = (uint16_t *)src;
    for(int i = 0;i < len ;i++){
        uint16_t rgb565 = src_buf[i];
        // 转换格式
        // 分离RGB565中的R、G、B值
        int r5 = (rgb565 >> 11) & 0x1F;
        int g6 = (rgb565 >> 5) & 0x3F;
        int b5 = rgb565 & 0x1F;

        // 将5位或6位的值扩展到8位，并进行补偿
        uint8_t r8 = (r5 << 3) | (r5 >> 2);
        uint8_t g8 = (g6 << 2) | (g6 >> 4);
        uint8_t b8 = (b5 << 3) | (b5 >> 2);

        // 合并扩展后的R、G、B值为RGB888
        out[i*3] = r8;
        out[i*3+1] = g8;
        out[i*3+2] = b8;
    }
}

static void task_save_photo(void *arg)
{
    camera_fb_t *frame = NULL;
    while (true)
    {
        if(xQueueReceive(xQueuePhotoFrame, &frame, portMAX_DELAY)){
            if (frame)
            {
                FILE *out = NULL;

                char path[64];
                sprintf(path, "/sdcard/pic_%ld.pic", esp_log_timestamp());
                out = fopen(path, "wb");
                if (out == NULL) {
                    esp_camera_fb_return(frame);
                    return;
                }
                lvgl_port_lock(0);
                fwrite(frame->buf, 1, frame->len, out);
                fclose(out);
                lvgl_port_unlock();
                esp_camera_fb_return(frame);

                ESP_LOGI(TAG, "SAVE FINISH");
            }
        }
    }
}

// 让摄像头显示到LCD
void app_camera_lcd(void)
{
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueuePhotoFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xTaskCreatePinnedToCore(task_process_camera, "task_process_camera", 3 * 1024, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(task_process_lcd, "task_process_lcd", 4 * 1024, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(task_save_photo, "task_save_photo", 4 * 1024, NULL, 5, NULL, 0);
}