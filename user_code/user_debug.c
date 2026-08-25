#include "user_debug.h"
#include "gpiodef_f3.h" // GPIO_Init()
#include "ca51f3xsfr.h"
#include "ca51f3sfr.h"

#if USER_DEBUG_ENABLE

void user_debug_io_init(void)
{
    GPIO_Init(P35F, OUTPUT);
}

void user_debug_io_toggle(void)
{
    USER_DEBUG_IO ^= USER_DEBUG_IO;
}

#endif