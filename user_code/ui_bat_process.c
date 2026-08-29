#include "ui_bat_process.h"
#include "user_typedef.h"
#include "instrument.h"

volatile u16 ui_bat_process_time = 0;

/**
 * @brief 
 * 
 * @note 在1ms中断内调用
 * 
 */
void ui_bat_process_time_add(void)
{
    if (ui_bat_process_time < ((u16)-1)) {
        ui_bat_process_time++;
    }
}

/**
 * @brief 将电池电压转换为对应的格子数
 *
 * @param voltage 电池电压，单位：mV
 *
 * @return u8 电池格子数
 *          0：低电量提示
 *          1：没有这种情况
 *          2：2格，表示电池电压大于等于9V
 *          ...
 *          8：8格，表示电池电压大于等于16V
 *
 */
u8 bat_vol_to_lev(u16 voltage)
{
    u8 ret = 0;

    if (voltage >= BAT_VOLTAGE_OF_FULL_GRID) {
        ret = 8;
    } else if (voltage >= BAT_VOLTAGE_OF_LOW_GRID) {
        // 至少要点亮2格，表达式加了2
        ret = 2 + (voltage - BAT_VOLTAGE_OF_LOW_GRID) / 1000;
    } else {
        ret = 0;
    }

    return ret;
}

void ui_bat_process(void)
{
    u16 cur_bat_voltage; // 当前电池电压,单位:mV

    static u8 is_initialized = 0;              // 是否初始化过
    static volatile u8 bat_lev_to_display = 0; // 需要延迟显示的电池格子数
    u8 bat_lev = 0;                            // 电池格子数
    u8 bat_lev_diff = 0;                       // 电池格子数差值

    cur_bat_voltage = instrument.bat_voltage;

    if (0 == is_initialized) {
        is_initialized = 1;
        bat_lev = bat_vol_to_lev();
    }
}
