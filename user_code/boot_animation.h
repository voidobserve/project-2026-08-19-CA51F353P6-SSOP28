#ifndef __BOOT_ANIMATION_H__
#define __BOOT_ANIMATION_H__

#include "user_typedef.h"

// 开机动画总时间
#define BOOT_ANIMATION_TOTAL_TIME ((u16)2000)

void boot_animation_time_base_add_1ms_isr(void);
void boot_animation_process(void);

#endif
