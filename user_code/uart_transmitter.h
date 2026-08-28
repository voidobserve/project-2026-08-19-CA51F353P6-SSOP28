#ifndef __UART_TRANSMITTER_H__
#define __UART_TRANSMITTER_H__

#include "user_typedef.h"
#include "uart_receiver.h" // 包含串口指令的定义

void uart_transmitter_send_instruct(uart_instruct_t uart_instruct);

#endif


