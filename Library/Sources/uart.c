#ifndef _UART_C_
#define _UART_C_
#include "ca51f_config.h"
#include "includes\ca51f3sfr.h"
#include "includes\ca51f3xsfr.h"
#include "includes\gpiodef_f3.h"

#include "includes\system.h"
#include "Library\includes\uart.h"
#include <stdarg.h>
#include <stdio.h>
#include "string.h"

#include "user_config.h"

/*********************************************************************************************************************/
/*********************************************************************************************************************/
#ifdef UART0_EN
uart0_rx_buffer_t xdata uart0_rx_buffer;
void Uart0_Initial(unsigned long int baudrate)
{
    unsigned int value_temp;

    GPIO_Init(P31F, P31_UART0_RX_SETTING);
    GPIO_Init(P30F, P30_UART0_TX_SETTING);

    uart0_send.head = 0;
    uart0_send.tail = 0;
    uart0_rev.head = 0;
    uart0_rev.tail = 0;
    uart0_tx_flag = 0;

    /************************************************************************************************************************/
    //TIMER2作为UART0的波特率发生器
    value_temp = 0x10000 - FOSC / (baudrate * 32);
    T2CON = 0x24;
    T2CH = (unsigned char)(value_temp >> 8);
    T2CL = (unsigned char)(value_temp);
    TH2 = (unsigned char)(value_temp >> 8);
    TL2 = (unsigned char)(value_temp);
    ;
    TR2 = 1;
    /************************************************************************************************************************/

    /************************************************************************************************************************/
    //TIMER1作为UART0的波特率发生器

    //  TMOD = (TMOD&0xCF)|0x20;
    //	TH1 = 0xff;		//19200
    //	TL1 = 0xff;
    //	ET1=0;
    //	TR1=1;
    //	PCON |= 0x80;
    /************************************************************************************************************************/
    S0CON = 0x50;
    ES0 = 1;
}
void Uart0_PutChar(unsigned char bdat)
{
    unsigned char free_space;
    unsigned char tail_tmp;

    while (1) {
        tail_tmp = uart0_send.tail;
        if (uart0_send.head < tail_tmp) {
            free_space = tail_tmp - uart0_send.head;
        } else {
            free_space = UART0_TX_BUF_SIZE + tail_tmp - uart0_send.head;
        }
        if (free_space > 1) {
            ES0 = 0;
            uart0_send.head++;
            uart0_send.head %= UART0_TX_BUF_SIZE;
            uart0_tx_buf[uart0_send.head] = bdat;
            if (!uart0_tx_flag) {
                ES0 = 1;
                uart0_send.tail++;
                uart0_send.tail %= UART0_TX_BUF_SIZE;
                S0BUF = uart0_tx_buf[uart0_send.tail];
                uart0_tx_flag = 1;
            } else {
                ES0 = 1;
            }
            break;
        }
    }
}
void UART0_ISR(void) interrupt 4
{
    if (RI0) {
        RI0 = 0;
        // uart0_rev.head++;
        // uart0_rev.head %= UART0_RX_BUF_SIZE;
        // uart0_rx_buf[uart0_rev.head]=S0BUF;

        uart0_rxbuffer_put(S0BUF);
    }
    if (TI0) {
        TI0 = 0;
        if (uart0_send.head != uart0_send.tail) {
            uart0_send.tail++;
            uart0_send.tail %= UART0_TX_BUF_SIZE;
            S0BUF = uart0_tx_buf[uart0_send.tail];
        } else {
            uart0_tx_flag = 0;
        }
    }
}

u8 uart0_rxbuffer_get_count(void)
{
    return uart0_rx_buffer.count;
}

u8 uart0_rxbuffer_get(void)
{
    u8 rxbyte;

    if (0 == uart0_rx_buffer.count) {
        // 缓冲区空
        return 0;
    }

    // 先偏移索引，再取出数据
    uart0_rx_buffer.tail = (uart0_rx_buffer.tail + 1) % UART0_RX_BUF_SIZE;
    rxbyte = uart0_rx_buffer.buffer[uart0_rx_buffer.tail];

    uart0_rx_buffer.count--;

    return rxbyte;
}

