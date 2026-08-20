#ifndef __UI_H__
#define __UI_H__

#include "include.h"

// 处于设置界面，等待超时的时间：
#define UI_SETTING_TIME_OUT_CNT ((u16)2 * 12 * 400)
#define UI_SETTING_BLINK_PERIOD ((u16)400)


enum
{
    UI_STATE_NORMAL,
    UI_STATE_SETTING_DISTANCE_UNIT_TYPE, // 设置 要显示的单位类型 公制 或 英制

    UI_STATE_SETTING_TIME_MINUTE, // 设置 时间 分钟
    UI_STATE_SETTING_TIME_HOUR,   // 设置 时间 小时 
};
typedef u8 ui_state_t;

typedef struct
{
    u16 blink_timer_cnt;
    u16 state_timeout_cnt;
    u16 auto_exit_setting_time_cnt; // 自动退出设置界面的计时器

    u8 blink_dir;

    ui_state_t state;  
} ui_manager_t;

extern volatile ui_manager_t ui_manager;

void ui_manager_init(void);
void ui_timer_handle_isr(void);

void ui_set_state(ui_state_t state);

void ui_display_handle(void);
void ui_display_refresh(void);

#endif