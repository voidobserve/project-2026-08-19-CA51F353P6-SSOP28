#ifndef __USER_DEBUG_H__
#define __USER_DEBUG_H__

#include "user_config.h"
#include "user_typedef.h"

#if USER_DEBUG_ENABLE

#define USER_DEBUG_IO P35

extern volatile u8 user_debug_printf_enable;

void user_debug_io_init(void);
void user_debug_io_toggle(void);

#endif
#endif

