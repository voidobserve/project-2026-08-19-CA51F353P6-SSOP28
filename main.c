// 芯片官方提供的头文件：
#include "ca51f_config.h"
#include "includes\ca51f3sfr.h"
#include "includes\ca51f3xsfr.h"
#include "includes\gpiodef_f3.h"
#include "system_clock.h"
#include "wdt.h"

#include "user_config.h"
#if USER_DEBUG_ENABLE
#include <stdio.h>
#include "user_debug.h"
#endif
// 自定义的芯片片上外设驱动
#include "uart.h"
#include "timer0.h"
#include "timer1.h"

// 自定义的功能驱动
#include "aip3368.h"
#include "aip3368h_display.h"

//
#include "ui.h"
#include "boot_animation.h"

//***************************************************************
void SystemInit(void)
{
#ifdef LVD_RST_ENABLE
    LVDCON = 0xE1; //设置LVD复位电压为2V
#endif

#if (SYSCLK_SRC == PLL)
    Sys_Clk_Set_PLL(PLL_Multiple);
#endif

/***************************看门狗阈值设置计算公式如下****************************
看门狗触发时间 = (WDVTH * 800H+7FFH) * clock cycle
当看门狗时钟为3.6864M时,时间覆盖范围为0.56ms--36s
*********************************************************************************/
//备注：需默认开启看门狗功能，并将其设置为复位模式
#if (RTC_CLK_SELECT == IRCL)
    CKCON |= ILCKE; //使能IRCL
    WDCON = WDTS(WDTS_IRCL) |
            WDRE(WDRE_reset); //设置看门狗时钟源为IRCL，模式为复位模式
#elif (RTC_CLK_SELECT == XOSCL)
    CKCON |= XLCKE;
    while (!(CKCON & XLSTA))
        ;
    WDCON = WDTS(WDTS_XOSCL) |
            WDRE(WDRE_reset); //设置看门狗时钟源为XOSCL，模式为复位模式
#endif

    WDVTHH = 0;  //看门狗复位阈值高八位设置 当前值为5s
    WDVTHL = 75; //看门狗复位阈值低八位设置
    WDFLG = 0xA5;
}

// TEST ONLY
volatile uint8_t user_debug_1ms_cnt = 0;

void user_1ms_isr(void)
{

    if (user_debug_1ms_cnt < ((u8)-1)) {
        user_debug_1ms_cnt++;
    }

    boot_animation_time_base_add_1ms_isr();
    ui_timer_handle_isr();

#if AIP3368H_DISPLAY_TEST_ENABLE
    // aip3368h_display_engine_speed_lev_test_1ms_isr();
    // aip3368h_display_gear_test_1ms_isr();
    // aip3368h_display_bat_lev_light_test_1ms_isr();
    // aip3368h_display_time_test_1ms_isr();
#endif
}

void main(void)
{

    SystemInit();
    EA = 1;

#if USER_DEBUG_ENABLE
    user_debug_io_init();
#endif
    uart1_init();
    timer0_init();
    timer1_init();

    aip3368h_module_init();
    // aip3368h_module_display();

    ui_manager_init();

    printf("sys init\n");

    // boot_animation_process();

    while (1) {
        WDFLG = 0xA5; // 喂狗

        if (user_debug_printf_enable) {
            user_debug_printf_enable = 0;
            printf("test\n");
        }

        ui_display_handle();
    }
}

// #endif
