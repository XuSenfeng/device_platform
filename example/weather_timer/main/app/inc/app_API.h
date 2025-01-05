/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-11-16 11:10:21
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2024-12-04 23:40:09
 */
#ifndef APP_API_H
#define APP_API_H


#include "esp_err.h"
void get_http_data_story(void);
void get_time(void);
void get_air_quality(void);
void get_now_weather(void);
void get_daily_weather(void);
void deal_story(int new_story, int next_page);
esp_err_t get_target_url(char *target_url);
void get_http_data_riddle(void);
void deal_riddle(int new_story, int next_page);
void get_http_data_poisonous_chicken_soup(void);
void get_http_data_constellation(const char *data);
void get_http_data_birthday_character(const char *data);
void deal_birthday_character(int new_story, int next_page);
#endif // APP_API_H