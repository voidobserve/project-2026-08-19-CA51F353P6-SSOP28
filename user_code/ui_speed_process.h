#ifndef __UI_SPEED_PROCESS_H__
#define __UI_SPEED_PROCESS_H__

#include "user_typedef.h"

// 更新显示时速的周期，单位：ms
#define UI_SPEED_REFRESH_PERIOD (75)

void aip3368h_display_speed_refresh_time_add(void);

void aip3368h_display_speed_by_unit_type(u16 speed);
void ui_speed_process(void);

#endif