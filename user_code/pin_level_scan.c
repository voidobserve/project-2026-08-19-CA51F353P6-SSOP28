#include "pin_level_scan.h"
#include "user_config.h"

#include "gpiodef_f3.h" // GPIO_Init()
#include "ca51f3xsfr.h"
#include "ca51f3sfr.h"

#include "aip3368h_display.h"

void pin_level_scan_init(void)
{
    GPIO_Init(P14F, INPUT); // 引脚有外部上拉，不用再配置上拉

    GPIO_Init(P15F, INPUT); // 引脚有外部上拉，不用再配置上拉
}

// 引脚电平扫描
void pin_level_handle(void)
{
    if (SIGNAL_VALID_LEV_OF_ABS == PIN_DETECT_ABS) {
        aip3368h_display_abs_light(1);
    } else {
        aip3368h_display_abs_light(0);
    }

    if (SIGNAL_VALID_LEV_OF_ENGINE_ERR == PIN_DETECT_ENGINE_ERR) {
        aip3368h_display_engine_err_light(1);
    } else {
        aip3368h_display_engine_err_light(0);
    }
}
