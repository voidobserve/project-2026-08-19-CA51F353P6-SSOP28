#include "ui_bat_process.h"
#include "user_typedef.h"
#include "instrument.h"
#include "aip3368h_display.h"

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
 *          1：1格
 *          2：2格 
 *          ...
 *          7：7格，表示电池电压大于等于16V
 *  
 *          8V -> 1 格
 *          12V -> 4 格
 *          16V -> 7 格
 *
 */
u8 bat_vol_to_lev(u16 voltage)
{
    u8 ret = 0;

    if (voltage >= BAT_VOLTAGE_OF_FULL_GRID) {
        ret = 7;
#if BAT_SHOW_1_GRID_LED
    } else if (BAT_SHOW_1_GRID_LED && voltage >= BAT_VOLTAGE_OF_MIN_GRID) {
        /* 
            线性插值：8V ~ 16V 差值共 8V ，对应 1 ~ 7 格，
            有如下线性关系：
            (16000 - 8000) / (7 - 1) == 8000 / 6 
            每mV对应 6 / 8000 格 
            公式里的 + 4000 作为四舍五入
        */
        ret = 1 + (((u32)voltage - BAT_VOLTAGE_OF_MIN_GRID) * 6 + 4000) / 8000;
        if (ret > 7) {
            ret = 7;
        }
#endif
    } else if (voltage >= BAT_VOLTAGE_OF_LOW_GRID) {
        /*
            关闭 1 格显示时，9V 起从 2 格开始
            有如下线性关系：
            (16000 - 9000) / (7 - 2) == 7000 / 5
            每mV对应 5 / 7000 格
            公式里的 + 3500 作为四舍五入
        */
        ret = 2 + (((u32)voltage - BAT_VOLTAGE_OF_LOW_GRID) * 5 + 3500) / 7000;
        if (ret > 7) {
            ret = 7;
        }
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
        bat_lev = bat_vol_to_lev(cur_bat_voltage);
        bat_lev_to_display = bat_lev;

        if (0 == bat_lev_to_display) {
            // 电池格子数为0，进入低电量提示
            instrument.flag_is_in_warning_of_low_battery = 1;
        }

        aip3368h_display_bat_lev_light(bat_lev_to_display);
    }

    bat_lev = bat_vol_to_lev(cur_bat_voltage);
    if (bat_lev_to_display != bat_lev) {
        if (bat_lev > bat_lev_to_display) {
            bat_lev_diff = bat_lev - bat_lev_to_display;
        } else {
            bat_lev_diff = bat_lev_to_display - bat_lev;
        }
    }

    if (bat_lev_diff == 0) {
        // 电池格子数没有变化，清空时间计数，再返回
        ui_bat_process_time = 0;
        return;
    } else if ((bat_lev_diff >= 2 &&
                ui_bat_process_time < BAT_DISPLAY_UPDATE_PERIOD) ||
               (bat_lev_diff <= 1 &&
                ui_bat_process_time <
                    BAT_DISPLAY_UPDATE_PERIOD_WHEN_BAT_CLOSE)) {
        // 电池电量格子数有变化，但是更新显示的时间没有到来，直接返回
        return;
    }

    ui_bat_process_time = 0;

    if (bat_lev_to_display < bat_lev) {
        bat_lev_to_display++;
    } else if (bat_lev_to_display > bat_lev) {
        bat_lev_to_display--;
    }

    if (0 == bat_lev_to_display) {
        // 打开低电量提示
        instrument.flag_is_in_warning_of_low_battery = 1;
    } else {
        instrument.flag_is_in_warning_of_low_battery = 0;
    }

    aip3368h_display_bat_lev_light(bat_lev_to_display);
}
