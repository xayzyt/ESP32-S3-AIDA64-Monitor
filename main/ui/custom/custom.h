/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void custom_init(lv_ui *ui);
void set_home_time(lv_ui *ui,int year,int month,int day,int w_day,int hour,int minute,int second);
void set_today_img(lv_ui* ui,const char* img_path,int low,int high);
void set_tomorrow_img(lv_ui* ui,const char* img_path,int low,int high);
void set_after_img(lv_ui* ui,const char* img_path,int low,int high);
void set_home_city(lv_ui* ui,const char* city);
//显示硬件参数
void set_monitor_param(lv_ui* ui,int cpu_rate,int cpu_temp,int mem_rate,int mem_use);
//显示连接状态
void set_monitor_connect_state(lv_ui* ui,bool flag);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
