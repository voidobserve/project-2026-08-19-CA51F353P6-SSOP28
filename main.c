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

    ui_timer_handle_isr();
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

    // tick_timer_config(); //定时器1初始化
    aip3368h_module_init();
    // aip3368h_module_display();

    ui_manager_init();

#if 0
	AD_Init();
#endif

    // powup_en = 1;
    // powup_stp = 0;
    // act_tmp = 0;

    // save_en = 1;
    // init_en = 1;

    printf("sys init\n");

    aip3368h_display_test();

    while (1) {
        // Mcu_FeedDog(); //喂狗
        WDFLG = 0xA5; // 喂狗

        // my_loop();

        ui_display_handle();
    }
}

// #endif
