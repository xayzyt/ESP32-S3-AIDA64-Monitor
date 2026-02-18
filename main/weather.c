<<<<<<< HEAD
#include "esp_http_client.h"
#include "esp_log.h"
#include "weather.h"
#include <string.h>
#include "cJSON.h"
#define TAG "weather"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_manager.h"
#include "custom.h"
#define WEATHER_BUFF_LEN 1024
#define WEATHER_PRIVATE_KEY "SRkeAiVJqr1pldHeT"
extern lv_ui guider_ui;
static char city_name[48]; 
//http接收到的数据
static uint8_t weather_data_buff[WEATHER_BUFF_LEN];
//htttp接收到的数据长度
static int weather_data_size = 0;

// 【回调函数】HTTP 事件处理
// 当 HTTP 请求连接、发送数据、收到数据、断开时，系统会自动调用这个函数
static esp_err_t weather_http_event_handler(esp_http_client_event_t *evt)
{
    // 根据事件 ID 判断发生了什么事
    switch(evt->event_id) {
        
        // 事件：HTTP 发生错误
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR"); // 打印调试日志
            break;
            
        // 事件：HTTP 连接成功（TCP 握手完成）
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
            
        // 事件：HTTP 请求头已发送给服务器
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
            
        // 事件：收到服务器发回的 HTTP 响应头 (Header)
        case HTTP_EVENT_ON_HEADER:
            // 打印收到的 Header 键值对
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
            
        // 事件：收到服务器发回的数据体 (Body)
        // 这里的 Body 通常是 OneNET 返回的 JSON 字符串
        case HTTP_EVENT_ON_DATA:
        {
            // 打印本次收到的数据长度
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // 打印数据内容
            printf("HTTP_EVENT_ON_DATA data = %.*s\r\n", evt->data_len, (char *)evt->data);
            
            // 定义本次要拷贝的长度
            int copy_len = 0;
            
            // 【防止缓冲区溢出逻辑】
            // 如果本次收到的数据长度 > (缓冲区总大小 - 已经存入的大小)
            if(evt->data_len > WEATHER_BUFF_LEN - weather_data_size)
            {
                // 只拷贝剩下的空间能装下的部分
                copy_len = WEATHER_BUFF_LEN - weather_data_size;
            }
            else
            {
                // 空间足够，拷贝全部收到的数据
                copy_len = evt->data_len;
            }
            
            // 将数据从事件结构体 (evt->data) 复制到我们的静态缓冲区 (weather_data_buff)
            memcpy(&weather_data_buff[weather_data_size], evt->data, copy_len);
            
            // 更新偏移量，加上本次拷贝的长度
            weather_data_size += copy_len;
            break;
        }
        
        // 事件：HTTP 会话结束
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
            
        // 事件：HTTP 连接断开
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
            
        // 事件：发生重定向
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;
            
        default:
            break;
    }
    return ESP_OK; // 返回成功
}
static esp_err_t pasre_weather(char* weather_data)
{
    weather_data_pkt_t data[8];
    char city[48];
    cJSON *wt_js = cJSON_Parse(weather_data);
    if(!wt_js)
    {
        ESP_LOGI(TAG, "cJSON_Parse error");
        return ESP_FAIL;
    }
    cJSON *result_js = cJSON_GetObjectItem(wt_js,"results");
    if(!result_js)
    {
        ESP_LOGI(TAG, "cJSON_GetObjectItem error");
        return ESP_FAIL;
    }
    cJSON *result_child_js = result_js->child;
    cJSON *location_js = cJSON_GetObjectItem(result_child_js,"location");
    cJSON *name_js = cJSON_GetObjectItem(location_js,"name");
    if(name_js)
    {
        snprintf(city,sizeof(city),"%s",cJSON_GetStringValue(name_js));
    }
    cJSON *daily_js = cJSON_GetObjectItem(result_child_js,"daily");
    
    int index = 0;
    if(daily_js)
    {
        cJSON *daily_child_js = daily_js->child;
        while(daily_child_js)
        {
            cJSON * high_js = cJSON_GetObjectItem(daily_child_js,"high");
            cJSON * low_js = cJSON_GetObjectItem(daily_child_js,"low");
            cJSON * code_js = cJSON_GetObjectItem(daily_child_js,"code_day");
            if(index<3)
            {
                sscanf(cJSON_GetStringValue(high_js),"%d",&data[index].high_temp);
                sscanf(cJSON_GetStringValue(low_js),"%d",&data[index].low_temp);
                snprintf(data[index].code,4,"%s",cJSON_GetStringValue(code_js));
                ESP_LOGI(TAG,"day[%d]->high temp:%d,low temp:%d,code:%s",index,data[index].high_temp,data[index].low_temp,data[index].code);
            }
            index++;
            daily_child_js = daily_child_js->next;
        }
        //数据设置到显示界面去
        char img_path[64];
        snprintf(img_path,sizeof(img_path),"/img/%s@1x.png",data[0].code);
        set_today_img(&guider_ui,img_path,data[0].low_temp,data[0].high_temp);

        snprintf(img_path,sizeof(img_path),"/img/%s@1x.png",data[1].code);
        set_tomorrow_img(&guider_ui,img_path,data[1].low_temp,data[1].high_temp);

        snprintf(img_path,sizeof(img_path),"/img/%s@1x.png",data[2].code);
        set_after_img(&guider_ui,img_path,data[2].low_temp,data[2].high_temp);

        set_home_city(&guider_ui,city);
    }
    cJSON_Delete(wt_js);
    return ESP_OK;
}

