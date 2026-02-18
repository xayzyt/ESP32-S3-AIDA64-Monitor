
#include "aida64.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "custom.h"
#define TAG "AIDA64"
extern lv_ui guider_ui;
static TaskHandle_t aida64_task_handle = NULL;
static char host_aida64_url[128];
static esp_http_client_handle_t http_handle = NULL;
#define AIDA64_CONNECT_BIT BIT0
static EventGroupHandle_t aida64_event_handle = NULL;
static bool is_http_connect = false;
static void aida64_monitor_task(void *param);
void aida64_monitor_start(const char* ip)
{
    if(aida64_event_handle == NULL)
    {
        aida64_event_handle = xEventGroupCreate();
    }
    if(aida64_task_handle == NULL)
    {
        xTaskCreatePinnedToCore(aida64_monitor_task,"AIDA64",8192,NULL,3,&aida64_task_handle,1);

    }
    snprintf(host_aida64_url,sizeof(host_aida64_url),"http://%s:80/sse",ip);
    xEventGroupSetBits(aida64_event_handle,AIDA64_CONNECT_BIT);
    
}

void aida64_monitor_stop(void)
{
    esp_http_client_close(http_handle);
}

int aida64_monitor_isconnect(void)
{
    return is_http_connect;
}

static bool aida64_monitor_parse(char* data,aida64_data_t* aida64_data)
{
    const char* search_str = NULL;
    if(!data)
    {
        return false;
    }
    search_str = strstr(data,"CPU_Rate");
    if (search_str)
    {
        sscanf(search_str,"%*[^0-9]%d",&aida64_data->cpu_rate);
    }
    else
    {
        return false;
    }
    search_str = strstr(data,"CPU_Temp");
    if (search_str)
    {
        sscanf(search_str,"%*[^0-9]%d",&aida64_data->cpu_temp);
    }
    else
    {
        return false;
    }
    search_str = strstr(data,"MEM_Rate");
    if (search_str)
    {
        sscanf(search_str,"%*[^0-9]%d",&aida64_data->mem_rate);
    }
    else
    {
        return false;
    }
    search_str = strstr(data,"MEM_Use");
    if (search_str)
    {
        sscanf(search_str,"%*[^0-9]%d",&aida64_data->mem_use);
    }
    else
    {
        return false;
    }
    return true;
    
}

static esp_err_t aida64_http_event_handler(esp_http_client_event_t *evt)
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
            is_http_connect = true;
            set_monitor_connect_state(&guider_ui,true);
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
            aida64_data_t aida64_data;
            if(aida64_monitor_parse((char *)evt->data,&aida64_data))
            {
                set_monitor_param(&guider_ui,aida64_data.cpu_rate,aida64_data.cpu_temp,aida64_data.mem_rate,aida64_data.mem_use);
            }

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
static void aida64_monitor_task(void *param)
{
     // 配置 HTTP 客户端
    esp_http_client_config_t config = {
        .url = "http://192.168.1.2:80/sse",                         // 设置请求地址
        .event_handler = aida64_http_event_handler, // 设置上面的回调函数处理响应
    };
    
    http_handle = esp_http_client_init(&config);   
    
    
    while(1)
    {
        //esp_err_t ret = esp_http_client_perform(http_handle);
        EventBits_t ev = xEventGroupWaitBits(aida64_event_handle, AIDA64_CONNECT_BIT, pdTRUE, pdFALSE,pdMS_TO_TICKS(1000));
        if(ev & AIDA64_CONNECT_BIT)
        {
            esp_http_client_set_url(http_handle,host_aida64_url);
            esp_http_client_set_method(http_handle,HTTP_METHOD_GET);
            esp_http_client_set_header(http_handle,"Accept","text/event-stream");
            ESP_LOGI(TAG,"http perform");
            esp_http_client_perform(http_handle);
            is_http_connect = false;
            set_monitor_connect_state(&guider_ui,false);
        }
    }
}
