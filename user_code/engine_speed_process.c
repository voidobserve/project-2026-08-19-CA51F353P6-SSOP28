#include "engine_speed_process.h"

#include "instrument.h"
#include "aip3368h_display.h"

#include "gpiodef_f3.h" // GPIO_Init()
#include "ca51f3xsfr.h"
#include "ca51f3sfr.h"

#include "user_config.h"
#if USER_DEBUG_ENABLE
#include <stdio.h>
#endif

// 由采集端通过串口发送过来的发动机转速
volatile u32 rpm_from_collector;

volatile u32 engine_speed_scan_cnt; // 检测到的脉冲个数，在定时器中断累加
volatile u16 engine_speed_scan_ms;  // 在定时器中断累加

static volatile u32 cur_engine_speed_scan_cnt;
static volatile u32 cur_engine_speed_scan_ms;

volatile u8 flag_is_engine_speed_scan_over_time; // 标志位，检测是否超时

// 计数器，计数满一段时间后，更新显示
static volatile u16 aip3368h_display_engine_speed_refresh_time_cnt = 0;

// 发动机转速的相关配置
void engine_speed_scan_config(void)
{
    GPIO_Init(P16F, INPUT);
}

void update_engine_speed_scan_data(void) // 更新检测发动机转速的数据
{
    cur_engine_speed_scan_cnt += engine_speed_scan_cnt;
    engine_speed_scan_cnt = 0;
    cur_engine_speed_scan_ms += engine_speed_scan_ms;
    engine_speed_scan_ms = 0;
}

void engine_speed_scan_timer_50us_isr(void)
{
    // 记录上一次检测到的引脚电平（发送机转速检测脚）
    static volatile bit last_engine_speed_scan_level = 0;
    // 记录发动机转速扫描的时间
    static u8 cnt = 0;

    cnt++;
    if (cnt >= 20) // 每1ms进入一次
    {
        cnt = 0;
        engine_speed_scan_ms++;

        if (engine_speed_scan_ms >= ENGINE_SPEED_SCAN_OVER_TIME &&
            flag_is_engine_speed_scan_over_time == 0) {
            engine_speed_scan_ms = 0;
            // 说明超时，脉冲计数一直没有加一
            flag_is_engine_speed_scan_over_time = 1;
        }
    }

    if (ENGINE_SPEED_SCAN_PIN) // 检测发动机转速的引脚
    {
        if (0 == last_engine_speed_scan_level) {
            // 如果之前检测到低电平，现在检测到高电平，说明有上升沿，对脉冲计数加一
            engine_speed_scan_cnt++;
            update_engine_speed_scan_data();
        }

        last_engine_speed_scan_level = 1;
    } else {
        // 如果现在检测到低电平
        last_engine_speed_scan_level = 0;
    }
}

