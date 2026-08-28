#ifndef __UART_RECEIVER_H__
#define __UART_RECEIVER_H__

#include "user_typedef.h"

// 接收数据的超时时间，单位：ms
#define UART_RECEIVER_PROCESS_TIMEOUT ((u16)2 * 1000)
#define UART_INSTRUCT_HEAD                   0xA5 // 格式头
#define UART_MAX_FRAME_LENGTH         20   // 最大帧长度

// 定义串口接收器使用到的各个状态
enum
{
    UART_RECEIVER_STA_IDLE = 0x00, // 空闲状态

    // 下面的状态表示已经接收完对应的数据
    UART_RECEIVER_STA_FORMAT_HEAD, // 接收格式头
    UART_RECEIVER_STA_LENGTH,      // 接收帧长度
    UART_RECEIVER_STA_INSTRUCT,    // 接收指令
    UART_RECEIVER_STA_DATA,        // 接收数据字节
    // UART_RECEIVER_STA_CHECKSUM,    // 接收校验和
};

// 各个串口指令
enum
{
    UART_INSTRUCT_GEAR = 0x01,       // 档位
    UART_INSTRUCT_BAT_PERCENT,       // 电池电量百分比 (当前工程未使用)
    UART_INSTRUCT_BAT_VOLTAGE,       // 电池电压
    UART_INSTRUCT_TIME,              // 时间
    UART_INSTRUCT_ENGINE_SPEED,      // 发动机转速
    UART_INSTRUCT_SPEED,             // 时速
    UART_INSTRUCT_TOTAL_MILEAGE,     // 总里程 ODO
    UART_INSTRUCT_SUBTOTAL_MILEAGE,  // 当前里程 TRIP
    UART_INSTRUCT_LEFT_TURN,         // 左转向
    UART_INSTRUCT_RIGHT_TURN,        // 右转向
    UART_INSTRUCT_ENGINE_ERR,        // 发动机故障
    UART_INSTRUCT_ABS,               // ABS故障
    UART_INSTRUCT_ENGINE_OIL,        // 机油压力提示
    UART_INSTRUCT_TEMP_OF_WATER_ERR, // 水温异常提示
    UART_INSTRUCT_BRAKE,             // 刹车 （当前工程未使用）
    UART_INSTRUCT_HIGH_BEAM,         // 远光
    UART_INSTRUCT_FUEL_PERCENT,      // 油量百分比
    UART_INSTRUCT_KEY,               // 按键
};
typedef u8 uart_instruct_t; // 串口指令类型

// 串口接收器结构体定义
typedef struct
{
    u8 buffer[UART_MAX_FRAME_LENGTH]; // 接收缓冲区
    u8 idx;                           // 当前接收位置

    u8 cur_recv_len;    // 已接收的数据长度
    u8 expect_recv_len; // 期望接收的数据长度

    u8 sta; // 接收器当前状态

    u8 timeout_enable; // 超时处理使能标志
    u16 timeout_cnt;   // 超时计数器
} uart_receiver_t;

void uart_receiver_reset(void);
void uart_receiver_process_timeout_add(void);
void uart_receiver_process(void);

#endif