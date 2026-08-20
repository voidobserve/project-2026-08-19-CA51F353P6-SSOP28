#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

/*********************************************************************************************************************/
#include "ca51f_config.h"
#include "includes\ca51f3sfr.h"
#include "includes\ca51f3xsfr.h"
#include "includes\gpiodef_f3.h"

#include "Library\includes\delay.h"
#include "uart.h"
#include "Library\includes\system_clock.h"
#include "Library\Includes\rtc.h"
#include "Library\Includes\pwm.h"
#include "includes\system.h"
#include "Library\includes\adc.h"
#include "Library\includes\wdt.h" 
// 用户自定义的数据类型
#include "user_typedef.h"

#include <stdio.h>
#include <string.h>

#include "uart_process_data_packets.h"
#include "user_aip3368h/aip3368h_module.h"
#include "user_aip3368h/aip3368h_app.h"
#include "user_aip3368h/ticks_module.h"

// #define		com_test		P31

#define USER_DEBUG_ENABLE 1

extern unsigned char act_tmp;
extern idata unsigned char hour_var;
extern idata unsigned char min_var;
extern idata unsigned char sec_var;
extern idata unsigned int pwm_tmp;
extern idata unsigned int pwm_var;

extern bit key_deb_en;
extern bit deal_showdat_en;
extern bit chk_ad_en;
extern bit fade_en;
extern bit fade_sta_en;

#endif
