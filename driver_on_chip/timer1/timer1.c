#include "timer1.h"
#include "ca51f3sfr.h"

#include "user_debug.h"

void timer1_init(void)
{
    TMOD &= 0xCF; //
    TMOD |= 0x10; //

    TH1 = TIMER1_HVAL;
    TL1 = TIMER1_LVAL;

    TR1 = 1; // 定时器1使能
    ET1 = 1; // 定时器1中断使能
}

void timer1_isr(void) interrupt 3
{
    // 触发中断后，重新写入计数值
    TH1 = TIMER1_HVAL;
    TL1 = TIMER1_LVAL;

	// TODO 待测试中断周期是否正确
	user_debug_io_toggle();
}