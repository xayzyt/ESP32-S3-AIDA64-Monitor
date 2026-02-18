<<<<<<< HEAD
#include <stdio.h>
#include "lv_port.h"            // 包含上面的屏幕驱动接口
#include "xl9555.h"             // 包含IO扩展芯片接口
#include "driver/gpio.h"
#include "lv_demos.h"           // LVGL官方Demo（比如跑分、控件展示）
#include "esp_lvgl_port.h"      // 包含锁机制的接口
#include "ft6336u_driver.h"
#include "esp_log.h"
#include "gui_guider.h"
#include "custom.h"
#include "button.h"
#include "ap_wifi.h"
#include <time.h>
#include "esp_sntp.h"
#include "nvs_flash.h"
#include "weather.h"
#include "esp_spiffs.h"
// 定义引脚宏
#define XL9555_SDA  GPIO_NUM_10 // IO扩展芯片的I2C数据线
#define XL9555_SCL  GPIO_NUM_11 // IO扩展芯片的I2C时钟线
#define LCD_RST_IO IO1_3        // 屏幕复位引脚（在扩展芯片上）
#define LCD_BL_IO IO1_2         // 屏幕背光引脚（在扩展芯片上）
#define TAG "main"
lv_ui guider_ui;
static volatile uint16_t xl9555_button_level = 0xFFFF;

static void img_spiffs_init_buffer(void)
{
    //定义挂载点
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/img",            //挂载点
        .partition_label = "img",         //分区名称
        .max_files = 5,                    //最大打开的文件数
        .format_if_mount_failed = false    //挂载失败是否执行格式化
        };
    //挂载spiffs
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));
}
void xl9555_input_callback(uint16_t io_num,int level)
{
    if(io_num == IO1_1)
    {
        if(!level)
        {
            ft6336u_int_info(true);
        }
        else
        {
            ft6336u_int_info(false);
        }
    }
    if(level)
        xl9555_button_level |= io_num;
    else
        xl9555_button_level &= ~io_num;

}
int get_button_level(int gpio)
{
    return (xl9555_button_level&gpio)?1:0;
}

void long_press(int gpio)
{
    ap_wifi_apcfg(true);
}

void button_init(void)
{
    button_config_t button_cfg = 
    {
        .active_level = 0,
        .getlevel_cb = get_button_level,
        .gpio_num = IO0_1,
        .long_cb = long_press,
        .long_press_time = 3000,
        .short_cb = NULL,
    };
    button_event_set(&button_cfg);
}
static  void sntp_finish_cb(struct timeval *tv)
{
    struct tm t;
    localtime_r(&tv->tv_sec, &t);
    set_home_time(&guider_ui,t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_wday,t.tm_hour,t.tm_min,t.tm_sec);
}
void mysntp_init(void)
{
    if(!esp_sntp_enabled())
    {

        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0,"sntp.aliyun.com");
        esp_sntp_setservername(1,"time.asia.apple.com");
        esp_sntp_setservername(2,"pool.ntp.org");
        esp_sntp_set_time_sync_notification_cb(sntp_finish_cb);
        esp_sntp_init();
    }
    
}
void wifi_state_callback(WIFI_STATE state)
{
    if(state == WIFI_STATE_CONNECTED)
    {
        mysntp_init();
        ESP_LOGI(TAG,"wifi connected");
    }
}
void app_main(void)
{
    img_spiffs_init_buffer();
    nvs_flash_init();
    xl9555_init(XL9555_SDA, XL9555_SCL, GPIO_NUM_17, xl9555_input_callback);
    xl9555_ioconfig((~(LCD_RST_IO | LCD_BL_IO)) & 0xFFFF);
    xl9555_pin_write(LCD_BL_IO, 1); 
    lv_port_init();
    lvgl_port_lock(0);
    setup_ui(&guider_ui);
    custom_init(&guider_ui);
    lvgl_port_unlock();
    button_init();
    ap_wifi_init(wifi_state_callback);
    setenv("TZ","CST-8",1);
    tzset();
    weather_start();
    
}
=======
#include <stdio.h>
#include "lv_port.h"            // 包含上面的屏幕驱动接口
#include "xl9555.h"             // 包含IO扩展芯片接口
#include "driver/gpio.h"
#include "lv_demos.h"           // LVGL官方Demo（比如跑分、控件展示）
#include "esp_lvgl_port.h"      // 包含锁机制的接口
#include "ft6336u_driver.h"
#include "esp_log.h"
#include "gui_guider.h"
#include "custom.h"
#include "button.h"
#include "ap_wifi.h"
#include <time.h>
#include "esp_sntp.h"
#include "nvs_flash.h"
#include "weather.h"
#include "esp_spiffs.h"
// 定义引脚宏
#define XL9555_SDA  GPIO_NUM_10 // IO扩展芯片的I2C数据线
#define XL9555_SCL  GPIO_NUM_11 // IO扩展芯片的I2C时钟线
#define LCD_RST_IO IO1_3        // 屏幕复位引脚（在扩展芯片上）
#define LCD_BL_IO IO1_2         // 屏幕背光引脚（在扩展芯片上）
#define TAG "main"
lv_ui guider_ui;
static volatile uint16_t xl9555_button_level = 0xFFFF;

