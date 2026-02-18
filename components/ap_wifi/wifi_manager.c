#include "wifi_manager.h"
#include <stdio.h>
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "lwip/ip4_addr.h"
#define TAG     "wifi_manager"

//重连次数
#define MAX_CONNECT_RETRY   6
static int sta_connect_count = 0;

static esp_netif_t *esp_netif_sta = NULL;
static esp_netif_t *esp_netif_ap = NULL;

//AP模式下的SSID名称
static const char* ap_ssid_name = "ESP32-AP";

//AP模式下的密码
static const char* ap_password = "12345678";

//回调函数
static p_wifi_state_callback    wifi_state_cb = NULL;

//当前sta连接状态
static bool is_sta_connected = false;

/** 事件回调函数
 * @param arg   用户传递的参数
 * @param event_base    事件类别
 * @param event_id      事件ID
 * @param event_data    事件携带的数据
 * @return 无
*/
static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{   
    if(event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:      //WIFI以STA模式启动后触发此事件
        {
            wifi_mode_t mode;
            esp_wifi_get_mode(&mode);
            if(mode == WIFI_MODE_STA)
                esp_wifi_connect();         //启动WIFI连接
            break;
        }
        case WIFI_EVENT_STA_CONNECTED:  //WIFI连上路由器后，触发此事件
            ESP_LOGI(TAG, "Connected to AP");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:   //WIFI从路由器断开连接后触发此事件
            if(is_sta_connected)
            {
                if(wifi_state_cb)
                    wifi_state_cb(WIFI_STATE_DISCONNECTED);
                is_sta_connected = false;
            }
            if(sta_connect_count < MAX_CONNECT_RETRY)
            {
                wifi_mode_t mode;
                esp_wifi_get_mode(&mode);
                if(mode == WIFI_MODE_STA)
                    esp_wifi_connect();             //继续重连
                sta_connect_count++;
            }
            ESP_LOGI(TAG,"connect to the AP fail,retry now");
            break;
        case WIFI_EVENT_AP_STACONNECTED:
        {
            //有设备连接了热点，把它的MAC打印出来
            wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
            ESP_LOGI(TAG, "Station "MACSTR" joined, AID=%d",
                    MAC2STR(event->mac), event->aid);
            break;
        }
        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            //有设备断开了热点
            wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
            ESP_LOGI(TAG, "Station "MACSTR" left, AID=%d",
                    MAC2STR(event->mac), event->aid);
            break;
        }
        default:
            break;
        }
    }
    if(event_base == IP_EVENT)                  //IP相关事件
    {
        switch(event_id)
        {
            case IP_EVENT_STA_GOT_IP:           //只有获取到路由器分配的IP，才认为是连上了路由器
                ESP_LOGI(TAG,"Get ip address");
                is_sta_connected = true;
                if(wifi_state_cb)
                    wifi_state_cb(WIFI_STATE_CONNECTED);
                break;
            default:break;
        }
    }
}

/** 初始化wifi，默认进入STA模式
 * @param 无
 * @return 无 
*/
void wifi_manager_init(p_wifi_state_callback f)
{
    ESP_ERROR_CHECK(esp_netif_init());  //用于初始化tcpip协议栈
    ESP_ERROR_CHECK(esp_event_loop_create_default());       //创建一个默认系统事件调度循环，之后可以注册回调函数来处理系统的一些事件
    esp_netif_sta = esp_netif_create_default_wifi_sta();    //使用默认配置创建STA对象
    esp_netif_ap = esp_netif_create_default_wifi_ap();      //使用默认配置创建AP对象
    //初始化WIFI
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    //注册事件
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&event_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&event_handler,NULL));

    wifi_state_cb = f;
    //启动WIFI
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );         //设置工作模式为STA
    ESP_ERROR_CHECK(esp_wifi_start() );                         //启动WIFI
    
    ESP_LOGI(TAG, "wifi_init finished.");
}

/** 进入ap+sta模式
 * @param 无
 * @return 成功/失败
*/
esp_err_t wifi_manager_ap(void)
{
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if(mode == WIFI_MODE_APSTA) //需要使用AP+STA模式，才可以执行扫描同时保持客户端连接
        return ESP_OK;
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_APSTA);
    wifi_config_t wifi_config = 
    {
        .ap = 
        {
            .channel = 5,               //wifi的通信信道
            .max_connection = 2,        //最大连接数
            .authmode = WIFI_AUTH_WPA2_PSK, //加密方式
        }
    };
    //填充ap的ssid名称
    snprintf((char*)wifi_config.ap.ssid,31,"%s",ap_ssid_name);
    wifi_config.ap.ssid_len = strlen(ap_ssid_name);
    //填充密码
    snprintf((char*)wifi_config.ap.password,63,"%s",ap_password);

    //设置wifi
    esp_wifi_set_config(WIFI_IF_AP,&wifi_config);

    //如果是AP模式，则需要设置如下网络层信息
    esp_netif_ip_info_t ipInfo;
    IP4_ADDR(&ipInfo.ip, 192,168,100,1);    //本地的IP地址
	IP4_ADDR(&ipInfo.gw, 192,168,100,1);    //网关IP地址
	IP4_ADDR(&ipInfo.netmask, 255,255,255,0);   //子网掩码
	esp_netif_dhcps_stop(esp_netif_ap);
	esp_netif_set_ip_info(esp_netif_ap, &ipInfo);
	esp_netif_dhcps_start(esp_netif_ap);

    esp_wifi_start();
    return ESP_OK;
}

static SemaphoreHandle_t scan_sem = NULL;

/** 扫描任务
 * @param 无
 * @return 成功/失败
*/
static void scan_task(void* param)
{
    p_wifi_scan_callback callback = (p_wifi_scan_callback)param;
    uint16_t number = 20;
    wifi_ap_record_t *ap_info = malloc(sizeof(wifi_ap_record_t)*number);
    uint16_t ap_count = 0;
    ESP_LOGI(TAG,"Start wifi scan");
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);
    if(callback)
        callback(number,ap_info);
    xSemaphoreGive(scan_sem);
    vTaskDelete(NULL);
}

/** 启动扫描
 * @param 无
 * @return 成功/失败
*/
esp_err_t wifi_manager_scan(p_wifi_scan_callback f)
{
    if(!scan_sem)
    {
        scan_sem = xSemaphoreCreateBinary();
        xSemaphoreGive(scan_sem);
    }
    if(pdTRUE == xSemaphoreTake(scan_sem,0))
    {
        //清除上次的扫描信息
        esp_wifi_clear_ap_list();
        //启动一个扫描任务
        if(pdTRUE == xTaskCreatePinnedToCore(scan_task,"scan",8192,f,3,NULL,0))
            return ESP_OK;
    }
    return ESP_FAIL;
}

/** 连接wifi
 * @param ssid
 * @param password
 * @return 成功/失败
*/
esp_err_t wifi_manager_connect(const char* ssid,const char* password)
{
    sta_connect_count = 0;
    wifi_config_t wifi_config = 
    {
        .sta = 
        {
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK,   //加密方式
        },
    };
    snprintf((char*)wifi_config.sta.ssid,31,"%s",ssid);
    snprintf((char*)wifi_config.sta.password,63,"%s",password);
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if(mode != WIFI_MODE_STA)
    {
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_start();
    }
    else
    {
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    return ESP_OK;
}

/** 是否已经连接了路由器
 * @param 无
 * @return 是/否
*/
bool wifi_manager_is_connect(void)
{
    return is_sta_connected;
}
