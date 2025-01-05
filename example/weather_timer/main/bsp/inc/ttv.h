#ifndef TTV_H
#define TTV_H
#include "esp_tts.h"
#include "freertos/queue.h"

extern QueueHandle_t ttv_queue;
void ttv_task(void *arg);
void ttv_init(void *data);
esp_err_t ttv_send_message(char *message, int len);
#define TTV_REGISTER        {                                                   \
                                device = (device_t *)malloc(sizeof(device_t));  \
                                memset(device, 0, sizeof(device_t));            \
                                device->name = "TTV";                           \
                                device->init = ttv_init;                        \
                                device->read = NULL;                            \
                                device->write = ttv_send_message;               \
                                device->id = DEVICE_TTV_NUM;                    \
                                device->state = NULL;                           \
                                device_register(device);                        \
                            }while(0);
extern SemaphoreHandle_t ttv_finish_mutex;
esp_err_t ttv_wait_to_end();
// #define page_read(s) {if(page_read_flag)device_write(DEVICE_TTV_NUM, s, strlen(s), 0);}while(0);
#define page_read(s)

#endif //TTV_H
