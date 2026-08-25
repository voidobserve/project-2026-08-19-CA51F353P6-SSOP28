#ifndef __UART_PROCESS_DATA_PACKETS_H__
#define __UART_PROCESS_DATA_PACKETS_H__

#define FORMAT_HEAD_FLAG 0xA5

#if 0
// 定义处理指令时使用到的各个状态：
enum
{
    CUR_RECV_CMD_STATUS_NONE = 0x00,
    CUR_RECV_CMD_STATUS_FORMATHEAD, // 格式头
    CUR_RECV_CMD_STATUS_LENGTH,     // 指令长度
    // CUR_RECV_CMD_STATUS_DATA,
    // CUR_RECV_CMD_STATUS_CHECKSUM, // 校验和

    CUR_RECV_CMD_STATUS_END,
};

// 定义单片机发送的带有数据的指令
enum
{
    SEND_GEAR = 0x01,         // 发送 挡位的状态
    SEND_BATTERY = 0x02,      // 发送 电池电量的状态
    SEND_BRAKE = 0x03,        // 发送 刹车的状态
    SEND_LEFT_TURN = 0x04,    // 发送 左转向灯的状态
    SEND_RIGHT_TURN = 0x05,   // 发送 右转向灯的状态
    SEND_HIGH_BEAM = 0x06,    // 发送 远光灯的状态
    SEND_ENGINE_SPEED = 0x07, // 发送 发动机的转速
    SEND_SPEED = 0x08,        // 发送 时速
    SEND_FUEL = 0x09,         // 发送 油量
    SEND_WATER_TEMP = 0x0A,   // 发送 水温

    /*
        发送大计里程， 数据 4 byte ， 数据直接用作屏幕显示，
        例如 00 04 17 65 ，显示 04176.5 km
    */
    SEND_TOTAL_MILEAGE_TENTH_OF_KM = 0x0B,
    /*
        发送大计里程， 数据 4 byte ， 数据直接用作屏幕显示，
        例如 00 04 17 65 ，显示 04176.5 mile
    */
    SEND_TOTAL_MILEAGE_TENTH_OF_MILE = 0x0C,

    SEND_TOUCH_KEY_STATUS = 0x0D, // 发送触摸按键的状态
    SEND_TIME = 0x0E,             // 发送时间（年月日，时分秒）
    SEND_HMS = 0x0F,              //发送时间(时分秒)

    SEND_VOLTAGE_OF_BATTERY = 0x10,  // 发送电池电压
    SEND_TEMP_OF_WATER_ALERT = 0x11, // 发送水温报警

    SEND_MALFUNCTION_STATUS = 0x12, // 发送故障的状态
    SEND_ABS_STATUS = 0x13,         // 发送ABS的状态

    /*
        发送小计里程， 数据 4 byte ， 数据直接用作屏幕显示，
        例如 00 04 17 65 ，显示 04176.5 km
    */
    SEND_SUBTOTAL_MILEAGE_TENTH_OF_KM = 0x14,
    /*
        发送小计里程， 数据 4 byte ， 数据直接用作屏幕显示，
        例如 00 04 17 65 ，显示 04176.5 mile
    */
    SEND_SUBTOTAL_MILEAGE_TENTH_OF_MILE = 0x15,

    /*
        发送 小计里程， 数据 2 byte ， 直接用作屏幕显示，
        例如 0d 1765 ，显示 1765 km
    */
    SEND_SUBTOTAL_MILEAGE_KM = 0x18,

    /*
        发送 小计里程， 数据 2 byte ， 直接用作屏幕显示，
        例如 0d 1765 ，显示 1765 mile
    */
    SEND_SUBTOTAL_MILEAGE_MILE = 0x19,

    SEND_SPEED_WITH_MILE = 0x1A, // 发送 时速（单位：英里每小时 mile/h）
};

// 定义存放 仪表数据 的结构体类型
typedef struct
{
    u32 total_mileage_with_km;   // 大计里程
    u32 total_mileage_with_mile; // 大计里程

    u32 subtotal_mileage_with_km;   // 小计里程
    u32 subtotal_mileage_with_mile; // 小计里程

    u16 battery_voltage; // 电池电压
    u16 engine_speed;    // 发动机转速
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;

    u8 gear_status;       // 档位状态
    u8 battery_percent;   // 电池电量百分比
    u8 brake_status;      // 刹车状态
    u8 left_turn_status;  // 左转状态
    u8 right_turn_status; // 右转状态
    u8 high_beam_status;  // 远光灯状态

    u8 speed_with_km;   // 时速，单位：km
    u8 speed_with_mile; // 时速，单位：mile

    u8 oil_percent; // 油量百分比
    // u8 temp_of_water; // 水温

} instrument_info_t;
extern instrument_info_t xdata instrument_info;

void uart_process_data_packets(void);
#endif

#endif