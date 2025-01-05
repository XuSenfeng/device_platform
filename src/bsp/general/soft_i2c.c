/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-10-26 10:14:25
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-10-26 10:16:29
 */
#if CONFIG_IDF_TARGET_ESP32C3
#include "bsp_iic.h"
#include "esp_log.h"
#include "unistd.h"
 
static const char *TAG = "I2C";
 
void I2cInit(void)
{
	const gpio_config_t   io_config = {
        .intr_type      = GPIO_INTR_DISABLE,
        .mode           = GPIO_MODE_OUTPUT,
        .pull_down_en   = GPIO_PULLDOWN_DISABLE,
        .pull_up_en     = GPIO_PULLUP_ENABLE,
        .pin_bit_mask   = (1ULL << I2C_SCL_PIN) 
                        | (1ULL << I2C_SDA_PIN) 
    };
    gpio_config(&io_config);
	I2C_SDA_HIGH;
	I2C_SCL_HIGH;
}
 
esp_err_t I2cSdaIn(void)
{
	const gpio_config_t   io_config = {
        .intr_type      = GPIO_INTR_DISABLE,
        .mode           = GPIO_MODE_INPUT,
        .pull_down_en   = GPIO_PULLDOWN_DISABLE,
        .pull_up_en     = GPIO_PULLUP_ENABLE,
        .pin_bit_mask   = (1ULL << I2C_SDA_PIN) 
    };
    return gpio_config(&io_config);
}
 
esp_err_t I2cSdaOut(void)
{
	const gpio_config_t   io_config = {
        .intr_type      = GPIO_INTR_DISABLE,
        .mode           = GPIO_MODE_OUTPUT,
        .pull_down_en   = GPIO_PULLDOWN_DISABLE,
        .pull_up_en     = GPIO_PULLUP_ENABLE,
        .pin_bit_mask   = (1ULL << I2C_SDA_PIN) 
    };
    return gpio_config(&io_config);
}
 
 
void I2cStart(void)
{
	I2cSdaOut();
	I2C_SDA_HIGH;
	I2C_SCL_HIGH;
	usleep(4);
	I2C_SDA_LOW;
	usleep(4);
	I2C_SCL_LOW;
}
 
void I2cStop(void)
{
	I2cSdaOut();
	I2C_SCL_LOW;
	I2C_SDA_LOW;
	usleep(4);
	I2C_SCL_HIGH;
	I2C_SDA_HIGH;
	usleep(4);
}
 
uint8_t I2cWaitAck(void)
{
	uint8_t ErrTime = 0;
 
	I2C_SDA_HIGH;
	usleep(1);
	I2cSdaIn();
	I2C_SCL_HIGH;
	usleep(1);
	while(I2C_SDA == 1)
	{
		ErrTime++;
		if(ErrTime > 200)
		{
			I2cStop();
			return 1;
		}
	}
	I2C_SCL_LOW;
	return 0;
}
 
void I2cAck(void)
{
	I2cSdaOut();
	I2C_SCL_LOW;
	I2C_SDA_LOW;
	usleep(2);
	I2C_SCL_HIGH;
	usleep(2);
	I2C_SCL_LOW;
}
 
void I2cNack(void)
{
	I2cSdaOut();
	I2C_SCL_LOW;
	I2C_SDA_HIGH;
	usleep(2);
	I2C_SCL_HIGH;
	usleep(2);
	I2C_SCL_LOW;
}
 
void I2cWriteByte(uint8_t Data)
{
    uint8_t i;
 
    I2cSdaOut();
    I2C_SCL_LOW;
    for(i = 0;i < 8;i++)
    {
		if((Data & 0x80) >> 7)
		{
			I2C_SDA_HIGH;
		}
		else
		{
			I2C_SDA_LOW;
		}
		Data <<= 1;
		usleep(2);
		I2C_SCL_HIGH;
		usleep(2);
		I2C_SCL_LOW;
		usleep(2);
    }
}
 
uint8_t I2cReadByte(uint8_t isAck)
{
	uint8_t i;
	uint8_t Data = 0x0;
 
	I2cSdaIn();
 
    for(i = 0;i < 8;i++)
	{
    	I2C_SCL_LOW;
    	usleep(2);
		I2C_SCL_HIGH;
		Data <<= 1;
        if(I2C_SDA == 1)
		{
        	Data++;
		}
        usleep(1);
    }
    if(isAck)
	{
    	I2cNack();
	}
    else
	{
		I2cAck();
	}
    return Data;
}
 
void EspI2cWriteByte(uint8_t dev_addr,uint8_t reg_addr,uint8_t write_data)
{
	I2cStart();
	I2cWriteByte((dev_addr << 1) | I2C_MASTER_WRITE);
	I2cWaitAck();
	I2cWriteByte(reg_addr);
	I2cWaitAck();
	I2cWriteByte(write_data);
	I2cWaitAck();
	I2cStop();
}
 
void EspI2cReadByte(uint8_t dev_addr,uint8_t reg_addr,uint8_t *read_data)
{
	I2cStart();
	I2cWriteByte((dev_addr << 1) | I2C_MASTER_WRITE);
	I2cWaitAck();
	I2cWriteByte(reg_addr);
	I2cWaitAck();
 
	I2cStart();
	I2cWriteByte((dev_addr << 1) | I2C_MASTER_READ);
	*read_data = I2cReadByte(NACK_VAL);
	I2cStop();
}
 
void EspI2cWriteBytes(uint8_t dev_addr,uint8_t reg_addr,uint8_t *write_data,uint8_t write_len)
{
	uint8_t i;
 
	I2cStart();
 
	I2cWriteByte((dev_addr << 1) | I2C_MASTER_WRITE);
	I2cWaitAck();
 
	I2cWriteByte(reg_addr);
	I2cWaitAck();
 
    for(i = 0;i < write_len;i++)
    {
		I2cWriteByte(write_data[i]);
		I2cWaitAck();
    }
	I2cStop();
}
 
void EspI2cReadBytes(uint8_t dev_addr,uint8_t reg_addr,uint8_t *read_data,uint8_t read_len)
{
    uint8_t i = 0;
 
	I2cStart();
 
	I2cWriteByte((dev_addr << 1) | I2C_MASTER_WRITE);
	I2cWaitAck();
 
	I2cWriteByte(reg_addr);
	I2cWaitAck();
 
	I2cStart();
	I2cWriteByte((dev_addr << 1) | I2C_MASTER_READ);
	I2cWaitAck();
 
	if(read_len > 1)
	{
		for(i = 0;i < read_len - 1;i++)
		{
			read_data[i] = I2cReadByte(ACK_VAL);
		}
	}
	read_data[i] = I2cReadByte(NACK_VAL);
 
	I2cStop();
}
/// @brief 用于检测I2C设备
/// @param  
void I2cDetect(void)
{
	uint8_t i;
	uint8_t j;
    uint8_t address;
	uint8_t ret = 0;
 
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for(i = 0;i < 128;i += 16) 
	{
        printf("%02x: ", i);
        for (j = 0;j < 16;j++) 
		{
            fflush(stdout);
            address = i + j;
 
			I2cStart();
			I2cWriteByte((address << 1) | I2C_MASTER_WRITE);
			ret = I2cWaitAck();
			I2cStop();
			
            if(ret == 0) 
			{
                printf("%02x ",address);
            } 
			else 
			{
                printf("-- ");
            }
        }
        printf("\r\n");
    }
}
#endif 