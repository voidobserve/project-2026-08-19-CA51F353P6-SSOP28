#ifndef _UART_H_
#define _UART_H_

#define UART0_TX_BUF_SIZE		20
#define UART0_RX_BUF_SIZE		100

#define UART1_TX_BUF_SIZE		20
#define UART1_RX_BUF_SIZE		100

#define UART2_TX_BUF_SIZE		20
#define UART2_RX_BUF_SIZE		100
typedef struct
{
	unsigned char head;
	unsigned char tail;
}T_Buf_Info;


#ifdef _UART_C_
#ifdef UART0_EN
	T_Buf_Info xdata	uart0_send;
	T_Buf_Info xdata	uart0_rev;
	unsigned char xdata uart0_tx_buf[UART0_TX_BUF_SIZE];
	unsigned char xdata uart0_rx_buf[UART0_RX_BUF_SIZE];
	bit uart0_tx_flag;
#endif
#ifdef UART1_EN
	T_Buf_Info xdata	uart1_send;
	T_Buf_Info xdata	uart1_rev;
	unsigned char xdata uart1_tx_buf[UART1_TX_BUF_SIZE];
	unsigned char xdata uart1_rx_buf[UART1_RX_BUF_SIZE];
	bit uart1_tx_flag;
#endif
#ifdef UART2_EN
	T_Buf_Info xdata	uart2_send;
	T_Buf_Info xdata	uart2_rev;
	unsigned char xdata uart2_tx_buf[UART2_TX_BUF_SIZE];
	unsigned char xdata uart2_rx_buf[UART2_RX_BUF_SIZE];
	bit uart2_tx_flag;
#endif
#else
#ifdef UART0_EN
	extern T_Buf_Info xdata	uart0_send;
	extern T_Buf_Info xdata	uart0_rev;
	extern unsigned char xdata uart0_tx_buf[UART0_TX_BUF_SIZE];
	extern unsigned char xdata uart0_rx_buf[UART0_RX_BUF_SIZE];
	extern bit uart0_tx_flag;	
#endif	
#ifdef UART1_EN
	extern T_Buf_Info xdata	uart1_send;
	extern T_Buf_Info xdata	uart1_rev;
	extern unsigned char xdata uart1_tx_buf[UART1_TX_BUF_SIZE];
	extern unsigned char xdata uart1_rx_buf[UART1_RX_BUF_SIZE];
	extern bit uart1_tx_flag;	
#endif
#ifdef UART2_EN
	extern T_Buf_Info xdata	uart2_send;
	extern T_Buf_Info xdata	uart2_rev;
	extern unsigned char xdata uart2_tx_buf[UART2_TX_BUF_SIZE];
	extern unsigned char xdata uart2_rx_buf[UART2_RX_BUF_SIZE];
	extern bit uart2_tx_flag;	
#endif
#endif
#ifndef UART0_EN
	  #define Uart0_PutChar(n)
#endif
#ifndef UART1_EN
	  #define Uart1_PutChar(n)
#endif
#ifndef UART2_EN
	  #define Uart2_PutChar(n)
#endif

#ifdef UART0_EN
void Uart0_PutChar(unsigned char bdat);
void Uart0_Initial(unsigned long int baudrate);
#endif	

#ifdef UART1_EN
void Uart1_PutChar(unsigned char bdat);
void Uart1_Initial(unsigned long int baudrate);

// 增加环形缓冲区结构体
typedef struct
{
	u8 buffer[UART0_RX_BUF_SIZE];
	u8 head;
	u8 tail;
	u8 count;
} uart0_rx_buffer_t;
// 增加环形缓冲区结构体
typedef struct
{
	u8 buffer[UART1_RX_BUF_SIZE];
	u8 head;
	u8 tail;
	u8 count;
} uart1_rx_buffer_t;

extern uart0_rx_buffer_t xdata uart0_rx_buffer;
u8 uart0_rxbuffer_get_count(void); // 获取接收缓冲区收到的数据个数
u8 uart0_rxbuffer_get(void);
void uart0_rxbuffer_put(u8 byte);

extern uart1_rx_buffer_t xdata uart1_rx_buffer;
u8 uart1_rxbuffer_get_count(void); // 获取接收缓冲区收到的数据个数
u8 uart1_rxbuffer_get(void);
void uart1_rxbuffer_put(u8 byte);
#endif	

#ifdef UART2_EN
void Uart2_PutChar(unsigned char bdat);
void Uart2_Initial(unsigned long int baudrate);
#endif	

#ifdef PRINT_EN
void uart_printf(char *fmt,...);
#endif

#endif