void uart0_rxbuffer_put(u8 byte)
{
    // 目前的逻辑：缓冲区满，覆盖旧的数据

    // 先偏移索引，再存放数据
    uart0_rx_buffer.head = (uart0_rx_buffer.head + 1) % UART0_RX_BUF_SIZE;
    uart0_rx_buffer.buffer[uart0_rx_buffer.head] = byte;

    uart0_rx_buffer.count++;

    if (uart0_rx_buffer.count > UART0_RX_BUF_SIZE) {
        uart0_rx_buffer.count = UART0_RX_BUF_SIZE;
    }
}
#endif
#ifdef UART1_EN
uart1_rx_buffer_t xdata uart1_rx_buffer;

void Uart1_Initial(unsigned long int baudrate)
{
    unsigned int value_temp;

    uart1_send.head = 0;
    uart1_send.tail = 0;
    uart1_rev.head = 0;
    uart1_rev.tail = 0;
    uart1_tx_flag = 0;

    uart1_rx_buffer.head = 0;
    uart1_rx_buffer.tail = 0;
    uart1_rx_buffer.count = 0;
    memset(uart1_rx_buffer.buffer, 0, UART1_RX_BUF_SIZE);

    value_temp = 0x400 - FOSC / (baudrate * 32);

    GPIO_Init(P11F, P11_UART1_TX_SETTING);
    GPIO_Init(P12F, P12_UART1_RX_SETTING);

    S1RELH = (unsigned char)(value_temp >> 8);
    S1RELL = (unsigned char)(value_temp);

    S1CON = 0xD0;
    ES1 = 1;
}
void Uart1_PutChar(unsigned char bdat)
{
    unsigned char free_space;
    unsigned char tail_tmp;

    while (1) {
        tail_tmp = uart1_send.tail;
        if (uart1_send.head < tail_tmp) {
            free_space = tail_tmp - uart1_send.head;
        } else {
            free_space = UART1_TX_BUF_SIZE + tail_tmp - uart1_send.head;
        }
        if (free_space > 1) {
            ES1 = 0;
            uart1_send.head++;
            uart1_send.head %= UART1_TX_BUF_SIZE;
            uart1_tx_buf[uart1_send.head] = bdat;
            if (!uart1_tx_flag) {
                ES1 = 1;
                uart1_send.tail++;
                uart1_send.tail %= UART1_TX_BUF_SIZE;
                S1BUF = uart1_tx_buf[uart1_send.tail];
                uart1_tx_flag = 1;
            } else {
                ES1 = 1;
            }
            break;
        }
    }
}
void UART1_ISR(void) interrupt 6
{
    if (S1CON & BIT0) {
        // S1CON = (S1CON&~(BIT0|BIT1))|BIT0;
        // uart1_rev.head++;
        // uart1_rev.head %= UART1_RX_BUF_SIZE;
        // uart1_rx_buf[uart1_rev.head]=S1BUF;

        S1CON = (S1CON & 0xFC) | 0x01;
        uart1_rxbuffer_put(S1BUF);
    }
    if (S1CON & BIT1) {
        S1CON = (S1CON & ~(BIT0 | BIT1)) | BIT1;
        if (uart1_send.head != uart1_send.tail) {
            uart1_send.tail++;
            uart1_send.tail %= UART1_TX_BUF_SIZE;
            S1BUF = uart1_tx_buf[uart1_send.tail];
        } else {
            uart1_tx_flag = 0;
        }
    }
}

u8 uart1_rxbuffer_get_count(void)
{
    return uart1_rx_buffer.count;
}

u8 uart1_rxbuffer_get(void)
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

#endif

#ifdef PRINT_EN
void UartPutStr(char *str)
{
    while (*str) {
#if (PRINT_PORT_SEL == 0)
        Uart0_PutChar(*str++);
#elif (PRINT_PORT_SEL == 1)
        Uart1_PutChar(*str++);
#elif (PRINT_PORT_SEL == 2)
        Uart2_PutChar(*str++);
#else
        str++;
#endif
    }
}
void uart_printf(char *fmt, ...)
{
    va_list ap;
    char xdata string[64];
    va_start(ap, fmt);
    vsprintf(string, fmt, ap);
    UartPutStr(string);
    va_end(ap);
}
#endif
/*********************************************************************************************************************/
#endif