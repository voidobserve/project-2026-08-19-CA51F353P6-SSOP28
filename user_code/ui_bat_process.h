#ifndef __UI_BAT_PROCESS_H__
#define __UI_BAT_PROCESS_H__

// 电池电压，满格子数时，对应的电压
#define BAT_VOLTAGE_OF_FULL_GRID ((u16)16000)
/*
    电池电压，低电量提示时，对应的电压
    小于这个阈值，才进行低电量提示，
    大于这个阈值，不进行低电量提示
*/
#define BAT_VOLTAGE_OF_LOW_GRID ((u16)9000) 
 
 
// 显示时，更新电池电压的周期，单位：ms
#define BAT_DISPLAY_UPDATE_PERIOD ((u16)3000)
// 显示时，如果电池电压比较接近，更新电池电压的周期，单位：ms
#define BAT_DISPLAY_UPDATE_PERIOD_WHEN_BAT_CLOSE ((u16)10000)

void ui_bat_process_time_add(void);
void ui_bat_process(void);

#endif

