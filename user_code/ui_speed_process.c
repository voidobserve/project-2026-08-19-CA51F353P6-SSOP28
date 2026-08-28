#include "ui_speed_process.h"
#include "user_typedef.h"
#include "aip3368h_display.h"
#include "ui.h"
#include "instrument.h"

#include "user_config.h"

// 计数器，计数满一段时间后，更新显示
static volatile u8 aip3368h_display_speed_refresh_time_cnt = 0;

// 滑动平均滤波的各项定义：
#define SPEED_FILTER_ARRAY_SIZE (40)
static volatile u16 speed_filter_array[SPEED_FILTER_ARRAY_SIZE] = {0};
static volatile u8 speed_filter_index = 0;

void speed_filter_init(u16 speed)
{
    u8 i;
    for (i = 0; i < SPEED_FILTER_ARRAY_SIZE; i++) {
        speed_filter_array[i] = speed;
    }

    speed_filter_index = 0;
}

void speed_filter_add(u16 speed)
{
    speed_filter_array[speed_filter_index] = speed;
    speed_filter_index++;
    if (speed_filter_index >= SPEED_FILTER_ARRAY_SIZE) {
        speed_filter_index = 0;
    }
}

u16 speed_filter_get_speed(void)
{
    u8 i;
    u32 sum = 0;
    u16 max_speed = 0;
    u16 min_speed = (u8)-1;

    // for (i = 0; i < SPEED_FILTER_ARRAY_SIZE; i++)
    // {
    //     printf("speed[%u] == %u\n", (u16)i, (u16)speed_filter_array[i]);
    // }
    // printf("\n===================\n");

    for (i = 0; i < SPEED_FILTER_ARRAY_SIZE; i++) {
        sum += speed_filter_array[i];

        if (max_speed < speed_filter_array[i]) {
            max_speed = speed_filter_array[i];
        }

        if (min_speed > speed_filter_array[i]) {
            min_speed = speed_filter_array[i];
        }
    }

    sum -= (max_speed + min_speed);

    return sum / (SPEED_FILTER_ARRAY_SIZE - 2);
}

/**
 * @brief 递增AIP3368H显示速度刷新时间计数
 * 	
 * @note  在1ms定时器中断内调用
 *
 */
void aip3368h_display_speed_refresh_time_add(void)
{
    if (!(UI_STATE_NORMAL == ui_manager.state ||
          UI_STATE_SETTING_DISTANCE_UNIT_TYPE == ui_manager.state)) {
        // 不在 正常界面 ，或者不在 设置 要显示的单位类型界面，直接返回
        aip3368h_display_speed_refresh_time_cnt = 0;
        return;
    }

    // 防止计数溢出
    if (aip3368h_display_speed_refresh_time_cnt < ((u8)-1)) {
        aip3368h_display_speed_refresh_time_cnt++;
    }
}

/**
 * @brief 根据单位类型（公制 或 英制）来显示时速
 *
 * @param speed 公制单位下的速度值
 */
void aip3368h_display_speed_by_unit_type(u16 speed)
{
    if (DISTANCE_UNIT_TYPE_METRIC == instrument.distance_unit_type) {
        // 显示时速
        aip3368h_display_speed(speed);
    } else if (DISTANCE_UNIT_TYPE_IMPERIAL == instrument.distance_unit_type) {
        // 1km/h == 0.621427mile/h
        // 显示时速
        aip3368h_display_speed((u32)speed * 621 / 1000);
    }
}

/**
 * @brief 根据当前时速值，来更新显示时速
 * 
 * @note 在主循环调用
 * 
 * @return * void 
 */
