#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include "user_typedef.h"

// 延时写入flash的时间：
#define INSTRUMENT_INFO_SAVE_TIME ((u16)2000)

// 挡位的定义
enum
{
    GEAR_NEUTRAL = 0x00, // 空挡

    GEAR_FIRST = 0x01,  // 一档
    GEAR_SECOND = 0x02, // 二档
    GEAR_THIRD = 0x03,  // 三档
    GEAR_FOURTH = 0x04, // 四档
    GEAR_FIFTH = 0x05,  // 五档
    GEAR_SIXTH = 0x06,  // 六档
    /*
        未知，如果 GEAR_NEUTRAL ~ GEAR_SIXTH 都没有检测到，
        则返回 GEAR_UNKNOWN ， 让显示屏中档位对应的图标空着
    */
    GEAR_UNKNOWN = 0xFF,
};
typedef u8 gear_t;

// 单位类型：公制单位 或 英制单位
enum
{
    DISTANCE_UNIT_TYPE_METRIC,   // 公制单位，时速用 km/h ，里程用 km
    DISTANCE_UNIT_TYPE_IMPERIAL, // 英制单位，时速用 mph ，里程用 mile
};
typedef u8 distance_unit_type_t; // 距离相关的单位类型

// 定义存储在flash中的数据
typedef struct
{
    u32 write_cnt;               // 写入flash的次数
    u8 check_sum;                // 校验和
    u8 is_display_total_mileage; // 0：显示总里程，1：显示短距离里程
    // 要显示的 单位类型，km/h 或 mph。在时速和里程中用到
    distance_unit_type_t distance_unit_type;
} save_info_t;

typedef struct
{
    // ==============================================================
    // ==============================================================
    // 需要掉电保存的数据：
    u8 is_display_total_mileage; // 0：显示短距离里程，1：显示总里程
    // 要显示的 单位类型，km/h 或 mph。在时速和里程中用到
    distance_unit_type_t distance_unit_type;

    // ==============================================================
    // ==============================================================

    // 总里程表（单位：km，使用英制单位时，需要进行转换）
    // （大计里程，范围：0 ~ 999 99 KM）
    u32 total_mileage;
    // 短距离里程表(单位：x100 m，使用英制单位时，需要进行转换)
    // （小计里程，范围：0 ~ 999 9.9 KM）
    u32 subtotal_mileage;

    u32 engine_speed; // 发动机的转速（单位：rpm）

    u16 speed; // 时速(单位：km/h，使用英制单位时，需要进行转换)
    // 最终要显示的时速
    u16 speed_to_display;

    u8 fuel_percent;            // 油量(单位：百分比)
    gear_t gear;                // 档位
    u16 bat_voltage;            // 电池电压（单位：mV）
    u8 time_hour;               // 小时
    u8 time_minute;             // 分钟
    u8 left_turn_valid;         // 左转灯是否有效
    u8 right_turn_valid;        // 右转灯是否有效
    u8 temp_of_water_err_valid; // 水温异常提示是否有效
    u8 high_beam_valid;         // 远光灯是否有效
    // u8 abs_valid; // ABS是否有效 (当前工程由芯片控制，不用从检测端接收)
    // u8 engine_err_valid; // 发动机故障是否有效 (当前工程由芯片控制，不用从检测端接收)
    // u8 engine_oil_valid; // 机油压力提示是否有效 (当前工程由芯片控制，不用从检测端接收)

    u8 key_event; // 按键事件

    // 标志位，是否处于低油量提示
    u8 flag_is_in_warning_of_low_fuel;
    // 标志位，是否处于低电量提示
    u8 flag_is_in_warning_of_low_battery;
} instrument_t;
extern volatile instrument_t instrument;

void instrument_info_init(void);

void instrument_info_save_time_add(void);
void instrument_info_save_enable(void);
void instrument_info_save_handle(void);

#endif