// 【内部函数】发起 HTTP 连接的通用封装
// 参数：URL，HTTP方法(GET/POST)，POST的数据载荷(payload)
static esp_err_t weather_http_connect(void)
{
    static char url[256];
    char *loc_param = (strlen(city_name) > 0) ? city_name : "ip";
    snprintf(url,sizeof(url),
        "http://api.seniverse.com/v3/weather/daily.json?key=%s&location=%s&language=zh-Hans&unit=c&start=0&days=3",
        WEATHER_PRIVATE_KEY, 
        loc_param
    );
    // 配置 HTTP 客户端
    esp_http_client_config_t config = {
        .url = url,                         // 设置请求地址
        .event_handler = weather_http_event_handler, // 设置上面的回调函数处理响应
    };
    // 初始化客户端句柄
    esp_http_client_handle_t client = esp_http_client_init(&config);   
    
    memset(weather_data_buff,0,WEATHER_BUFF_LEN);

    weather_data_size = 0;

    // 【执行】发起 HTTP 请求（阻塞直到完成）
    esp_err_t ret = esp_http_client_perform(client);

    pasre_weather((char*)weather_data_buff);
    
    
    // 销毁客户端句柄，释放资源
    esp_http_client_cleanup(client);
    return ret; // 返回执行结果
}

static esp_err_t parse_location(char * location_data)
{
    cJSON *location_js = cJSON_Parse(location_data);
    if(!location_js)
    {
        ESP_LOGI(TAG, "cJSON_Parse error");
        return ESP_FAIL;
    }
    cJSON *city_js = cJSON_GetObjectItem(location_js,"city");
    if(!city_js)
    {
        ESP_LOGI(TAG, "cJSON_GetObjectItem error");
        return ESP_FAIL;
    }
    snprintf(city_name,sizeof(city_name),"%s",cJSON_GetStringValue(city_js));
    ESP_LOGI(TAG,"city:%s",city_name);
    cJSON_Delete(location_js);
    return ESP_OK;
}

