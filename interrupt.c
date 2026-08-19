#ifndef _INTERRUPT_C_
#define _INTERRUPT_C_
/*********************************************************************************************************************/
#include "ca51f_config.h"
#include "includes\ca51f3sfr.h"
#include "includes\ca51f3xsfr.h"
#include "includes\gpiodef_f3.h"

#include "Library\includes\delay.h"
#include "Library\includes\uart.h"
#include "Library\includes\system_clock.h"
#include "Library\Includes\rtc.h"
#include "includes\system.h"
/*********************************************************************************************************************/
#include "TS_Lib\Includes\ts_configuration.h"
#include "TS_Lib\Includes\ts_def.h"
#include "TS_Lib\Includes\ts_api.h"
#include "TS_Lib\Includes\ts_service.h"
/*********************************************************************************************************************/
#if DEBUG
#include "Library\Includes\i2c.h"
void I2C_ISR(void);
#endif
void INT8_ISR(void) interrupt 13
{
    if (RTCIF & RTC_MF) //毫秒中断	   时间为 (RTMSS+1)*128*Trtc,
    {
        RTCIF = RTC_MF;
        TS_MS_ISR();
        if (TS_LongKeyTimer) {
            TS_LongKeyTimer--;
        }
    }
    if (RTCIF & RTC_HF) //半秒中断
    {
        RTCIF = RTC_HF;
        TS_HS_ISR();
    }
}
void INT3_ISR(void) interrupt 8
{
    if (TKIF != 0) {
        TS_ISR();
    }
}
void INT6_ISR(void) interrupt 11
{
#if DEBUG
    if (I2CFLG & I2CF) {
        I2C_ISR();
    }
#endif
}
#endif