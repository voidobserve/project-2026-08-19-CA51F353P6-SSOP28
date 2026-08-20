#include "ui.h"
// #include "user_include.h"
#include "user_config.h"
#include "aip3368h_display.h"

#include "speed_scan.h"
#include "engine_speed_scan.h"
#include "aip1302.h"

// 错误处理函数的调用周期计数值：
static volatile u16 ui_display_err_time_cnt = 0;

volatile ui_manager_t ui_manager = {0};

void ui_manager_init(void)
{
    ui_manager.state = UI_STATE_NORMAL;
}

/**
 * @brief ui定时器处理
 *
 * @attention 在定时器调用，负责所有ui相关的计数器处理
 *
 */
void ui_timer_handle_isr(void)
{
    if (aip1302_update_time_interval < ((u16)-1) &&
        ui_manager.state == UI_STATE_NORMAL) {
        aip1302_update_time_interval++;
    } else {
        aip1302_update_time_interval = 0;
    }

    aip3368h_refresh_time_add(); // 控制将显存数据刷新到屏幕驱动ic的周期
#if SPEED_SCAN_ENABLE
    // 递增 AIP3368H 显示 速度 刷新时间计数
    aip3368h_display_speed_refresh_time_add();
#endif
    // 递增 AIP3368H 显示 发动机转速 刷新时间计数
    aip3368h_display_engine_speed_refresh_time_add();

    if (ui_display_err_time_cnt < ((u16)-1)) {
        ui_display_err_time_cnt++;
    }

    if (ui_manager.state == UI_STATE_SETTING_DISTANCE_UNIT_TYPE ||
        ui_manager.state == UI_STATE_SETTING_TIME_MINUTE ||
        ui_manager.state == UI_STATE_SETTING_TIME_HOUR) {
        if (ui_manager.blink_timer_cnt < ((u16)-1)) {
            ui_manager.blink_timer_cnt++;
        }

        if (ui_manager.auto_exit_setting_time_cnt < ((u16)-1)) {
            ui_manager.auto_exit_setting_time_cnt++;
        }
    } else {
        ui_manager.blink_timer_cnt = 0;
        ui_manager.auto_exit_setting_time_cnt = 0;
    }
}

// 设置ui状态（切换ui）
void ui_set_state(ui_state_t state)
{
    ui_manager.state = state;
    ui_manager.blink_dir = 0; // 初始化闪烁方向
}

/**
 * @brief 显示错误提示，由 ui_display_handle() 调用
 *
 */
void ui_display_err_handle(void)
{
    static volatile u8 display_err_dir = 0;

    if (ui_display_err_time_cnt < 475) {
        return;
    } else {
        ui_display_err_time_cnt = 0;
    }

    // 低油量 提示
    if (instrument.flag_is_in_warning_of_low_fuel) {
        // 直接操作显存，判断指示灯是否点亮，进而让它闪烁
        // 让第 0 格油量的指示灯和油量图标指示灯一起闪烁

        // if ((aip3368h_engine_speed_panel_display_buff[2] >> 11) & 0x01) {
        if (0 == display_err_dir) {
            // 油量 图标
            aip3368h_engine_speed_panel_display_buff[1] &= ~(0x01 << 8);
            // 油量 第 0 格 指示灯
            aip3368h_engine_speed_panel_display_buff[2] &= ~(0x01 << 11);
        } else {
            // 油量 图标
            aip3368h_engine_speed_panel_display_buff[1] |= (0x01 << 8);
            // 油量 第 0 格 指示灯
            aip3368h_engine_speed_panel_display_buff[2] |= (0x01 << 11);
        }
    }

    // 低电量提示
    if (instrument.flag_is_in_warning_of_low_battery) {
        // 直接操作显存，判断指示灯是否点亮，进而让它闪烁
        // 让第 0 格电量的指示灯、电池8字样、电池图标指示灯一起闪烁

        // 判断电池电量第 0 格指示灯有没有点亮
        // if ((aip3368h_speed_panel_display_buff[3] >> 12) & 0x01) {
        if (0 == display_err_dir) {
            aip3368h_speed_panel_display_buff[3] &= ~(0x01 << 12);
            aip3368h_display_battery_8_symbol_light(0);
            aip3368h_display_battery_icon_light(0);
        } else {
            aip3368h_speed_panel_display_buff[3] |= (0x01 << 12);
            aip3368h_display_battery_8_symbol_light(1);
            aip3368h_display_battery_icon_light(1);
        }
    }

    // 时间冒号闪烁（样机的时间分隔符与错误提示共用一个时基）
    // if ((aip3368h_engine_speed_panel_display_buff[1] >> 6) & 0x01) {
    if (0 == display_err_dir) {
        aip3368h_display_time_colon_light(0);
    } else {
        aip3368h_display_time_colon_light(1);
    }

    display_err_dir = !display_err_dir;
}

