#include "device_platform.h"
#include "esp_err.h"
#include "gen_i2c.h"
#include "esp_log.h"
#include "soft_i2c.h"
#include "driver/uart.h"
#include "device_manage.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "device_platform";
wait_data_t * wait_data = NULL; //记录等待的数据
/// @brief 通道一暂时不使用
/// @return 
esp_err_t dev_plt_channel1_init(){
    // return i2c_master_init();
    return ESP_OK;
}
EventGroupHandle_t plt_dev_group;

esp_err_t dev_plt_channel2_init(int mode){
    if(mode == DEVIDE_CHANNEL2_MODE_I2C){
        ESP_LOGE(TAG, "I2C mode is not supported");
        return ESP_FAIL;
    }else if(mode == DEVIDE_CHANNEL2_MODE_UART){
        // 初始化一下UART
        const uart_port_t uart_num = DEV_PLT_USART_NUM;
        uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS, // 8位数据位
            .parity = UART_PARITY_DISABLE, // 无奇偶校验
            .stop_bits = UART_STOP_BITS_1, // 1位停止位
            .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS, // 硬件流控
            .rx_flow_ctrl_thresh = 122, // 122字节
        };
        // Configure UART parameters
        ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
        if(!DEVICE_MODE){
            ESP_ERROR_CHECK(uart_set_pin(uart_num, DEVICE_CHANNEL2_GPIO2, DEVICE_CHANNEL2_GPIO1, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        }else{
            ESP_ERROR_CHECK(uart_set_pin(uart_num, DEVICE_CHANNEL2_GPIO1, DEVICE_CHANNEL2_GPIO2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        }
        // Setup UART buffered IO with event queue
        const int uart_buffer_size = (1024 * 1);

        // 事件队列, uart_queue用于处理UART事件, plt_dev_group用于处理不同的设备
        QueueHandle_t uart_queue; 
        
        // Install UART driver using an event queue here
        ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, \
                                                uart_buffer_size, 10, &uart_queue, 0));

        plt_dev_group = xEventGroupCreate(); //建立一个实践组
        // xTaskCreate(dev_plt_task, "dev_plt_task", 2048, uart_queue, 22, NULL);
        xTaskCreatePinnedToCore(dev_plt_task, "dev_plt_task", 1024*3, uart_queue, 22, NULL, 1);
        return ESP_OK;
    } 
    return ESP_FAIL;
}

/// @brief 设备平台的初始化函数
/// @param  
esp_err_t dev_plt_init(dev_plt_init_t * dev_plt_init){
    if(dev_plt_init == NULL){
        return ESP_FAIL;
    }
    if(dev_plt_init->channel == 1){
        // 初始化I2C
        dev_plt_channel1_init();
    }else if(dev_plt_init->channel == 2){
        // 初始化UART或I2C
        dev_plt_channel2_init(dev_plt_init->connect_mode);
    }
    return ESP_OK;
}
/// @brief 处理获取的数据
/// @param data 获取的数据
///             如果这是一个主机发过来的数据数据格式如下 len(32 byte) + 's' + data len记录需要数据的大小
///             如果这是一个从机发过来的数据数据格式如下 len(32 byte) + 't' + data len记录发送数据的大小
///             获取数据失败的话返回 len(32 byte) + 'f' + data  len记录0
/// @param len 
void dev_plt_deal_data(uint8_t * data, int len){

    printf("data:%s\n", data);
    dev_data_t *dev_data = (dev_data_t *)data; //对方的消息的信息
    if(dev_data->direction == 's'){
        // 主机发过来请求数据
        ESP_LOGI(TAG, "get data from server:%u, len:%u", (unsigned int)dev_data->device_num, (unsigned int)dev_data->len);
        // 获取数据
        char * back_data = (char *)malloc(dev_data->len + sizeof(dev_data_t));
        int ret = device_read(dev_data->device_num, back_data + sizeof(dev_data_t), dev_data->len, 0);
        dev_data_t *back_dev_data = (dev_data_t *)back_data;
        back_dev_data->device_num = dev_data->device_num;
        if(ret != ESP_FAIL){
            back_dev_data->len = ret;
            back_dev_data->direction = 't';
            ESP_LOGI(TAG, "get data success. get data len %d", ret);
        }else{
            back_dev_data->len = 0;
            back_dev_data->direction = 'f';
            ESP_LOGI(TAG, "get data fail");
        }
        // 发送数据
        uart_write_bytes(DEV_PLT_USART_NUM, back_data, ret + sizeof(dev_data_t)); 
    }else if(dev_data->direction == 't'){
        // 从机发过来的数据成功获取
        ESP_LOGI(TAG, "get data from client success:%u, len:%u", (unsigned int)dev_data->device_num, (unsigned int)dev_data->len);
        // 获取数据返回地址
        wait_data_t *wait_data_temp = wait_data;
        while(wait_data_temp != NULL && wait_data_temp->device_num != dev_data->device_num ){
            wait_data_temp = wait_data_temp->next;
        }
        if(wait_data_temp == NULL || wait_data_temp->data == NULL){
            ESP_LOGE(TAG, "get data error no data struct");
            return;
        }
        memcpy(wait_data_temp->data, data + sizeof(dev_data_t), dev_data->len);
        wait_data_temp->len = dev_data->len;
        // 释放事件组
        xEventGroupSetBits(plt_dev_group, dev_data->device_num);
    }else if(dev_data->direction == 'f'){
        // 从机发过来的数据获取失败
        ESP_LOGI(TAG, "get data from client fail:%u, len:%u", (unsigned int)dev_data->device_num, (unsigned int)dev_data->len);
        wait_data_t *wait_data_temp = wait_data;
        while(wait_data_temp != NULL && wait_data_temp->device_num != dev_data->device_num && 
                    wait_data_temp->len == 0){
            wait_data_temp = wait_data_temp->next;
        }

        if(wait_data_temp == NULL){
            ESP_LOGE(TAG, "get data error no data struct");
            return;
        }
        wait_data_temp->len = -1;
        // 释放事件组
        xEventGroupSetBits(plt_dev_group, dev_data->device_num);
    }
}

/// @brief 从设备平台获取数据
/// @param device_num 设备的编号
/// @param data 获取的数据
/// @param len 获取的数据的长度
esp_err_t dev_plt_get_data(uint32_t device_num, char * data, uint32_t len){
    dev_data_t dev_data;    //发送的数据
    dev_data.device_num = device_num;
    dev_data.len = len;
    dev_data.direction = 's';
    uart_write_bytes(DEV_PLT_USART_NUM, (char *)&dev_data, sizeof(dev_data_t)); //发送数据
    wait_data_t *wait_data_temp = (wait_data_t *)malloc(sizeof(wait_data_t));
    
    wait_data_temp->device_num = device_num;
    wait_data_temp->len = 0;
    wait_data_temp->next = wait_data;
    wait_data_temp->data = data;
    wait_data = wait_data_temp; //记录数据

    ESP_LOGI(TAG, "want data from server:%u, len:%u", (unsigned int)dev_data.device_num, (unsigned int)dev_data.len);
    xEventGroupWaitBits(plt_dev_group, device_num, pdTRUE, pdTRUE, 1000); //等待数据
    ESP_LOGI(TAG, "get data from server");
    wait_data_temp = wait_data; //获取第一个对应设备的数据
    wait_data_t *pre_wait_data = wait_data;
    while(wait_data_temp != NULL && wait_data_temp->device_num != device_num 
                    &&  wait_data_temp->len == 0){
        pre_wait_data = wait_data_temp;
        wait_data_temp = wait_data_temp->next; //找到数据
    }

    if(wait_data_temp == NULL || pre_wait_data == NULL){
        ESP_LOGE(TAG, "get data error no data struct");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "get data from server success");
    if(pre_wait_data == wait_data_temp){
        wait_data = wait_data_temp->next; //删除数据
    }else{
        pre_wait_data->next = wait_data_temp->next; //删除数据
    }
    ESP_LOGI(TAG, "free status");
    if(wait_data_temp->len == -1){
        free(wait_data_temp);
        ESP_LOGE(TAG, "get data error no data");
        return ESP_FAIL;
    }
    free(wait_data_temp);
    return ESP_OK;
}

/// @brief 设备平台的主机服务函数
/// @param  
void dev_plt_task(void * data){
    QueueHandle_t uart1_queue = (QueueHandle_t)data;
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    ESP_LOGI(TAG, "uart[%d] task start", DEV_PLT_USART_NUM);
    for (;;) {
        //Waiting for UART event.
        if (xQueueReceive(uart1_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            memset(dtmp, 0, RD_BUF_SIZE);
            ESP_LOGI(TAG, "uart[%d] event:", DEV_PLT_USART_NUM);
            switch (event.type) {
            //Event of UART receving data
            case UART_DATA:
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(DEV_PLT_USART_NUM, dtmp, event.size, portMAX_DELAY);
                // printf("data:%s\n", dtmp);
                dev_plt_deal_data(dtmp, event.size);
                break;
            //Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(DEV_PLT_USART_NUM);
                xQueueReset(uart1_queue);
                break;
            //Event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider increasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(DEV_PLT_USART_NUM);
                xQueueReset(uart1_queue);
                break;
            //Event of UART RX break detected
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            //Event of UART parity check error
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            //Event of UART frame error
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            //Others
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}