static esp_err_t location_http_connect(void)
{
    static char url[256];
    
    snprintf(url,sizeof(url),"http://ip-api.com/json/?lang=en");
    // 配置 HTTP 客户端
    esp_http_client_config_t config = {
        .url = url,                         // 设置请求地址
        .event_handler = weather_http_event_handler, // 设置上面的回调函数处理响应
    };
    // 初始化客户端句柄
    esp_http_client_handle_t client = esp_http_client_init(&config);   
    memset(weather_data_buff,0,WEATHER_BUFF_LEN);
    weather_data_size = 0;
    // 【执行】发起 HTTP 请求（阻塞直到完成）
    esp_err_t ret = esp_http_client_perform(client);
    parse_location((char*)weather_data_buff);

    // 销毁客户端句柄，释放资源
    esp_http_client_cleanup(client);
    return ret; // 返回执行结果
}
static void weather_task(void *param)
{
    while(1)
    {
        if(!wifi_manager_is_connect())
        {
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        else
        {
            //location_http_connect();
            weather_http_connect();
            vTaskDelay(pdMS_TO_TICKS(1000*1800));
        }

    }
}
void weather_start(void)
{
    xTaskCreatePinnedToCore(weather_task,"weather",8192,NULL,2,NULL,1);

}
=======
#include "esp_http_client.h"
#include "esp_log.h"
#include "weather.h"
#include <string.h>
#include "cJSON.h"
#define TAG "weather"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_manager.h"
#include "custom.h"
#define WEATHER_BUFF_LEN 1024
#define WEATHER_PRIVATE_KEY "SRkeAiVJqr1pldHeT"
extern lv_ui guider_ui;
static char city_name[48]; 
//http接收到的数据
static uint8_t weather_data_buff[WEATHER_BUFF_LEN];
//htttp接收到的数据长度
static int weather_data_size = 0;

// 【回调函数】HTTP 事件处理
// 当 HTTP 请求连接、发送数据、收到数据、断开时，系统会自动调用这个函数
static esp_err_t weather_http_event_handler(esp_http_client_event_t *evt)
{
    // 根据事件 ID 判断发生了什么事
    switch(evt->event_id) {
        
        // 事件：HTTP 发生错误
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR"); // 打印调试日志
            break;
            
        // 事件：HTTP 连接成功（TCP 握手完成）
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
            
        // 事件：HTTP 请求头已发送给服务器
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
            
        // 事件：收到服务器发回的 HTTP 响应头 (Header)
        case HTTP_EVENT_ON_HEADER:
            // 打印收到的 Header 键值对
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
            
        // 事件：收到服务器发回的数据体 (Body)
        // 这里的 Body 通常是 OneNET 返回的 JSON 字符串
        case HTTP_EVENT_ON_DATA:
        {
            // 打印本次收到的数据长度
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // 打印数据内容
            printf("HTTP_EVENT_ON_DATA data = %.*s\r\n", evt->data_len, (char *)evt->data);
            
            // 定义本次要拷贝的长度
            int copy_len = 0;
            
            // 【防止缓冲区溢出逻辑】
            // 如果本次收到的数据长度 > (缓冲区总大小 - 已经存入的大小)
            if(evt->data_len > WEATHER_BUFF_LEN - weather_data_size)
            {
                // 只拷贝剩下的空间能装下的部分
                copy_len = WEATHER_BUFF_LEN - weather_data_size;
            }
            else
            {
                // 空间足够，拷贝全部收到的数据
                copy_len = evt->data_len;
            }
            
            // 将数据从事件结构体 (evt->data) 复制到我们的静态缓冲区 (weather_data_buff)
            memcpy(&weather_data_buff[weather_data_size], evt->data, copy_len);
            
            // 更新偏移量，加上本次拷贝的长度
            weather_data_size += copy_len;
            break;
        }
        
        // 事件：HTTP 会话结束
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
            
        // 事件：HTTP 连接断开
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
            
        // 事件：发生重定向
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;
            
        default:
            break;
    }
    return ESP_OK; // 返回成功
}
static esp_err_t pasre_weather(char* weather_data)
{
    weather_data_pkt_t data[8];
    char city[48];
    cJSON *wt_js = cJSON_Parse(weather_data);
    if(!wt_js)
    {
        ESP_LOGI(TAG, "cJSON_Parse error");
        return ESP_FAIL;
    }
    cJSON *result_js = cJSON_GetObjectItem(wt_js,"results");
    if(!result_js)
    {
        ESP_LOGI(TAG, "cJSON_GetObjectItem error");
        return ESP_FAIL;
    }
    cJSON *result_child_js = result_js->child;
    cJSON *location_js = cJSON_GetObjectItem(result_child_js,"location");
    cJSON *name_js = cJSON_GetObjectItem(location_js,"name");
    if(name_js)
    {
        snprintf(city,sizeof(city),"%s",cJSON_GetStringValue(name_js));
    }
    cJSON *daily_js = cJSON_GetObjectItem(result_child_js,"daily");
    
    int index = 0;
    if(daily_js)
    {
        cJSON *daily_child_js = daily_js->child;
        while(daily_child_js)
        {
            cJSON * high_js = cJSON_GetObjectItem(daily_child_js,"high");
            cJSON * low_js = cJSON_GetObjectItem(daily_child_js,"low");
            cJSON * code_js = cJSON_GetObjectItem(daily_child_js,"code_day");
            if(index<3)
            {
                sscanf(cJSON_GetStringValue(high_js),"%d",&data[index].high_temp);
                sscanf(cJSON_GetStringValue(low_js),"%d",&data[index].low_temp);
                snprintf(data[index].code,4,"%s",cJSON_GetStringValue(code_js));
                ESP_LOGI(TAG,"day[%d]->high temp:%d,low temp:%d,code:%s",index,data[index].high_temp,data[index].low_temp,data[index].code);
            }
            index++;
            daily_child_js = daily_child_js->next;
        }
        //数据设置到显示界面去
        char img_path[64];
        snprintf(img_path,sizeof(img_path),"/img/%s@1x.png",data[0].code);
        set_today_img(&guider_ui,img_path,data[0].low_temp,data[0].high_temp);

        snprintf(img_path,sizeof(img_path),"/img/%s@1x.png",data[1].code);
        set_tomorrow_img(&guider_ui,img_path,data[1].low_temp,data[1].high_temp);

        snprintf(img_path,sizeof(img_path),"/img/%s@1x.png",data[2].code);
        set_after_img(&guider_ui,img_path,data[2].low_temp,data[2].high_temp);

        set_home_city(&guider_ui,city);
    }
    cJSON_Delete(wt_js);
    return ESP_OK;
}

// 【内部函数】发起 HTTP 连接的通用封装
// 参数：URL，HTTP方法(GET/POST)，POST的数据载荷(payload)
static esp_err_t weather_http_connect(void)
{
    static char url[256];
    char *loc_param = (strlen(city_name) > 0) ? city_name : "ip";
    snprintf(url,sizeof(url),
        "http://api.seniverse.com/v3/weather/daily.json?key=%s&location=%s&language=zh-Hans&unit=c&start=0&days=3",
        WEATHER_PRIVATE_KEY, 
        loc_param
    );
    // 配置 HTTP 客户端
    esp_http_client_config_t config = {
        .url = url,                         // 设置请求地址
        .event_handler = weather_http_event_handler, // 设置上面的回调函数处理响应
    };
    // 初始化客户端句柄
    esp_http_client_handle_t client = esp_http_client_init(&config);   
    
    memset(weather_data_buff,0,WEATHER_BUFF_LEN);

    weather_data_size = 0;

    // 【执行】发起 HTTP 请求（阻塞直到完成）
    esp_err_t ret = esp_http_client_perform(client);

    pasre_weather((char*)weather_data_buff);
    
    
    // 销毁客户端句柄，释放资源
    esp_http_client_cleanup(client);
    return ret; // 返回执行结果
}

static esp_err_t parse_location(char * location_data)
{
    cJSON *location_js = cJSON_Parse(location_data);
    if(!location_js)
    {
        ESP_LOGI(TAG, "cJSON_Parse error");
        return ESP_FAIL;
    }
    cJSON *city_js = cJSON_GetObjectItem(location_js,"city");
    if(!city_js)
    {
        ESP_LOGI(TAG, "cJSON_GetObjectItem error");
        return ESP_FAIL;
    }
    snprintf(city_name,sizeof(city_name),"%s",cJSON_GetStringValue(city_js));
    ESP_LOGI(TAG,"city:%s",city_name);
    cJSON_Delete(location_js);
    return ESP_OK;
}

static esp_err_t location_http_connect(void)
{
    static char url[256];
    
    snprintf(url,sizeof(url),"http://ip-api.com/json/?lang=en");
    // 配置 HTTP 客户端
    esp_http_client_config_t config = {
        .url = url,                         // 设置请求地址
        .event_handler = weather_http_event_handler, // 设置上面的回调函数处理响应
    };
    // 初始化客户端句柄
    esp_http_client_handle_t client = esp_http_client_init(&config);   
    memset(weather_data_buff,0,WEATHER_BUFF_LEN);
    weather_data_size = 0;
    // 【执行】发起 HTTP 请求（阻塞直到完成）
    esp_err_t ret = esp_http_client_perform(client);
    parse_location((char*)weather_data_buff);

    // 销毁客户端句柄，释放资源
    esp_http_client_cleanup(client);
    return ret; // 返回执行结果
}
static void weather_task(void *param)
{
    while(1)
    {
        if(!wifi_manager_is_connect())
        {
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        else
        {
            //location_http_connect();
            weather_http_connect();
            vTaskDelay(pdMS_TO_TICKS(1000*1800));
        }

    }
}
void weather_start(void)
{
    xTaskCreatePinnedToCore(weather_task,"weather",8192,NULL,2,NULL,1);

}
>>>>>>> 81fc6d11b1a3aa3674e1ca4f6c48bf0d832d09d3
