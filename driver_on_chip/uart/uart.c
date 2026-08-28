#include "uart.h"
#include "user_config.h"

#include "ca51f_config.h"
#include "includes\ca51f3sfr.h"
#include "includes\ca51f3xsfr.h"
#include "includes\gpiodef_f3.h"

#include "includes\system.h"

#include <string.h>

#if USER_DEBUG_ENABLE
#include <stdio.h>
#endif

volatile bit uart1_tx_busy_flag; // 发送忙碌标志
volatile uart1_rx_buffer_t uart1_rx_buffer;

volatile uart1_tx_buffer_t uart1_tx_buffer;

#if USER_DEBUG_ENABLE
// 重写puchar()函数
char putchar(char c)
{
    uart1_send_byte(c);
    return c;
}
#endif

void uart1_init(void)
{
    unsigned int value_temp;

    uart1_tx_busy_flag = 0;
    memset(&uart1_tx_buffer, 0, sizeof(uart1_tx_buffer_t));
    memset(&uart1_rx_buffer, 0, sizeof(uart1_rx_buffer_t));

    GPIO_Init(P11F, P11_UART1_TX_SETTING);
    GPIO_Init(P12F, P12_UART1_RX_SETTING);

    // 波特率
    value_temp = 0x400 - FOSC / (UART1_BAUTRATE * 32);
    S1RELH = (unsigned char)(value_temp >> 8);
    S1RELL = (unsigned char)(value_temp);

    S1CON = ((0x01 << 7) | // 8位异步模式
             (0x01 << 6) | // 使能中断
             (0x01 << 4)); // 串行接收使能
    ES1 = 1;               // UART1中断使能
}

/**
 * @brief 发送一个字节数据
 * 
 * 往发送缓冲区中存放数据，通过串口中断发送
 * 
 * @note 会阻塞主循环，直到发送完成
 *      如果缓冲区有空间，则存放数据。如果缓冲区已满，则等待
 * 
 */
void uart1_send_byte(unsigned char byte)
{
    while (1) {

        // 等待上一次发送完成
        if (!uart1_tx_busy_flag) {

            ES1 = 0; // 不使能 UART1 中断
            S1BUF = byte;
            uart1_tx_busy_flag = 1;
            ES1 = 1;
            break;
        }

        // 在这里加喂狗，在频繁收发时，会把整个程序卡死
        // WDFLG = 0xA5; // 喂狗
    }
}

u8 uart1_rxbuffer_get_count(void)
{
    return uart1_rx_buffer.count;
}

/**
 * @brief 从接收缓冲区中取出一个字节数据
 *     
 * @note 使用该函数前，需要先判断接收缓冲区中是否有数据
 * 
 */
u8 uart1_rxbuffer_get_byte(void)
{
    u8 rxbyte;

    if (0 == uart1_rx_buffer.count) {
        // 缓冲区空
        return 0;
    }

    // 先偏移索引，再取出数据
    uart1_rx_buffer.tail++;
    if (uart1_rx_buffer.tail >= UART1_RX_BUF_SIZE) {
        uart1_rx_buffer.tail = 0;
    }

    rxbyte = uart1_rx_buffer.buffer[uart1_rx_buffer.tail];

    uart1_rx_buffer.count--;

    return rxbyte;
}

void uart1_rxbuffer_put(u8 byte)
{
    // 目前的逻辑：缓冲区满，覆盖旧的数据

    // 先偏移索引，再存放数据
    uart1_rx_buffer.head++;
    if (uart1_rx_buffer.head >= UART1_RX_BUF_SIZE) {
        uart1_rx_buffer.head = 0;
    }
    uart1_rx_buffer.buffer[uart1_rx_buffer.head] = byte;

    uart1_rx_buffer.count++;

    if (uart1_rx_buffer.count > UART1_RX_BUF_SIZE) {
        uart1_rx_buffer.count = UART1_RX_BUF_SIZE;
    }
}

void uart1_txbuffer_put_byte(u8 byte)
{
    // 目前的逻辑：缓冲区满，覆盖旧的数据

    // 先偏移索引，再存放数据
    uart1_tx_buffer.head++;
    if (uart1_tx_buffer.head >= UART1_TX_BUF_SIZE) {
        uart1_tx_buffer.head = 0;
    }
    uart1_tx_buffer.buffer[uart1_tx_buffer.head] = byte;

    uart1_tx_buffer.count++;

    if (uart1_tx_buffer.count > UART1_TX_BUF_SIZE) {
        uart1_tx_buffer.count = UART1_TX_BUF_SIZE;
    }
}

u8 uart1_txbuffer_get_byte(void)
{
    u8 tx_byte;

    if (0 == uart1_tx_buffer.count) {
        // 缓冲区空
        return 0;
    }

    // 先偏移索引，再取出数据
    uart1_tx_buffer.tail++;
    if (uart1_tx_buffer.tail >= UART1_TX_BUF_SIZE) {
        uart1_tx_buffer.tail = 0;
    }

    tx_byte = uart1_tx_buffer.buffer[uart1_tx_buffer.tail];

    uart1_tx_buffer.count--;

    return tx_byte;
}

/**
 * @brief 往串口发送缓冲区放入一串数据
 * 
 * @note 受制于芯片性能，不能频繁调用该函数，也不能一次发送过多数据
 * 
 * @param buff 
 * @param len 
 */
void uart1_txbuffer_put_buff(u8 *buff, u8 len)
{
    u8 i;
    for (i = 0; i < len; i++) {
        uart1_txbuffer_put_byte(buff[i]);
    }
}

/**
 * @brief 处理串口发送缓冲区的数据，尽量以非阻塞的形式
 * 
 */
void uart1_txbuffer_handle(void)
{
    u8 tx_byte;
    if (uart1_tx_buffer.count == 0 || uart1_tx_busy_flag) {
        // 发送缓冲区中没有数据，直接返回
        // 发送缓冲区有数据，但发送忙碌，直接返回
        return;
    }

    tx_byte = uart1_txbuffer_get_byte();
    uart1_send_byte(tx_byte);
}

void UART1_ISR(void) interrupt 6
{
    if (S1CON & BIT0) {
        // 接收中断
        // S1CON = (S1CON & 0xFC) | 0x01; // REVIEW 该行是官方demo的示例
        S1CON |= 0x01; // 写1清空接收中断标志
        uart1_rxbuffer_put(S1BUF);
    }

    if (S1CON & BIT1) {
        // 串口1 发送完 S1BUF 数据中的停止位后，触发发送中断，进入这里
        S1CON |= (0x01 << 1); // 写1清空发送中断标志
        uart1_tx_busy_flag = 0;
    }
}