// 发动机转速扫描
void engine_speed_scan(void)
{
#define CONVER_ONE_MINUTE_TO_MS (60000UL) // 将1min转换成以ms为单位的数据
    volatile u8 is_local_rpm_update = 0;
    // 转速值是否有效，如果无效，不给 instrument.engine_speed 赋值
    volatile u8 is_rpm_valid = 0;
    // 初始化为 0，防止后续比较时使用未初始化的值
    static volatile u32 rpm = 0;
    volatile u32 final_rpm = 0;

    if (cur_engine_speed_scan_ms >= ENGINE_SPEED_SCAN_UPDATE_TIME ||
        flag_is_engine_speed_scan_over_time) {
#if USER_DEBUG_ENABLE
// printf("cur_engine_speed_scan_ms:%lu\n", cur_engine_speed_scan_ms);
#endif
        if (flag_is_engine_speed_scan_over_time) {
            flag_is_engine_speed_scan_over_time = 0;
            rpm = 0;
        } else {
            /*
                扫描时间内转过的圈数 == 一个脉冲对应转过的圈数 * 扫描时间内采集到的脉冲个数
                1min转过的圈数 == 扫描时间内转过的圈数 / 扫描时间 * 1min
            */
            rpm = (u32)cur_engine_speed_scan_cnt *
                  ENGINE_SPEED_SCAN_A_PULSE_PER_TURNS *
                  (CONVER_ONE_MINUTE_TO_MS / ENGINE_SPEED_SCAN_COMPONSATION) /
                  cur_engine_speed_scan_ms;
        }
#if USER_DEBUG_ENABLE
        // 打印检测到的脉冲个数
        // printf("cur engine speed pulse cnt:%lu\n", cur_engine_speed_scan_cnt);
#endif

        cur_engine_speed_scan_cnt = 0;
        cur_engine_speed_scan_ms = 0;

        // 限制得到的发动机转速
        if (rpm >= 65535) {
            rpm = 65535;
        }

        is_local_rpm_update = 1;

#if USER_DEBUG_ENABLE
        // printf("cur rpm %lu\n", rpm);
#endif
    }

    // 本地的转速有更新，则赋值。后续比较如果发现采集端的转速更大，则用采集端的转速
    if (is_local_rpm_update) {
        is_local_rpm_update = 0;
        final_rpm = rpm;
        is_rpm_valid = 1;
    }

    if (rpm_from_collector > final_rpm) {
        final_rpm = rpm_from_collector;
        is_rpm_valid = 1;
    }

    if (is_rpm_valid) {
        is_rpm_valid = 0;

#if USER_DEBUG_ENABLE
        // printf("final_rpm == %lu\n", final_rpm);
#endif
        instrument.engine_speed = final_rpm;
    }
}

/**
 * @brief 累加发动机转速的显示刷新时间
 * 
 */
void aip3368h_display_engine_speed_refresh_time_add(void)
{
    if (aip3368h_display_engine_speed_refresh_time_cnt < ((u16)-1)) {
        aip3368h_display_engine_speed_refresh_time_cnt++;
    }
}

// 将采集到的发动机转速转换为仪表对应的转速滑动条挡位
u8 engine_speed_get_level(void)
{
    u8 level = instrument.engine_speed / 500; // 仪表上的一格对应 500 rpm

    // 为了节省空间，这里可以删掉，对应的显示函数可以处理溢出的情况
    // if (level > 13 * 2) {
    //     level = 13 * 2;
    // }

    return level;
}

void aip3368h_display_engine_speed_handle(void)
{
    // 延迟显示的发动机转速
    static u8 engine_speed_level_of_lag = 0;
    static u8 is_initialized = 0;
    u8 cur_engine_speed_level = 0;
    u8 level_diff;              // 显示的和实际计算得到的挡位插值
    u16 refresh_time_threshold; // 刷新间隔阈值

    if (is_initialized == 0) {
        is_initialized = 1;
        engine_speed_level_of_lag = engine_speed_get_level();
        // aip3368h_display_x1000rpm_light(1);
    }

    // 如果当前发动机转速与显示的发动机转速很接近，延长刷新时间（样机大约是2s）
    cur_engine_speed_level = engine_speed_get_level();
    if (cur_engine_speed_level == 0) {
        // 转速为0时，快速更新
        refresh_time_threshold = AIP3368H_DISPLAY_ENGINE_SPEED_REFRESH_TIME;
    } else {
        level_diff = (cur_engine_speed_level > engine_speed_level_of_lag)
                         ? (cur_engine_speed_level - engine_speed_level_of_lag)
                         : (engine_speed_level_of_lag - cur_engine_speed_level);

        refresh_time_threshold =
            (level_diff <= 1) ? (AIP3368H_DISPLAY_ENGINE_SPEED_REFRESH_TIME * 2)
                              : AIP3368H_DISPLAY_ENGINE_SPEED_REFRESH_TIME;
    }

    if (aip3368h_display_engine_speed_refresh_time_cnt >=
        refresh_time_threshold) {
        aip3368h_display_engine_speed_refresh_time_cnt = 0;
        if (engine_speed_level_of_lag < cur_engine_speed_level) {
            engine_speed_level_of_lag++;
        } else if (engine_speed_level_of_lag > cur_engine_speed_level) {
            if (engine_speed_level_of_lag > 0) {
                engine_speed_level_of_lag--;
            }
        }

        aip3368h_display_engine_speed_lev(engine_speed_level_of_lag);
    }
}
