#ifndef __TIMER1_H__
#define __TIMER1_H__

#include "ca51f_config.h"

// 定时时间，单位为us
#define TIMER1_CNT_PERIOD ((u16)50)
#define TIMER1_HVAL (u8)((0x10000 - ((u32)TIMER1_CNT_PERIOD * (FOSC / 1000)) / 12000) >> 8)
#define TIMER1_LVAL (u8)(0x10000 - ((u32)TIMER1_CNT_PERIOD * (FOSC / 1000)) / 12000)

#if 1
// TEST ONLY 观察定时器的计数器值
enum
{
	TIMER1_HVAL_TEST = TIMER1_HVAL,
	TIMER1_LVAL_TEST = TIMER1_LVAL,
};
#endif

void timer1_init(void);

#endif