void ui_speed_process(void)
{
    static volatile u8 is_initialized = 0; // 是否初始化
    volatile u16 cur_speed;                //

    /*
        一阶低通滤波器配置，滤波系数 α ，范围：0 ~ 10，推荐值 1 ~ 3
        α越小越平滑但响应慢,α越大响应快但平滑差
    */
#define ALPHA 7 // 滤波系数 α
    static volatile u16 filtered_speed;
    u8 base_step;       // 显示时速期间，每次调节显示的步长
    u16 speed_abs_diff; // 存放当前显示的时速和当前时速的差值

    if (!(UI_STATE_NORMAL == ui_manager.state ||
          UI_STATE_SETTING_DISTANCE_UNIT_TYPE == ui_manager.state)) {
        // 不在 正常界面 ，或者不在 设置 要显示的单位类型界面，直接返回
        is_initialized = 0; //
        return;
    }

    if (0 == is_initialized) {
        is_initialized = 1;

        instrument.speed_to_display =
            instrument.speed; // 初始化，直接获取当前最新的速度值
        filtered_speed = instrument.speed;
        aip3368h_display_speed_by_unit_type(instrument.speed_to_display);
        speed_filter_init(instrument.speed);

        if (instrument.distance_unit_type == DISTANCE_UNIT_TYPE_METRIC) {
            // 公制单位： km/h
            aip3368h_display_mph_light(0);
            aip3368h_display_kmh_light(1);
        } else {
            // 英制单位： mile/h
            aip3368h_display_kmh_light(0);
            aip3368h_display_mph_light(1);
        }
    }

    if (aip3368h_display_speed_refresh_time_cnt >= UI_SPEED_REFRESH_PERIOD) {
        aip3368h_display_speed_refresh_time_cnt = 0;

        // 如果当前显示的速度值和计算出来的速度值相差太大，需要进行快速逼近：
        if (instrument.speed_to_display > instrument.speed) {
            speed_abs_diff = instrument.speed_to_display - instrument.speed;
            if (speed_abs_diff >= 10) {
                // 根据速度插值，调节步长
                if (speed_abs_diff >= 50) {
                    base_step = 20;
                } else if (speed_abs_diff >= 20) {
                    base_step = 10;
                } else {
                    base_step = 5;
                }

                instrument.speed_to_display -= base_step;
#if USER_DEBUG_ENABLE
                // printf("instrument.speed_to_display == %u\n", (u16)instrument.speed_to_display);
#endif

                speed_filter_init(instrument.speed);
                filtered_speed = instrument.speed;
                aip3368h_display_speed_by_unit_type(
                    instrument.speed_to_display);
                return;
            }
        } else if (instrument.speed_to_display < instrument.speed) {
            speed_abs_diff = instrument.speed - instrument.speed_to_display;
            if (speed_abs_diff >= 10) {
                // 根据速度插值，调节步长
                if (speed_abs_diff >= 50) {
                    base_step = 20;
                } else if (speed_abs_diff >= 20) {
                    base_step = 10;
                } else {
                    base_step = 5;
                }

                instrument.speed_to_display += base_step;
#if USER_DEBUG_ENABLE
                // printf("instrument.speed_to_display == %u\n", (u16)instrument.speed_to_display);
#endif

                speed_filter_init(instrument.speed);
                filtered_speed = instrument.speed;

                aip3368h_display_speed_by_unit_type(
                    instrument.speed_to_display);
                return;
            }
        }

        speed_filter_add(instrument.speed);
        cur_speed = speed_filter_get_speed();
        /*
            如果当前速度值和过滤后的速度值都是10以下，需要注意
            不能再经过低通滤波器，会导致最后计算出的时速变为0
        */
        if (cur_speed < 10) {
            filtered_speed = cur_speed;
        } else {
            // 套用一阶低通滤波器计算公式：
            filtered_speed =
                ((u32)ALPHA * cur_speed + (10 - ALPHA) * (u32)filtered_speed) /
                10;
        }

        cur_speed = filtered_speed;

        if (instrument.speed_to_display > cur_speed) {
            instrument.speed_to_display--;
        } else if (instrument.speed_to_display < cur_speed) {
            instrument.speed_to_display++;
        }

#if USER_DEBUG_ENABLE
        // printf("speed_to_display == %u\n", (u16)speed_to_display);
#endif
        aip3368h_display_speed_by_unit_type(instrument.speed_to_display);
    }
}