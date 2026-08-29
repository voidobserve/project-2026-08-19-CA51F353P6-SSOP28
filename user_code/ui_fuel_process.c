#include "ui_fuel_process.h"
#include "user_typedef.h"
#include "aip3368h_display.h"
#include "instrument.h"

volatile u16 ui_fuel_process_cnt;

void ui_fuel_process_cnt_add(void)
{
    if (ui_fuel_process_cnt < ((u16)-1)) {
        ui_fuel_process_cnt++;
    }
}

u8 fuel_percent_to_lev(u8 fuel_percent)
{
    u8 ret = 0;
    if (fuel_percent <= FUEL_LEV_0 || (fuel_percent == 0xFF)) {
        ret = 0;
    } else if (fuel_percent <= FUEL_LEV_1) {
        ret = 1;
    } else if (fuel_percent <= FUEL_LEV_2) {
        ret = 2;
    } else if (fuel_percent <= FUEL_LEV_3) {
        ret = 3;
    } else if (fuel_percent <= FUEL_LEV_4) {
        ret = 4;
    } else {
        ret = 5;
    }

    return ret;
}

void ui_fuel_process(void)
{
    static u8 is_initialized = 0;               //  是否已经初始化
    static volatile u8 fuel_lev_to_display = 0; // 最终要显示的油量等级
    volatile u8 cur_fuel_lev = 0;               // 当前油量等级
    volatile u8 fuel_lev_diff = 0; // 当前的油量等级与显示的油量等级之间的差值

    if (0 == is_initialized) {
        is_initialized = 1;
        cur_fuel_lev = fuel_percent_to_lev(instrument.fuel_percent);
        fuel_lev_to_display = cur_fuel_lev;

        if (0 == fuel_lev_to_display) {
            instrument.flag_is_in_warning_of_low_fuel = 1;
        } else {
            instrument.flag_is_in_warning_of_low_fuel = 0;
        }

        aip3368h_display_fuel_lev(fuel_lev_to_display);
    }

    cur_fuel_lev = fuel_percent_to_lev(instrument.fuel_percent);
    if (cur_fuel_lev != fuel_lev_to_display) {
        if (cur_fuel_lev > fuel_lev_to_display) {
            fuel_lev_diff = cur_fuel_lev - fuel_lev_to_display;
        } else {
            fuel_lev_diff = fuel_lev_to_display - cur_fuel_lev;
        }
    }

    if ((fuel_lev_diff >= 2 && ui_fuel_process_cnt < FUEL_UPDATE_TIME) ||
        (fuel_lev_diff == 1 && ui_fuel_process_cnt < FUEL_UPDATE_TIME_EXTEND)) {
        // 没有到油量挡位更新时间，直接返回
        return;
    }

    ui_fuel_process_cnt = 0;

    if (fuel_lev_to_display < cur_fuel_lev) {
        fuel_lev_to_display++;
    } else if (fuel_lev_to_display > cur_fuel_lev) {
        fuel_lev_to_display--;
    }

    if (fuel_lev_to_display == 0) {
        // 打开低油量报警 
        if (instrument.flag_is_in_warning_of_low_fuel == 0) {
            // 如果之前没有进入低油量报警
            aip3368h_display_fuel_lev(0); // 清空油量显示
            instrument.flag_is_in_warning_of_low_fuel = 1;
        }
    } else {
        // 关闭低油量报警
        instrument.flag_is_in_warning_of_low_fuel = 0;
        // 正常显示油量
        aip3368h_display_fuel_lev(fuel_lev_to_display); 
    }
}
