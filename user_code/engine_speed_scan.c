#include "engine_speed_scan.h"

#include "instrument.h"
#include "aip3368h_display.h"

#include "gpiodef_f3.h" // GPIO_Init()
#include "ca51f3xsfr.h"
#include "ca51f3sfr.h"

#if ENGINE_SPEED_SCAN_ENABLE

volatile u32 engine_speed_scan_cnt; // 检测到的脉冲个数，在定时器中断累加
volatile u16 engine_speed_scan_ms;  // 在定时器中断累加

static volatile u32 cur_engine_speed_scan_cnt;
static volatile u32 cur_engine_speed_scan_ms;

volatile u8 flag_is_engine_speed_scan_over_time; // 标志位，检测是否超时
// ==========================================================================
// ==========================================================================
volatile u32 engine_speed_scan_cnt_expend; // 检测到的脉冲个数，在定时器中断累加
volatile u16 engine_speed_scan_ms_expend;  // 在定时器中断累加

static volatile u32 cur_engine_speed_scan_cnt_expend;
static volatile u32 cur_engine_speed_scan_ms_expend;

volatile bit flag_is_engine_speed_scan_over_time_expend; // 标志位，检测是否超时

// 计数器，计数满一段时间后，更新显示
static volatile u16 aip3368h_display_engine_speed_refresh_time_cnt = 0;

// 发动机转速的相关配置
void engine_speed_scan_config(void)
{
    // GPIO_Init(P02F, OUTPUT);
}

void update_engine_speed_scan_data(void) // 更新检测发动机转速的数据
{
    cur_engine_speed_scan_cnt += engine_speed_scan_cnt;
    engine_speed_scan_cnt = 0;
    cur_engine_speed_scan_ms += engine_speed_scan_ms;
    engine_speed_scan_ms = 0;
}

void update_engine_speed_scan_data_expend(void)
{
    cur_engine_speed_scan_cnt_expend += engine_speed_scan_cnt_expend;
    engine_speed_scan_cnt_expend = 0;
    cur_engine_speed_scan_ms_expend += engine_speed_scan_ms_expend;
    engine_speed_scan_ms_expend = 0;
}

