/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"
#include "aida64.h"
#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif


static void screen_home_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        switch(dir) {
        case LV_DIR_LEFT:
        {
            lv_indev_wait_release(lv_indev_active());
            ui_load_scr_animation(&guider_ui, &guider_ui.screen_monitor, guider_ui.screen_monitor_del, &guider_ui.screen_home_del, setup_scr_screen_monitor, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, false, false);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_screen_home (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_home, screen_home_event_handler, LV_EVENT_ALL, ui);
}

static void screen_monitor_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        switch(dir) {
        case LV_DIR_RIGHT:
        {
            lv_indev_wait_release(lv_indev_active());
            ui_load_scr_animation(&guider_ui, &guider_ui.screen_home, guider_ui.screen_home_del, &guider_ui.screen_monitor_del, setup_scr_screen_home, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, false, false);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void screen_monitor_btn_connect_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui* ui = (lv_ui*)lv_event_get_user_data(e);

    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(!aida64_monitor_isconnect())
        {
            const char* ip = lv_textarea_get_text(ui->screen_monitor_ta_ip);
            aida64_monitor_start(ip);
        }
        else
        {
            aida64_monitor_stop();
        }
        break;
    }
    default:
        break;
    }
}

void events_init_screen_monitor (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_monitor, screen_monitor_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_monitor_btn_connect, screen_monitor_btn_connect_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}
