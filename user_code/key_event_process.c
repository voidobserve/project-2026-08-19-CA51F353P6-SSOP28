#include "key_event_process.h"
#include "instrument.h"
#include "ui.h"
#include "aip3368h_display.h"
#include "uart.h"
#include "uart_transmitter.h"

void key_event_process(void)
{ 
    if (instrument.key_event == KEY_EVENT_NONE) {
        return;
    }

    switch (instrument.key_event) {
    case 0x00:
        // 按键短按
        if (UI_STATE_NORMAL == ui_manager.state) {
            // 正常显示模式下，按键单击切换显示的里程类型
            instrument.is_display_total_mileage =
                !instrument.is_display_total_mileage;
            instrument_info_save_enable();
        } else if (UI_STATE_SETTING_DISTANCE_UNIT_TYPE == ui_manager.state) {
            // 切换 单位类型
            if (DISTANCE_UNIT_TYPE_METRIC == instrument.distance_unit_type) {
                instrument.distance_unit_type = DISTANCE_UNIT_TYPE_IMPERIAL;
            } else {
                instrument.distance_unit_type = DISTANCE_UNIT_TYPE_METRIC;
            }
        } else if (UI_STATE_SETTING_TIME_MINUTE == ui_manager.state) {
            // 在设置分钟时，检测到短按
            instrument.time_minute++;
            if (instrument.time_minute > 59) {
                instrument.time_minute = 0;
            }
        } else if (UI_STATE_SETTING_TIME_HOUR == ui_manager.state) {
            // 在设置小时时，检测到短按
            instrument.time_hour++;
            if (instrument.time_hour > 23) {
                instrument.time_hour = 0;
            }
        }

        ui_display_refresh();
        break;
    case 0x01:
        // 按键长按
        if (UI_STATE_NORMAL == ui_manager.state) {
            // 如果正在显示总里程
            if (1 == instrument.is_display_total_mileage) {
                // 如果显示的是 TOTAL 里程，切换到设置要显示的单位类型
                ui_set_state(UI_STATE_SETTING_DISTANCE_UNIT_TYPE);
            } else {
                // 如果显示的是 TRIP 里程，清空它
                instrument.subtotal_mileage = 0;
                instrument_info_save_enable();
                aip3368h_display_mileage_refresh();
                aip3368h_display_mileage_unit_lights(
                    instrument.distance_unit_type);
            }
        } else if (UI_STATE_SETTING_DISTANCE_UNIT_TYPE == ui_manager.state) {
            ui_set_state(UI_STATE_SETTING_TIME_MINUTE);
        } else if (UI_STATE_SETTING_TIME_MINUTE == ui_manager.state) {
                // 重复发送设置时间的指令
                uart_transmitter_send_instruct(UART_INSTRUCT_TIME);
            ui_set_state(UI_STATE_SETTING_TIME_HOUR);
        } else if (UI_STATE_SETTING_TIME_HOUR == ui_manager.state) { 
               // 重复发送设置时间的指令
                uart_transmitter_send_instruct(UART_INSTRUCT_TIME); 
            ui_set_state(UI_STATE_NORMAL);
        }

        ui_display_refresh();
        break;
    default:
        break;
    }

    instrument.key_event = KEY_EVENT_NONE;
}