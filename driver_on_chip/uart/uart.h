#ifndef __UART_H__
#define __UART_H__

#include "user_typedef.h"

#define UART1_TX_BUF_SIZE 20
#define UART1_RX_BUF_SIZE 100

#define UART1_BAUTRATE 115200

#define UART1_ENABLE 1

// ===========================================================================
// 驱动层定义
// ===========================================================================

typedef struct
{
    unsigned char head;
    unsigned char tail;
    unsigned char buff[UART1_TX_BUF_SIZE];
} uart_driver_tx_info_t;

// ===========================================================================
//
// ===========================================================================

extern volatile uart_driver_tx_info_t uart1_drv_tx_info;

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

#endif