void engine_speed_scan_timer_50us_isr(void)
{
    // 记录上一次检测到的引脚电平（发送机转速检测脚）
    static volatile bit last_engine_speed_scan_level = 0;
    static volatile bit last_engine_speed_scan_level_expend = 0;
    // 记录发动机转速扫描的时间
    static u8 cnt = 0;

    cnt++;
    if (cnt >= 20) // 每1ms进入一次
    {
        cnt = 0;
        engine_speed_scan_ms++;
        // 为 expend 侧也累加 ms
        engine_speed_scan_ms_expend++;

        if (engine_speed_scan_ms >= ENGINE_SPEED_SCAN_OVER_TIME &&
            flag_is_engine_speed_scan_over_time == 0) {
            engine_speed_scan_ms = 0;
            // 说明超时，脉冲计数一直没有加一
            flag_is_engine_speed_scan_over_time = 1;
        }

        if (engine_speed_scan_ms_expend >= ENGINE_SPEED_SCAN_OVER_TIME &&
            flag_is_engine_speed_scan_over_time_expend == 0) {
            engine_speed_scan_ms_expend = 0;
            // 说明超时，脉冲计数一直没有加一
            flag_is_engine_speed_scan_over_time_expend = 1;
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

    if (ENGINE_SPEED_SCAN_PIN_EXPEND) {
        if (0 == last_engine_speed_scan_level_expend) {
            // 如果之前检测到低电平，现在检测到高电平，说明有上升沿，对脉冲计数加一
            engine_speed_scan_cnt_expend++;
            update_engine_speed_scan_data_expend();
        }

        last_engine_speed_scan_level_expend = 1;
    } else {
        last_engine_speed_scan_level_expend = 0;
    }
}

// 发动机转速扫描
void engine_speed_scan(void)
{
#define CONVER_ONE_MINUTE_TO_MS (60000UL) // 将1min转换成以ms为单位的数据
    // 初始化为 0，防止后续比较时使用未初始化的值
    volatile u32 rpm = 0;
    // 初始化为 0，防止后续比较时使用未初始化的值
    volatile u32 rpm_expend = 0;
    volatile bit is_rpm_update = 0; // 是否有新的非0的转速值更新

    volatile u32 final_rpm = 0;

    if (cur_engine_speed_scan_ms >= ENGINE_SPEED_SCAN_UPDATE_TIME ||
        flag_is_engine_speed_scan_over_time) {
#if USER_DEBUG_ENABLE
// printf("cur_engine_speed_scan_ms:%lu\n", cur_engine_speed_scan_ms);
#endif
        if (flag_is_engine_speed_scan_over_time) {
            // flag_is_engine_speed_scan_over_time = 0;
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

            is_rpm_update = 1;
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

#if USER_DEBUG_ENABLE
        // printf("cur rpm %lu\n", rpm);
#endif
    }

    if (cur_engine_speed_scan_ms_expend >= ENGINE_SPEED_SCAN_UPDATE_TIME ||
        flag_is_engine_speed_scan_over_time_expend) {
        if (flag_is_engine_speed_scan_over_time_expend) {
            // flag_is_engine_speed_scan_over_time_expend = 0;
            rpm_expend = 0;
        } else {
            /*
                扫描时间内转过的圈数 == 一个脉冲对应转过的圈数 * 扫描时间内采集到的脉冲个数
                1min转过的圈数 == 扫描时间内转过的圈数 / 扫描时间 * 1min
            */
            rpm_expend = (u32)cur_engine_speed_scan_cnt_expend *
                         ENGINE_SPEED_SCAN_A_PULSE_PER_TURNS_EXPEND *
                         (CONVER_ONE_MINUTE_TO_MS /
                          ENGINE_SPEED_SCAN_COMPONSATION_EXPEND) /
                         cur_engine_speed_scan_ms_expend;

            is_rpm_update = 1;
        }

        cur_engine_speed_scan_cnt_expend = 0;
        cur_engine_speed_scan_ms_expend = 0;

        // 限制得到的发动机转速
        if (rpm_expend >= 65535) {
            rpm_expend = 65535;
        }

#if USER_DEBUG_ENABLE
        // printf("cur rpm %lu\n", rpm);
#endif
    }

    if (is_rpm_update) {
        is_rpm_update = 0;

        // 取采集到的最大转速
        if (0 == flag_is_engine_speed_scan_over_time) {
            // 如果没超时，取得转速
            final_rpm = rpm;
        }

        if (0 == flag_is_engine_speed_scan_over_time_expend) {
            // 如果没超时，取得转速
            if (final_rpm < rpm_expend) {
                final_rpm = rpm_expend;
            }
        }

        // TODO instrument 还没有完善
        // instrument.engine_speed = final_rpm;
    } else if (flag_is_engine_speed_scan_over_time &&
               flag_is_engine_speed_scan_over_time_expend) {
        // 如果两个检测脚都检测不到脉冲，一直超时
        flag_is_engine_speed_scan_over_time = 0;
        flag_is_engine_speed_scan_over_time_expend = 0;
        /*
            REVIEW 必须要两个引脚都检测超时，才给这两个标志位都清零，
            如果提前清除了其中一个标志位，这里的判断条件就会无法进入
        */

        // TODO instrument 还没有完善
        // instrument.engine_speed = 0;
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
// TODO instrument 还没有完善
#if 0
    u8 level = instrument.engine_speed / 500; // 仪表上的一格对应 500 rpm

    // 为了节省空间，这里可以删掉，对应的显示函数可以处理溢出的情况
    // if (level > 13 * 2) {
    //     level = 13 * 2;
    // }

    return level;
#else
    return 0;
#endif
}

#if 0
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
        aip3368h_display_engine_speed_scale_bar(1);
        aip3368h_display_x1000rpm_light(1);
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

        if (engine_speed_level_of_lag > 0) {
            // 转速大于第 0 挡时，至少要显示第 0 挡对应的指示灯，仪表上第 0 挡对应 0 * 1000RPM
            aip3368h_display_engine_speed_gear(engine_speed_level_of_lag + 1);
        } else {
            aip3368h_display_engine_speed_gear(0);
        }
    }
}
#endif

#endif //  ENGINE_SPEED_SCAN_ENABLE
