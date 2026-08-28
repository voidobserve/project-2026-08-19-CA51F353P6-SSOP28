#include "uart_transmitter.h"
#include "uart.h"
#include "uart_receiver.h" // 包含串口指令的定义
#include "instrument.h"

// #include "user_typedef.h"

volatile u8 uart_transmitter_buffer[10] = {0};

void uart_transmitter_send_instruct(uart_instruct_t uart_instruct)
{
    u8 check_sum = 0; // 存放校验和

    uart_transmitter_buffer[0] = UART_INSTRUCT_HEAD;

    switch (uart_instruct) {
    case UART_INSTRUCT_TIME:
        uart_transmitter_buffer[1] = 0x06;               // 帧长度
        uart_transmitter_buffer[2] = UART_INSTRUCT_TIME; // 指令
        uart_transmitter_buffer[3] = instrument.time_hour;
        uart_transmitter_buffer[4] = instrument.time_minute;

        check_sum = uart_transmitter_buffer[0] + uart_transmitter_buffer[1] +
                    uart_transmitter_buffer[2] + uart_transmitter_buffer[3] +
                    uart_transmitter_buffer[4];
        uart_transmitter_buffer[5] = check_sum;
        break;

    default:
        return;
        break;
    }

    uart1_txbuffer_put_buff(uart_transmitter_buffer,
                            ARRAY_SIZE(uart_transmitter_buffer));
}
 