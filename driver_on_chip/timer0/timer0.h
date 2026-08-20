#ifndef __TIMER0_H__
#define __TIMER0_H__

#include "user_typedef.h"

#include "ca51f_config.h"

// 定时时间，单位为us
#define TIMER0_CNT_PERIOD ((u16)1000)
#define TIMER0_HVAL (u8)((0x10000 - ((u32)TIMER0_CNT_PERIOD * (FOSC / 1000)) / 12000) >> 8)
#define TIMER0_LVAL (u8)(0x10000 - ((u32)TIMER0_CNT_PERIOD * (FOSC / 1000)) / 12000)

#if 0
// TEST ONLY 观察定时器0的计数器值
enum
{
	TIMER0_HVAL_TEST = TIMER0_HVAL,
	TIMER0_LVAL_TEST = TIMER0_LVAL,
};
#endif

void timer0_init(void);

#endif