static void img_spiffs_init_buffer(void)
{
    //定义挂载点
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/img",            //挂载点
        .partition_label = "img",         //分区名称
        .max_files = 5,                    //最大打开的文件数
        .format_if_mount_failed = false    //挂载失败是否执行格式化
        };
    //挂载spiffs
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));
}
void xl9555_input_callback(uint16_t io_num,int level)
{
    if(io_num == IO1_1)
    {
        if(!level)
        {
            ft6336u_int_info(true);
        }
        else
        {
            ft6336u_int_info(false);
        }
    }
    if(level)
        xl9555_button_level |= io_num;
    else
        xl9555_button_level &= ~io_num;

}
int get_button_level(int gpio)
{
    return (xl9555_button_level&gpio)?1:0;
}

void long_press(int gpio)
{
    ap_wifi_apcfg(true);
}

void button_init(void)
{
    button_config_t button_cfg = 
    {
        .active_level = 0,
        .getlevel_cb = get_button_level,
        .gpio_num = IO0_1,
        .long_cb = long_press,
        .long_press_time = 3000,
        .short_cb = NULL,
    };
    button_event_set(&button_cfg);
}
static  void sntp_finish_cb(struct timeval *tv)
{
    struct tm t;
    localtime_r(&tv->tv_sec, &t);
    set_home_time(&guider_ui,t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_wday,t.tm_hour,t.tm_min,t.tm_sec);
}
void mysntp_init(void)
{
    if(!esp_sntp_enabled())
    {

        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0,"sntp.aliyun.com");
        esp_sntp_setservername(1,"time.asia.apple.com");
        esp_sntp_setservername(2,"pool.ntp.org");
        esp_sntp_set_time_sync_notification_cb(sntp_finish_cb);
        esp_sntp_init();
    }
    
}
void wifi_state_callback(WIFI_STATE state)
{
    if(state == WIFI_STATE_CONNECTED)
    {
        mysntp_init();
        ESP_LOGI(TAG,"wifi connected");
    }
}
void app_main(void)
{
    img_spiffs_init_buffer();
    nvs_flash_init();
    xl9555_init(XL9555_SDA, XL9555_SCL, GPIO_NUM_17, xl9555_input_callback);
    xl9555_ioconfig((~(LCD_RST_IO | LCD_BL_IO)) & 0xFFFF);
    xl9555_pin_write(LCD_BL_IO, 1); 
    lv_port_init();
    lvgl_port_lock(0);
    setup_ui(&guider_ui);
    custom_init(&guider_ui);
    lvgl_port_unlock();
    button_init();
    ap_wifi_init(wifi_state_callback);
    setenv("TZ","CST-8",1);
    tzset();
    weather_start();
    
}
>>>>>>> 81fc6d11b1a3aa3674e1ca4f6c48bf0d832d09d3