// 显示处理
void ui_display_handle(void)
{
    switch (ui_manager.state) {
    case UI_STATE_NORMAL:
        // 正常显示
        if (aip1302_update_time_interval >= AIP1302_UPDATE_TIME_INTERVAL) {
            aip1302_update_time_interval = 0;
            aip1302_read_all(); // 读取时间
            aip3368h_display_time(aip1302_info.time_hour,
                                  aip1302_info.time_min);
        }

        break;

    case UI_STATE_SETTING_DISTANCE_UNIT_TYPE:
        // 设置 要显示的单位类型 km/h 或 mph

        if (ui_manager.blink_timer_cnt >= UI_SETTING_BLINK_PERIOD) {
            ui_manager.blink_timer_cnt = 0;

            if (DISTANCE_UNIT_TYPE_METRIC ==
                instrument.save_info.distance_unit_type) {
                // 如果当前 设置的项目 是公制单位

                // 直接读取显存，判断有没有点亮对应的指示灯
                if ((aip3368h_speed_panel_display_buff[0] >> 3) & 0x01) {
                    // 如果是点亮的，改为熄灭
                    aip3368h_display_km_light(0);
                    aip3368h_display_kmh_light(0);
                } else {
                    // 如果当前是熄灭的，点亮它
                    aip3368h_display_km_light(1);
                    aip3368h_display_kmh_light(1);
                }
            } else if (DISTANCE_UNIT_TYPE_IMPERIAL ==
                       instrument.save_info.distance_unit_type) {
                // 直接读取显存，判断有没有点亮对应的指示灯
                if ((aip3368h_speed_panel_display_buff[0] >> 2) & 0x01) {
                    // 如果是点亮的，改为熄灭
                    aip3368h_display_miles_light(0);
                    aip3368h_display_mph_light(0);
                } else {
                    // 如果当前是熄灭的，点亮它
                    aip3368h_display_miles_light(1);
                    aip3368h_display_mph_light(1);
                }
            }
        }

        break;

    case UI_STATE_SETTING_TIME_MINUTE:
        // 设置 时间 分钟

        if (ui_manager.blink_timer_cnt < UI_SETTING_BLINK_PERIOD) {
            break;
        }

        ui_manager.blink_timer_cnt = 0;

        if (0 == ui_manager.blink_dir) {
            aip3368h_display_time_digits_when_setting(1, aip1302_info.time_min,
                                                      1);
        } else {
            aip3368h_display_time_digits_when_setting(0, aip1302_info.time_min,
                                                      1);
        }

        ui_manager.blink_dir = !ui_manager.blink_dir;

        break;

    case UI_STATE_SETTING_TIME_HOUR:

        if (ui_manager.blink_timer_cnt < UI_SETTING_BLINK_PERIOD) {
            break;
        }

        ui_manager.blink_timer_cnt = 0;
        if (0 == ui_manager.blink_dir) {
            aip3368h_display_time_digits_when_setting(1, aip1302_info.time_hour,
                                                      0);
        } else {
            aip3368h_display_time_digits_when_setting(0, aip1302_info.time_hour,
                                                      0);
        }

        ui_manager.blink_dir = !ui_manager.blink_dir;
        break;
    }

    // 设置超时
    if (ui_manager.auto_exit_setting_time_cnt >= UI_SETTING_TIME_OUT_CNT) {
        ui_manager.auto_exit_setting_time_cnt = 0;

        if (UI_STATE_SETTING_TIME_MINUTE == ui_manager.state ||
            UI_STATE_SETTING_TIME_HOUR == ui_manager.state) {
            // 如果是设置时间，超时后，将时间写回时钟IC
            aip1302_update_time(aip1302_info);
        }

        ui_set_state(UI_STATE_NORMAL);
        ui_display_refresh();

        // 自动退出设置界面后，保存相关数据
        instrument_info_save_enable();
    }

#if SPEED_SCAN_ENABLE
    aip3368h_display_speed_handle(); // 显示时速
#endif
    aip3368h_display_mileage_handle();      // 显示里程
    aip3368h_display_engine_speed_handle(); // 显示发动机转速

    ui_display_err_handle(); // 显示错误提示（例如低油量提示）
    aip3368h_module_display();
}

/**
 * @brief
 * 		清空所有计时
 * 		立即显示内容
 *
 * @note 只在切换了状态，需要立即刷新显示时使用，不能在每次显示时调用
 *
 */
void ui_display_refresh(void)
{
    ui_manager.blink_dir = 0;
    ui_manager.blink_timer_cnt = 0;
    ui_manager.auto_exit_setting_time_cnt = 0;

    switch (ui_manager.state) {
    case UI_STATE_NORMAL:
        // 正常显示

#if SPEED_SCAN_ENABLE
        // 显示时速
        aip3368h_display_speed_by_unit_type(instrument.speed_of_lag);
#endif

        break;
    case UI_STATE_SETTING_DISTANCE_UNIT_TYPE:
        // 设置 要显示的单位类型 km/h 或 mph

        break;
    case UI_STATE_SETTING_TIME_MINUTE:
        // 设置 车轮的周长

        break;
    }

    // 立即显示 时速 单位类型
    if (DISTANCE_UNIT_TYPE_METRIC == instrument.save_info.distance_unit_type) {
        // 公制单位
        aip3368h_display_mph_light(0);
        aip3368h_display_kmh_light(1);
    } else {
        // 英制单位
        aip3368h_display_kmh_light(0);
        aip3368h_display_mph_light(1);
    }

    // 立即更新里程显示：
    aip3368h_display_mileage_refresh();
    aip3368h_display_mileage_unit_lights(
        instrument.save_info.distance_unit_type);

    // 立即更新时间显示：
    aip3368h_display_time(aip1302_info.time_hour, aip1302_info.time_min);
}
