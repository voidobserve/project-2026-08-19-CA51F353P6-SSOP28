#ifndef TICKS_MODULE_H
#define TICKS_MODULE_H

#include "user_typedef.h"

#define SYSTIME_FLAG_5MS   0x01
#define SYSTIME_FLAG_10MS  0x02
#define SYSTIME_FLAG_25MS  0x04
#define SYSTIME_FLAG_50MS  0x08
#define SYSTIME_FLAG_60MS  0x20
#define SYSTIME_FLAG_100MS 0x10
// #define SYSTIME_FLAG_200MS      0x20
#define SYSTIME_FLAG_500MS  0x40
#define SYSTIME_FLAG_1000MS 0x80
#define SYSTIME_FLAG_ALL    0xFF

u8 systimer_flag_is_valid(u8 flag);
void systimer_flag_update(void);
void systimer_set_flag(u8 flag);
void tick_timer_config(void);
void tick_timer_config_close(void);
void idle_mode_timer_wakeup_config(void);

#endif
