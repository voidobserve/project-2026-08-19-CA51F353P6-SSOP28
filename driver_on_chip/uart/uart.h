#ifndef __UART_H__
#define __UART_H__

#include "user_typedef.h"

#define UART1_ENABLE 1

#define UART1_TX_BUF_SIZE 100
#define UART1_RX_BUF_SIZE 100

#define UART1_BAUTRATE 115200

// 定义串口发送的管理结构体
typedef struct
{
    u8 buffer[UART1_TX_BUF_SIZE];
    u8 head;
    u8 tail;
    u8 count;
} uart1_tx_buffer_t;

extern volatile uart1_tx_buffer_t uart1_tx_buffer;

// 环形缓冲区结构体定义
typedef struct
{
    u8 buffer[UART1_RX_BUF_SIZE];
    u8 head;
    u8 tail;
    u8 count;
} uart1_rx_buffer_t;

extern volatile uart1_rx_buffer_t uart1_rx_buffer;

void uart1_init(void);

extern void uart1_send_byte(unsigned char byte);

u8 uart1_rxbuffer_get_count(void);
u8 uart1_rxbuffer_get_byte(void);

void uart1_txbuffer_put_byte(u8 byte);
u8 uart1_txbuffer_get_byte(void);
void uart1_txbuffer_put_buff(u8 *buff, u8 len);
void uart1_txbuffer_handle(void);

#endif
