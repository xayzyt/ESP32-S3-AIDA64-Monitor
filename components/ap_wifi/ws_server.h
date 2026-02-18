#ifndef _WS_SERVER_H_
#define _WS_SERVER_H_
#include "esp_err.h"

//ws接收到的处理回调函数
typedef void(*ws_receive_cb)(uint8_t* payload,int len);

typedef struct
{
    const char* html_code;              //当执行http访问时返回的html页面
    ws_receive_cb   receive_fn;         //当ws接收到数据时，调用此函数
}ws_cfg_t;

/** 启动ws
 * @param cfg ws一些配置,请看ws_cfg_t定义
 * @return  ESP_OK or ESP_FAIL
*/
esp_err_t   web_ws_start(ws_cfg_t *cfg);

esp_err_t   web_ws_stop(void);

esp_err_t   web_ws_send(uint8_t* data, int len);

#endif
