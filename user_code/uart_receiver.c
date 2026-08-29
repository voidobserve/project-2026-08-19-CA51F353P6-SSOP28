#include "user_config.h"
#include "uart.h"
#include "uart_receiver.h"

#include "instrument.h"
#include "aip3368h_display.h"
#include "engine_speed_process.h" // variable: rpm_from_collector

#if USER_DEBUG_ENABLE
#include <stdio.h>
#endif

// 串口数据接收器
static volatile uart_receiver_t uart_receiver = {0};

void uart_receiver_reset(void)
{
    uart_receiver.cur_recv_len = 0;
    uart_receiver.idx = 0;
    uart_receiver.timeout_enable = 0;
    uart_receiver.timeout_cnt = 0;
    uart_receiver.sta = UART_RECEIVER_STA_IDLE;
}

/**
 * @brief 接收器处理超时使能之后，会累计超时时间
 *      该函数在定时器中断内调用
 * 
 */
void uart_receiver_process_timeout_add(void)
{
    if (uart_receiver.timeout_enable) {
        if (uart_receiver.timeout_cnt < ((u16)-1)) {
            uart_receiver.timeout_cnt++;
        }
    }
}

/**
 * @brief 接收器处理超时，调用该处理函数
 * 
 */
void uart_receiver_process_timeout_handle(void)
{
#if USER_DEBUG_ENABLE
    u8 i;
#endif

    if (uart_receiver.timeout_cnt < UART_RECEIVER_PROCESS_TIMEOUT) {
        return;
    }

#if USER_DEBUG_ENABLE
    printf("UART receive process timeout, current status: %u\n",
           (u16)uart_receiver.sta);

    // 打印当前缓冲区内容
    printf("Buffer content: \n");
    for (i = 0; i < uart_receiver.idx; i++) {
        printf("0x%02x ", (u16)uart_receiver.buffer[i]);
    }
    printf("\n");
#endif

    // 重置接收器
    uart_receiver_reset();
}

/**
 * @brief 串口接收器处理当前接收到的1个字节数据
 * 
 * @param byte 
 */
u8 uart_receiver_process_byte(u8 byte)
{
    u8 ret = 1; // 0: 处理成功，1: 处理失败
    u8 i;       // 循环计数值
    u8 check_sum = 0;

    // 表示当前接收了一个字节，并不表示接收成功
    uart_receiver.cur_recv_len++;

    switch (uart_receiver.sta) {
    case UART_RECEIVER_STA_IDLE:
        // 空闲状态，判断有没有格式头到来
        if (UART_INSTRUCT_HEAD == byte) {
            uart_receiver.sta++;
            ret = 0;
        }
        break;
    case UART_RECEIVER_STA_FORMAT_HEAD:
        // 接收了格式头，接下来接收数据长度
        uart_receiver.sta++;
        uart_receiver.expect_recv_len = byte;
        ret = 0;
        break;
    case UART_RECEIVER_STA_LENGTH:
        // 接收了数据长度，现在接收指令码
        uart_receiver.sta++;
        ret = 0;
        break;
    case UART_RECEIVER_STA_INSTRUCT:
        // 接收了指令码，当前在接收有效数据字节：
        ret = 0;

        if (uart_receiver.cur_recv_len >= uart_receiver.expect_recv_len) {
            // 数据接收完成，计算校验和

            for (i = 0; i < uart_receiver.expect_recv_len - 1; i++) {
                check_sum += uart_receiver.buffer[i];
            }

            if (check_sum != byte) {
                // 校验和错误
                ret = 1;
            } else {
                // 校验和正确，接收成功
                uart_receiver.sta++;
            }
        }
        break;
    default:
#if USER_DEBUG_ENABLE
        printf("uart_receiver_process_byte\n");
        printf("uart_receiver.sta invalid, val == %u\n",
               (u16)uart_receiver.sta);
#endif
        break;
    }

    if (0 == ret) {
        // 处理成功，将当前接收到的字节数据存入缓冲区
        uart_receiver.buffer[uart_receiver.idx] = byte;
        uart_receiver.idx++;
    }

    return ret;
}

