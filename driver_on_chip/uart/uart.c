#include "uart.h"
#include "user_config.h"

#include "ca51f_config.h"
#include "includes\ca51f3sfr.h"
#include "includes\ca51f3xsfr.h"
#include "includes\gpiodef_f3.h"

#include "includes\system.h"

// #include <stdarg.h>

#if USER_DEBUG_ENABLE
#include <stdio.h>
#endif

volatile uart_driver_tx_info_t uart1_drv_tx_info;

volatile bit uart1_tx_busy_flag; // 发送忙碌标志
volatile uart1_rx_buffer_t uart1_rx_buffer;

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
    memset(&uart1_drv_tx_info, 0, sizeof(uart_driver_tx_info_t));
    memset(&uart1_rx_buffer, 0, sizeof(uart1_rx_buffer_t));

    GPIO_Init(P11F, P11_UART1_TX_SETTING);
    GPIO_Init(P12F, P12_UART1_RX_SETTING);

    // 波特率
    value_temp = 0x400 - FOSC / (UART1_BAUTRATE * 32);
    S1RELH = (unsigned char)(value_temp >> 8);
    S1RELL = (unsigned char)(value_temp);

    // S1CON = 0xD0;
    S1CON = ((0x01 << 7) | // 8位异步模式
             (0x01 << 6) | // 使能中断
             (0x01 << 4)); // 串行接收使能
    ES1 = 1;               // UART1中断使能
}

// 发送一个字节数据
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
    unsigned char free_space;
    unsigned char tail_tmp;

    while (1) {
        tail_tmp = uart1_drv_tx_info.tail;
        if (uart1_drv_tx_info.head < tail_tmp) {
            // 头指针在尾指针前面
            free_space = tail_tmp - uart1_drv_tx_info.head;
        } else {
            // 头指针在尾指针后面
            free_space = UART1_TX_BUF_SIZE + tail_tmp - uart1_drv_tx_info.head;
        }

        // 如果缓冲区有空间，则存放数据。如果缓冲区已满，则等待
        if (free_space > 1) {
            ES1 = 0; // 不使能UART1中断

            // 先偏移指针，再存放数据
            uart1_drv_tx_info.head++;
            uart1_drv_tx_info.head %= UART1_TX_BUF_SIZE;
            uart1_drv_tx_info.buff[uart1_drv_tx_info.head] = byte;

            if (!uart1_tx_busy_flag) {
                // 如果上一次发送已经完成，则立即发送
                ES1 = 1;
                uart1_drv_tx_info.tail++;
                uart1_drv_tx_info.tail %= UART1_TX_BUF_SIZE;
                S1BUF = uart1_drv_tx_info.buff[uart1_drv_tx_info.tail];
                uart1_tx_busy_flag = 1;
            } else {
                ES1 = 1;
            }

            break;
        }
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
    uart1_rx_buffer.tail = (uart1_rx_buffer.tail + 1) % UART1_RX_BUF_SIZE;
    rxbyte = uart1_rx_buffer.buffer[uart1_rx_buffer.tail];

    uart1_rx_buffer.count--;

    return rxbyte;
}

void uart1_rxbuffer_put(u8 byte)
{
    // 目前的逻辑：缓冲区满，覆盖旧的数据

    // 先偏移索引，再存放数据
    uart1_rx_buffer.head = (uart1_rx_buffer.head + 1) % UART1_RX_BUF_SIZE;
    uart1_rx_buffer.buffer[uart1_rx_buffer.head] = byte;

    uart1_rx_buffer.count++;

    if (uart1_rx_buffer.count > UART1_RX_BUF_SIZE) {
        uart1_rx_buffer.count = UART1_RX_BUF_SIZE;
    }
}

void UART1_ISR(void) interrupt 6
{
    if (S1CON & BIT0) {
        // 接收中断
        // S1CON = (S1CON & 0xFC) | 0x01; // REVIEW 官方的示例
        S1CON |= 0x01; // 写1清空接收中断标志
        uart1_rxbuffer_put(S1BUF);
    }

    if (S1CON & BIT1) {
        S1CON = (S1CON & ~(BIT0 | BIT1)) | BIT1;
        if (uart1_drv_tx_info.head != uart1_drv_tx_info.tail) {
            uart1_drv_tx_info.tail++;
            uart1_drv_tx_info.tail %= UART1_TX_BUF_SIZE;
            S1BUF = uart1_drv_tx_info.buff[uart1_drv_tx_info.tail];
        } else {
            uart1_tx_busy_flag = 0;
        }
    }
}
