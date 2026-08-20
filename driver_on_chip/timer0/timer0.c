#include "timer0.h"

#include "ca51f3sfr.h" // 寄存器定义

#include "aip3368.h"

void timer0_init(void)
{ 
    TMOD &= 0xFC;      // 使能定时器 0 ，定时器0配置为计数器
    TMOD |= 0x01;      // 定时器0配置为模式1，组成16位
    TH0 = TIMER0_HVAL; // 高8位装初值
    TL0 = TIMER0_LVAL; // 低8位装初值

    TR0 = 1; //定时器0使能
    ET0 = 1; //定时器0中断使能
}

extern void test_timer0_cnt_add(void);

void timer0_isr(void) interrupt 1
{
    // 触发中断后，重新写入计数值
    TH0 = TIMER0_HVAL;
    TL0 = TIMER0_LVAL;

    aip3368h_refresh_time_add();    
}