void uart_receiver_process(void)
{
    u8 recv_byte;
    u8 ret;

    // 检查是否接收超时
    if (uart_receiver.timeout_enable) {
        uart_receiver_process_timeout_handle();
    }

    if (0 == uart1_rxbuffer_get_count()) {
        // 串口接收缓冲区为空，不处理
        return;
    }

    recv_byte = uart1_rxbuffer_get_byte();
    // 接收一次数据后，启用超时处理，重置超时计数
    uart_receiver.timeout_enable = 1;
    uart_receiver.timeout_cnt = 0;

    ret = uart_receiver_process_byte(recv_byte);
    if (ret) {
// 处理失败，重置接收器
#if USER_DEBUG_ENABLE
#if 1
        // 如果连格式头都没有接收，不打印错误信息
        if (uart_receiver.sta != 0) {
            printf("Byte processing failed\n");
            printf("cur uart receiver status: %02u\n", (u16)uart_receiver.sta);
        }
#endif
#endif

        uart_receiver_reset();
    }

    if (uart_receiver.sta != UART_RECEIVER_STA_DATA) {
        // 没有接收完，函数直接返回，等待下次数据到来
        return;
    }

    // =======================================================
    // =======================================================
    // 接收完成，处理接收到的数据

    // 根据指令码，处理对应的数据
    switch (uart_receiver.buffer[2]) {
    case UART_INSTRUCT_GEAR:

        instrument.gear = uart_receiver.buffer[3];

#if USER_DEBUG_ENABLE
        // printf("recv gear\n");
        // aip3368h_display_gear(instrument.gear);
#endif
        break;
    case UART_INSTRUCT_BAT_VOLTAGE:

        instrument.bat_voltage =
            ((u16)uart_receiver.buffer[3] << 8) | (u16)uart_receiver.buffer[4];

#if USER_DEBUG_ENABLE
        // printf("recv bat voltage\n");
        // printf("bat voltage == %u mV\n", (u16)instrument.bat_voltage);
#endif
        break;

    case UART_INSTRUCT_TIME:
        instrument.time_hour = uart_receiver.buffer[3];
        instrument.time_minute = uart_receiver.buffer[4];

#if USER_DEBUG_ENABLE
        // printf("recv time\n");
        // printf("hour == %u\n", (u16)instrument.time_hour);
        // printf("minute == %u\n", (u16)instrument.time_minute);
#endif
        break;

    case UART_INSTRUCT_ENGINE_SPEED:
        // instrument.engine_speed =
        //     ((u16)uart_receiver.buffer[3] << 8) | (u16)uart_receiver.buffer[4];
        rpm_from_collector =
            ((u16)uart_receiver.buffer[3] << 8) | (u16)uart_receiver.buffer[4];
            // 后续交给 engine_speed_process 处理
        break;

    case UART_INSTRUCT_SPEED:
        instrument.speed =
            ((u16)uart_receiver.buffer[3] << 8) | (u16)uart_receiver.buffer[4];
        break;
    case UART_INSTRUCT_TOTAL_MILEAGE:
        instrument.total_mileage = (u32)uart_receiver.buffer[3] << 24 |
                                   (u32)uart_receiver.buffer[4] << 16 |
                                   (u32)uart_receiver.buffer[5] << 8 |
                                   (u32)uart_receiver.buffer[6]; 
        break;
    case UART_INSTRUCT_SUBTOTAL_MILEAGE:
        instrument.subtotal_mileage = (u32)uart_receiver.buffer[3] << 24 |
                                      (u32)uart_receiver.buffer[4] << 16 |
                                      (u32)uart_receiver.buffer[5] << 8 |
                                      (u32)uart_receiver.buffer[6]; 
        break;
    case UART_INSTRUCT_LEFT_TURN:
        instrument.left_turn_valid = uart_receiver.buffer[3];
        break;
    case UART_INSTRUCT_RIGHT_TURN:
        instrument.right_turn_valid = uart_receiver.buffer[3];
        break;
    // case UART_INSTRUCT_ENGINE_ERR:
    //     instrument.engine_err_valid = uart_receiver.buffer[3];
    //     break;
    // case UART_INSTRUCT_ABS:
    //     instrument.abs_valid = uart_receiver.buffer[3];
    //     break;
    // case UART_INSTRUCT_ENGINE_OIL:
    //     instrument.engine_oil_valid = uart_receiver.buffer[3];
    //     break;
    case UART_INSTRUCT_TEMP_OF_WATER_ERR:
        instrument.temp_of_water_err_valid = uart_receiver.buffer[3];
        break;
    case UART_INSTRUCT_HIGH_BEAM:
        instrument.high_beam_valid = uart_receiver.buffer[3];
        break;
    case UART_INSTRUCT_FUEL_PERCENT:
        instrument.fuel_percent = uart_receiver.buffer[3];
        break;
    case UART_INSTRUCT_KEY:
        instrument.key_event = uart_receiver.buffer[3];
        // REVIEW 处理完按键事件后，注意要清零
        break;

    default:
        break;
    }

    // 处理完一帧数据后，重置接收器，准备下一帧接收
    uart_receiver_reset();
}
