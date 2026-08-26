#include "boot_animation.h"
#include "ca51f3sfr.h" // WDFLG
#include "user_typedef.h"
#include <string.h> // memset
#include "aip3368.h"

volatile u16 boot_animation_param_total_time_cnt = 0;

void boot_animation_time_base_add_1ms_isr(void)
{
    if (boot_animation_param_total_time_cnt < ((u16)-1)) {
        boot_animation_param_total_time_cnt++;
    }
}

void boot_animation_process(void)
{
    // 直接让所有指示灯点亮：
    memset(aip3368h_speed_panel_display_buff, 0xFF,
           sizeof(aip3368h_speed_panel_display_buff));
    memset(aip3368h_engine_speed_panel_display_buff, 0xFF,
           sizeof(aip3368h_engine_speed_panel_display_buff));

    while (1) {
        WDFLG = 0xA5; // 喂狗

        if (boot_animation_param_total_time_cnt >= BOOT_ANIMATION_TOTAL_TIME) {
            // 开机动画结束，退出

			// 关闭手机图标对应的指示灯
			aip3368h_engine_speed_panel_display_buff[5] &= ~(0x01 << 15); 
            break;
        }

        aip3368h_module_display();
    }
}