/*
 * @Descripttion: 
 * @version: 
 * @Author: XvSenfeng
 * @Date: 2024-12-01 10:01:13
 * @LastEditors: XvSenfeng
 * @LastEditTime: 2025-01-05 09:47:11
 */
#ifndef USER_CONFIG_H
#define USER_CONFIG_H

/*********
 * music
 * *******/
// 默认的音量大小
#define DEFAULT_SOUND_VALUSE  50

/*********
 * SD卡
 * *******/
//挂载点
#define MOUNT_POINT              "/sdcard"

/***********
 * LCD显示屏
 * *********/
// 默认亮屏时间
#define DEFOULT_LCD_LIGHT_TIME  30
// 默认亮度
#define DEFOULT_LCD_LIGHT_VALUE 100
//默认的背景颜色
#define DEFAULT_LC_COLOR        0xf1e1e5

/***********
 * 其他
 * *********/
//默认文本框的单次显示字符数量
#define FILE_READ_BUF_SIZE      216
//是否使用中文
#define USING_CHINESE       1
// 最大扫描wifi个数
#define DEFAULT_SCAN_LIST_SIZE   10     
/***********
 * 使用的开发板
 * *********/
#define BOARD_C3            0
#define BOARD_S3            1

/***********
 * 开发板的模式
 * *********/
#define DEVICE_MODE             DEVICE_MASTER

/***********
 * API
 * *********/
#define BUF_TYPE_STORY  1
#define STORY_API_URL "http://apis.tianapi.com/story/index?key=bf966cb5e47fca5b8dc0427db8678f6d"

#define BUF_TYPE_RIDDLE 2
#define RIDDLE_API_URL "http://apis.tianapi.com/naowan/index?key=bf966cb5e47fca5b8dc0427db8678f6d&num=5"

#define BUF_TYPE_POISONOUS_CHICKEN_SOUP 3
#define POISONOUS_CHICKEN_SOUP_API_URL "http://apis.tianapi.com/dujitang/index?key=bf966cb5e47fca5b8dc0427db8678f6d"

#define BUF_TYPE_CONSTELLATION 4
#define CONSTELLATION_API_URL "http://apis.tianapi.com/star/index?key=bf966cb5e47fca5b8dc0427db8678f6d&astro="

#define BUF_TYPE_BIRTHDAY_CHARACTRE 5
#define BIRTHDAY_CHARACTRE_API_URL "http://apis.tianapi.com/dob/index?key=bf966cb5e47fca5b8dc0427db8678f6d"

/*获取天气*/
#define QWEATHER_DAILY_URL   "https://devapi.qweather.com/v7/weather/3d?&location=101010100&key=7be28a7cc429438abf4f1d5feeb9c910" 
#define QWEATHER_NOW_URL     "https://devapi.qweather.com/v7/weather/now?&location=101010100&key=7be28a7cc429438abf4f1d5feeb9c910" 
#define QAIR_NOW_URL         "https://devapi.qweather.com/v7/air/now?&location=101010100&key=7be28a7cc429438abf4f1d5feeb9c910" 

#endif