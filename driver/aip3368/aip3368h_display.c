#include "aip3368h_display.h"
#include "aip3368.h"

#include "user_config.h"

#include "instrument.h"

// #include <string.h>
#if USER_DEBUG_ENABLE
#include <stdio.h> //
#include "user_debug.h"
#endif

/**
 * @brief 7段数码管段码定义 (a,b,c,d,e,f,g)
 *        对应二进制位: bit0=a, bit1=b, bit2=c, bit3=d, bit4=e, bit5=f, bit6=g
 */
static const u8 digit_segment_code[10] = {
    0x3F, // 0: abcdef
    0x06, // 1: bc
    0x5B, // 2: abdeg
    0x4F, // 3: abcdg
    0x66, // 4: bcfg
    0x6D, // 5: acdfg
    0x7D, // 6: acdefg
    0x07, // 7: abc
    0x7F, // 8: abcdefg
    0x6F  // 9: abcdfg
};

/*
    指示灯和显存的映射关系
    发动机转速对应的格数，从低到高
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t engine_speed_gear_map[] = {
    // 发动机转速，第 0 格指示灯（从左往右数，从0开始，白）
    {1, 4},  {1, 3},  {1, 2},  {1, 1},  {1, 0}, {2, 15}, {2, 14}, {2, 13},
    {2, 12}, {2, 11}, {2, 10}, {2, 9},  {2, 8}, {2, 7},  {2, 6},  {2, 5},
    {2, 4},  {2, 3},  {2, 2},  {2, 1},  {2, 0}, {3, 15}, {3, 14}, {3, 13},
    {3, 12}, {0, 15}, {3, 11}, {3, 10}, {3, 9},
};

/*
    指示灯和显存的映射关系
    挡位对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t gear_map[] = {
    // a 段 ~ g 段
    {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 6},
};

/*
    指示灯和显存的映射关系
    电池电量对应的指示灯
    对应的显存： aip3368h_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t bat_lev_map[] = {
    {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6},
};

/*
    指示灯和显存的映射关系
    小时个位对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t hour_bit_0_map[] = {
    // a 段 ~ g 段
    {7, 8}, {7, 9}, {7, 10}, {7, 11}, {7, 12}, {7, 13}, {7, 14},
};

/*
    指示灯和显存的映射关系
    小时十位对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t hour_bit_1_map[] = {
    // a 段 ~ g 段
    {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7},
};

/*
    指示灯和显存的映射关系
    分钟个位对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t minute_bit_0_map[] = {
    // a 段 ~ g 段
    {6 ,8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14},
};

/*
    指示灯和显存的映射关系
    分钟十位对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t minute_bit_1_map[] = {
    // a 段 ~ g 段
    {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6}, {6, 7},
};

/**
 * @brief 根据传参,显示发动机转速挡位
 * 
 * @param lev 档位等级 0 ~ 28
 * 
 *  0: 显示第 0 格指示灯
 *  1: 显示第 0 ~ 1 格指示灯
 * 
 * @note 没有转速时,固定显示第 0 格指示灯
 * 
 */
void aip3368h_display_engine_speed_lev(u8 lev)
{
    // 清空原来的显示
    u8 i;

    for (i = 0; i < ARRAY_SIZE(engine_speed_gear_map); i++) {
        if (lev < i) {
            // 如果传参的挡位，比当前遍历的挡位还要小，清空对应的显示
            aip3368h_engine_speed_panel_display_buff[engine_speed_gear_map[i]
                                                         .buff_index] &=
                ~(0x01 << engine_speed_gear_map[i].bit_offset);
        } else {
            // 如果传参的挡位，大于等于当前遍历的挡位
            aip3368h_engine_speed_panel_display_buff[engine_speed_gear_map[i]
                                                         .buff_index] |=
                (0x01 << engine_speed_gear_map[i].bit_offset);
        }
    }
}

/**
 * @brief N 字样对应的指示灯
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_n_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[4] |= 0x01 << 9;
    } else {

        aip3368h_speed_panel_display_buff[4] &= ~(0x01 << 9);
    }
}

/**
 * @brief 显示挡位
 *
 * @param gear 挡位 0 ~ 6、GEAR_UNKNOWN
 *          0：空挡，
 *          1：1挡，显示1
 *          GEAR_UNKNOWN ： 表示什么挡位都没有
 */
void aip3368h_display_gear(u8 gear)
{
    // 清空原来的显示
    u8 i;
    u8 segment_code;

    // 清空数码管的显示
    for (i = 0; i < ARRAY_SIZE(gear_map); i++) {
        aip3368h_engine_speed_panel_display_buff[gear_map[i].buff_index] &=
            ~(0x01 << gear_map[i].bit_offset);
    }

    if (GEAR_NEUTRAL == gear) {
        aip3368h_display_n_light(1);
    } else {
        aip3368h_display_n_light(0);
    }

    if (GEAR_NEUTRAL == gear || GEAR_UNKNOWN == gear) {
        // 如果什么挡位都没有，或者是检测到了空挡
    } else {
        // 1 ~ 6 挡，显示对应数字

        // 获取该数字对应的7段码 (要显示的数字 --> 七段码)
        segment_code = digit_segment_code[gear];

        // 遍历 a ~ g 段数码管
        for (i = 0; i < 7; i++) {
            // 检查该段是否需要点亮 (segment_code的对应bit是否为1)
            if (segment_code & (0x01 << i)) {
                aip3368h_engine_speed_panel_display_buff[gear_map[i].buff_index] |=
                    (0x01 << gear_map[i].bit_offset);
            }
        }
    }
}

/**
 * @brief 电池电量 8 字样对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_battery_8_symbol_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[3] |= (0x01 << 6);
    } else {
        aip3368h_engine_speed_panel_display_buff[3] &= ~(0x01 << 6);
    }
}

/**
 * @brief 电池图标对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_battery_icon_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[3] |= (0x01 << 7);
    } else {
        aip3368h_engine_speed_panel_display_buff[3] &= ~(0x01 << 7);
    }
}

/**
 * @brief 电池16字样对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_battery_16_symbol_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[3] |= (0x01 << 8);
    } else {
        aip3368h_engine_speed_panel_display_buff[3] &= ~(0x01 << 8);
    }
}

/**
 * @brief 电池电量对应的指示灯
 *
 * @param idx 0 ~ 8，第 idx 个指示灯（从左往右，从0开始）
 *
 * @param is_display
 *
 */
void __aip3368h_display_bat_lev_light__(u8 idx, u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[bat_lev_map[idx].buff_index] |=
            (0x01 << bat_lev_map[idx].bit_offset);
    } else {
        aip3368h_engine_speed_panel_display_buff[bat_lev_map[idx].buff_index] &=
            ~(0x01 << bat_lev_map[idx].bit_offset);
    }
}

/**
 * @brief 根据传参，显示对应的电池电量等级（不包括低电量提示）
 *
 * @param bat_lev 1 ~ 8，第 idx 个指示灯（从左往右，从0开始）
 *              0：清空显示
 *              1：显示第 0 个指示灯
 *              2：显示第 1 个指示灯
 *
 */
void aip3368h_display_bat_lev_light(u8 bat_lev)
{
    u8 i;
    // 遍历整个电池电量等级对应的指示灯
    for (i = 0; i < ARRAY_SIZE(bat_lev_map); i++) {
        // 如果当前传参的值减去1，比当前遍历的值还要小，清空对应的显示
        if (bat_lev <= i) {
            __aip3368h_display_bat_lev_light__(i, 0);
        } else if (bat_lev >= (i + 1)) {
            // 例如，如果要点亮第 0 个指示灯，bat_lev至少要大于等于1
            __aip3368h_display_bat_lev_light__(i, 1);
        }
    }
}

/**
 * @brief 显示小时的指定位
 *
 * @param bit_x 小时的指定位，0：个位，1：十位
 *
 * @param num 要显示的数字 0 ~ 9
 *
 */
void __aip3368h_display_hour_digit__(u8 bit_x, u8 num)
{
    u8 i;
    u8 segment_code;
    // 获取该数字对应的7段码 (要显示的数字 --> 七段码)
    segment_code = digit_segment_code[num];

    if (0 == bit_x) {
        // 第 0 位(个位)
        // 遍历 a ~ g 段数码管
        for (i = 0; i < 7; i++) {
            // 检查该段是否需要点亮 ( segment_code 的对应 bit 是否为1)
            if (segment_code & (0x01 << i)) {
                aip3368h_engine_speed_panel_display_buff[hour_bit_0_map[i]
                                                             .buff_index] |=
                    (0x01 << hour_bit_0_map[i].bit_offset);
            } else {
                aip3368h_engine_speed_panel_display_buff[hour_bit_0_map[i]
                                                             .buff_index] &=
                    ~(0x01 << hour_bit_0_map[i].bit_offset);
            }
        }
    } else {
        // 第 1 位（十位）
        // 遍历 a ~ g 段数码管
        for (i = 0; i < 7; i++) {
            // 检查该段是否需要点亮 ( segment_code 的对应 bit 是否为1)
            if (segment_code & (0x01 << i)) {
                aip3368h_engine_speed_panel_display_buff[hour_bit_1_map[i]
                                                             .buff_index] |=
                    (0x01 << hour_bit_1_map[i].bit_offset);
            } else {
                aip3368h_engine_speed_panel_display_buff[hour_bit_1_map[i]
                                                             .buff_index] &=
                    ~(0x01 << hour_bit_1_map[i].bit_offset);
            }
        }
    }
}

/**
 * @brief 显示分钟的指定位
 *
 * @param bit_x 分钟的指定位，0：个位，1：十位
 *
 * @param num 要显示的数字 0 ~ 9
 *
 */
void __aip3368h_display_minute_digit__(u8 bit_x, u8 num)
{
    u8 i;
    u8 segment_code;
    // 获取该数字对应的7段码 (要显示的数字 --> 七段码)
    segment_code = digit_segment_code[num];

    if (0 == bit_x) {
        // 第 0 位(个位)
        // 遍历 a ~ g 段数码管
        for (i = 0; i < 7; i++) {
            // 检查该段是否需要点亮 ( segment_code 的对应 bit 是否为1)
            if (segment_code & (0x01 << i)) {
                aip3368h_engine_speed_panel_display_buff[minute_bit_0_map[i]
                                                             .buff_index] |=
                    (0x01 << minute_bit_0_map[i].bit_offset);
            } else {
                aip3368h_engine_speed_panel_display_buff[minute_bit_0_map[i]
                                                             .buff_index] &=
                    ~(0x01 << minute_bit_0_map[i].bit_offset);
            }
        }
    } else {
        // 第 1 位（十位）
        // 遍历 a ~ g 段数码管
        for (i = 0; i < 7; i++) {
            // 检查该段是否需要点亮 ( segment_code 的对应 bit 是否为1)
            if (segment_code & (0x01 << i)) {
                aip3368h_engine_speed_panel_display_buff[minute_bit_1_map[i]
                                                             .buff_index] |=
                    (0x01 << minute_bit_1_map[i].bit_offset);
            } else {
                aip3368h_engine_speed_panel_display_buff[minute_bit_1_map[i]
                                                             .buff_index] &=
                    ~(0x01 << minute_bit_1_map[i].bit_offset);
            }
        }
    }
}

/**
 * @brief 时间中间的冒号对应的指示灯（时间分隔符）
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_time_colon_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[7] |= (0x01 << 15);
        aip3368h_engine_speed_panel_display_buff[6] |= (0x01 << 0);
    } else {
        aip3368h_engine_speed_panel_display_buff[7] &= ~(0x01 << 15);
        aip3368h_engine_speed_panel_display_buff[6] &= ~(0x01 << 0);
    }
}

/**
 * @brief 显示时间
 *
 * @param hour 小时
 *
 * @param min 分钟
 *
 */
void aip3368h_display_time(u8 hour, u8 min)
{
    u8 i;

    // 遍历 a ~ g 段数码管，清空显示
    for (i = 0; i < 7; i++) {
        aip3368h_engine_speed_panel_display_buff[hour_bit_0_map[i].buff_index] &=
            ~(0x01 << hour_bit_0_map[i].bit_offset);
        aip3368h_engine_speed_panel_display_buff[hour_bit_1_map[i].buff_index] &=
            ~(0x01 << hour_bit_1_map[i].bit_offset);
        aip3368h_engine_speed_panel_display_buff[minute_bit_0_map[i].buff_index] &=
            ~(0x01 << minute_bit_0_map[i].bit_offset);
        aip3368h_engine_speed_panel_display_buff[minute_bit_1_map[i].buff_index] &=
            ~(0x01 << minute_bit_1_map[i].bit_offset);
    }

    __aip3368h_display_hour_digit__(0, hour % 10);
    __aip3368h_display_hour_digit__(1, hour / 10);
    __aip3368h_display_minute_digit__(0, min % 10);
    __aip3368h_display_minute_digit__(1, min / 10);
}

/**
 * @brief 在设置时间时，显示当前设置的时间的指定位
 * 
 * @param is_display 是否显示
 * 
 * @param num 要显示的数字 
 *          如果在设置分钟，范围：0 ~ 59
 *          如果在设置小时，范围：0 ~ 23
 * 
 * @param is_setting_min 当前是否在设置分钟
 * 
 */
void aip3368h_display_time_digits_when_setting(u8 is_display, u8 num,
                                               u8 is_setting_min)
{
    u8 i;

    if (is_display) {
        if (is_setting_min) {
            __aip3368h_display_minute_digit__(0, num % 10);
            __aip3368h_display_minute_digit__(1, num / 10);
        } else {
            __aip3368h_display_hour_digit__(0, num % 10);
            __aip3368h_display_hour_digit__(1, num / 10);
        }
    } else {
        // 如果需要清空显示

        if (is_setting_min) {
            // 遍历 a ~ g 段数码管，清空显示
            for (i = 0; i < 7; i++) {
                aip3368h_engine_speed_panel_display_buff[minute_bit_0_map[i]
                                                             .buff_index] &=
                    ~(0x01 << minute_bit_0_map[i].bit_offset);
                aip3368h_engine_speed_panel_display_buff[minute_bit_1_map[i]
                                                             .buff_index] &=
                    ~(0x01 << minute_bit_1_map[i].bit_offset);
            }
        } else {
            // 遍历 a ~ g 段数码管，清空显示
            for (i = 0; i < 7; i++) {
                aip3368h_engine_speed_panel_display_buff[hour_bit_0_map[i]
                                                             .buff_index] &=
                    ~(0x01 << hour_bit_0_map[i].bit_offset);
                aip3368h_engine_speed_panel_display_buff[hour_bit_1_map[i]
                                                             .buff_index] &=
                    ~(0x01 << hour_bit_1_map[i].bit_offset);
            }
        }
    }
}

#if AIP3368H_DISPLAY_TEST_ENABLE

void aip3368h_display_engine_speed_lev_test_1ms_isr(void)
{
    static u8 lev = 0;
    static u8 dir = 0;  // 控制递增、递减
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 200) {
        return;
    } else {
        cnt = 0;
    }

    aip3368h_display_engine_speed_lev(lev);

    if (0 == dir) {
        lev++;
    } else {
        lev--;
    }

    if ((lev >= ARRAY_SIZE(engine_speed_gear_map)) || (lev == 0)) {
        dir = !dir;
    }
}

void aip3368h_display_gear_test_1ms_isr(void)
{
    static u8 gear = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    aip3368h_display_gear(gear);

    // 实际最大只有6挡,这里的9只是为了测试
    if (gear < 9) {
        gear++;
    } else if (gear == 9) {
        gear = GEAR_UNKNOWN; // 表示没有接挡位
    } else {
        gear = 0;
    }
}

void aip3368h_display_bat_lev_light_test_1ms_isr(void)
{
    static u8 is_display = 0;
    static u8 idx = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    aip3368h_display_bat_lev_light(idx);
    aip3368h_display_battery_8_symbol_light(is_display);
    aip3368h_display_battery_icon_light(is_display);
    aip3368h_display_battery_16_symbol_light(is_display);

    is_display = !is_display;
    idx++;
    if (idx >= ARRAY_SIZE(bat_lev_map) + 1) {
        // 0: 清空显示,ARRAY_SIZE(bat_lev_map) + 1:显示第 ARRAY_SIZE(bat_lev_map) 个指示灯
        idx = 0;
    }
}


void aip3368h_display_time_test_1ms_isr(void)
{
    static u8 hour = 0;
    static u8 min = 0;
    static u8 is_display = 0; // 控制时间分隔符是否显示
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    aip3368h_display_time(hour, min); 
    aip3368h_display_time_colon_light(is_display);
    is_display = !is_display;

    hour++;
    min++;
    if (hour >= 24) {
        hour = 0;
    }

    if (min >= 60) {
        min = 0;
    }
}

void aip3368h_display_test(void)
{
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 0; // 发动机转速面板边框，第 3 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 1; // 发动机转速面板边框，第 2 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 2; // 发动机转速面板边框，第 1 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 3; // 发动机转速面板边框，第 0 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 4; // 发动机转速面板边框，第 23 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 5; // 发动机转速面板边框，第 22 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 6; // 发动机转速面板边框，第 21 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 7; // 发动机转速面板边框，第 20 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 8; // 发动机转速面板边框，第 19 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 9; // 发动机转速面板边框，第 18 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 10;// 发动机转速面板边框，第 17 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 11; // 发动机转速面板边框，第 16 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 12; // 发动机转速面板边框，第 15 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 13; // 发动机转速面板边框，第 14 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 14; // 发动机转速面板边框，第 13 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 15; // 发动机转速，第 25 格指示灯，红色(从0开始排序)

    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 0; // 发动机转速，第 4 格指示灯(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 1; // 发动机转速，第 3 格指示灯(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 2; // 发动机转速，第 2 格指示灯(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 3; // 发动机转速，第 1 格指示灯(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 4; // 发动机转速，第 0 格指示灯(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 5; // x1000rpm 字样，第 0 个指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 6; // x1000rpm 字样，第 1 个指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 7; // 发动机转速面板边框，第 12 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 8; // 发动机转速面板边框，第 11 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 9; // 发动机转速面板边框，第 10 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 10; // 发动机转速面板边框，第 9 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 11; // 发动机转速面板边框，第 8 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 12; // 发动机转速面板边框，第 7 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 13; // 发动机转速面板边框，第 6 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 14; // 发动机转速面板边框，第 5 个指示灯(正上方为第0个，按顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 15; // 发动机转速面板边框，第 4 个指示灯(正上方为第0个，按顺时针排序)

    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 0;  // 发动机转速，第 20 格指示灯，红色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 1;// 发动机转速，第 19 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 2;  // 发动机转速，第 18 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 3;  // 发动机转速，第 17 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 4;  // 发动机转速，第 16 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 5;  // 发动机转速，第 15 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 6;  // 发动机转速，第 14 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 7; // 发动机转速，第 13 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 8; // 发动机转速，第 12 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 9; // 发动机转速，第 11 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 10; // 发动机转速，第 10 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 11; // 发动机转速，第 9 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 12; // 发动机转速，第 8 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 13; // 发动机转速，第 7 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 14; // 发动机转速，第 6 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 15; // 发动机转速，第 5 格指示灯，绿色(从0开始排序)

    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 0; // 发动机转速， 刻度 5 指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 1;// 发动机转速， 刻度 4 指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 2; // 发动机转速， 刻度 3 指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 3;// 发动机转速， 刻度 2 指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 4; // 发动机转速， 刻度 1 指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 5; // 发动机转速， 刻度 0 指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 6;// 电池电压， 8 字样对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 7; // 电池图标对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 8; // 电池电压， 16 字样对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 9;  // 发动机转速，第 28 格指示灯，红色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 10; // 发动机转速，第 27 格指示灯，红色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 11; // 发动机转速，第 26 格指示灯，红色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 12; // 发动机转速，第 24 格指示灯，红色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 13;// 发动机转速，第 23 格指示灯，红色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 14;  // 发动机转速，第 22 格指示灯，红色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 15; // 发动机转速，第 21 格指示灯，红色(从0开始排序)

    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 0; // 电池第 0 格指示灯，红色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 1; // 电池第 1 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 2; // 电池第 2 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 3;// 电池第 3 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 4; // 电池第 4 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 5; // 电池第 5 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 6; // 电池第 6 格指示灯，绿色(从0开始排序)
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 7; // 发动机转速，刻度 14 指示灯，红色
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 8; // 发动机转速，刻度 13 指示灯，红色
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 9; // 发动机转速，刻度 12 指示灯，红色
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 10; // 发动机转速，刻度 11 指示灯，红色
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 11; // 发动机转速，刻度 10 指示灯，红色
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 12; // 发动机转速，刻度 9 指示灯，白色
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 13; // 发动机转速，刻度 8 指示灯，白色
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 14; // 发动机转速，刻度 7 指示灯，白色
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 15; // 发动机转速，刻度 6 指示灯，白色

    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 0; // 挡位数码管， A 段
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 1; // 挡位数码管， B 段
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 2; // 挡位数码管， C 段
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 3; // 挡位数码管， D 段
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 4; // 挡位数码管， E 段
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 5; // 挡位数码管， F 段
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 6; // 挡位数码管， G 段
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 7; // 挡位边框，第 0 个指示灯(左上角为第0个，顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 8; // 挡位边框，第 1 个指示灯(左上角为第0个，顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 9; // 挡位边框，第 2 个指示灯(左上角为第0个，顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 10; // 挡位边框，第 3 个指示灯(左上角为第0个，顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 11; // 挡位边框，第 4 个指示灯(左上角为第0个，顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 12; // 挡位边框，第 5 个指示灯(左上角为第0个，顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 13; // 挡位边框，第 6 个指示灯(左上角为第0个，顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 14; // 挡位边框，第 7 个指示灯(左上角为第0个，顺时针排序)
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 15; // 手机图标对应的指示灯，白色

    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 0; // 时间分隔符，第 1 个指示灯
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 1; // 分钟十位数码管, A 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 2; // 分钟十位数码管, B 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 3; // 分钟十位数码管, C 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 4; // 分钟十位数码管, D 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 5; // 分钟十位数码管, E 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 6; // 分钟十位数码管, F 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 7; // 分钟十位数码管, G 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 8; // 分钟个位数码管, A 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 9; // 分钟个位数码管, B 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 10; // 分钟个位数码管, C 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 11;      // 分钟个位数码管, D 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 12; // 分钟个位数码管, E 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 13; // 分钟个位数码管, F 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 14; // 分钟个位数码管, G 段
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 15; // 大灯(远光灯)指示灯

    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 0; // COLOCK 字样指示灯
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 1; // 小时十位数码管, A 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 2; // 小时十位数码管, B 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 3; // 小时十位数码管, C 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 4; // 小时十位数码管, D 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 5; // 小时十位数码管, E 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 6; // 小时十位数码管, F 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 7; // 小时十位数码管, G 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 8; // 小时个位数码管, A 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 9; // 小时个位数码管, B 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 10; // 小时个位数码管, C 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 11;      // 小时个位数码管, D 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 12; // 小时个位数码管, E 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 13; // 小时个位数码管, F 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 14; // 小时个位数码管, G 段
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 15; // 时间分隔符, 第 0 个指示灯

    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 0; // 时速面板边框，第 1 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 1; // 时速面板边框，第 0 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 2; // 时速面板边框，第 23 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 3;// 时速面板边框，第 22 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 4;// 时速面板边框，第 21 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 5; // TRIP 字样，第 0 个指示灯
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 6;// TRIP 字样，第 1 个指示灯
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 7;// 时速面板边框，第 20 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 8;// 时速面板边框，第 19 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 9;// 时速面板边框，第 18 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 10;// 时速面板边框，第 17 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 11;// 时速面板边框，第 16 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 12;// 时速面板边框，第 15 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 13; // 时速面板边框，第 14 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 14;// 时速面板边框，第 13 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 15; // 油量边框，第 1 个指示灯(从面板左下方第 0 个开始，顺时针排序)

    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 0; // 左侧油量格数，第 3 格指示灯(从第 0 格开始)
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 1;// 右侧油量格数，第 3 格指示灯(从第 0 格开始)
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 2;// 右侧油量格数，第 4 格指示灯(从第 0 格开始)
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 3;// 左侧油量格数，第 4 格指示灯(从第 0 格开始)
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 4;// 油量图标对应的指示灯
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 5;// 时速面板边框，第 12 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 6;// 时速面板边框，第 11 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 7;// 时速面板边框，第 10 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 8;// 时速面板边框，第 9 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 9;// 时速面板边框，第 8 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 10;// 时速面板边框，第 7 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 11;// 时速面板边框，第 6 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 12;// 时速面板边框，第 5 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 13;// 时速面板边框，第 4 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 14;// 时速面板边框，第 3 个指示灯（正面最上边是第 0 个，按顺时针排序）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 15;// 时速面板边框，第 2 个指示灯（正面最上边是第 0 个，按顺时针排序）

    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 0; // ABS图标对应的指示灯
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 1; // 发动机故障对应的指示灯
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 2; // 油量边框，第 2 个指示灯(从面板左下方第 0 个开始，顺时针排序)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 3;// 油量边框，第 3 个指示灯(从面板左下方第 0 个开始，顺时针排序)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 4;// 油量边框，第 4 个指示灯(从面板左下方第 0 个开始，顺时针排序)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 5;// 油量边框，第 5 个指示灯(从面板左下方第 0 个开始，顺时针排序)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 6;// 油量边框，第 6 个指示灯(从面板左下方第 0 个开始，顺时针排序)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 7;// 油量边框，第 7 个指示灯(从面板左下方第 0 个开始，顺时针排序)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 8;// 油量边框，第 8 个指示灯(从面板左下方第 0 个开始，顺时针排序)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 9;// 油量边框，第 9 个指示灯(从面板左下方第 0 个开始，顺时针排序)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 10;// 右侧油量格数，第 0 格指示灯(从第 0 格开始)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 11;// 左侧油量格数，第 0 格指示灯(从第 0 格开始)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 12;// 左侧油量格数，第 1 格指示灯(从第 0 格开始)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 13;// 右侧油量格数，第 1 格指示灯(从第 0 格开始)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 14;// 右侧油量格数，第 2 格指示灯(从第 0 格开始)
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 15;// 左侧油量格数，第 2 格指示灯(从第 0 格开始)

    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 0; // 时速，第 1 个数码管， E 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 1;// 时速，第 1 个数码管， F 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 2;// 时速，第 1 个数码管， G 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 3;// 时速，第 2 个数码管， A 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 4;// 时速，第 2 个数码管， B 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 5;// 时速，第 2 个数码管， C 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 6;// 时速，第 2 个数码管， D 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 7;// 时速，第 2 个数码管， E 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 8;// 时速，第 2 个数码管， F 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 9;// 时速，第 2 个数码管， G 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 10; // 右转向指示灯
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 11; // mph 字样对应的指示灯
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 12; // km/h 字样第 0 个指示灯
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 13;// km/h 字样第 1 个指示灯
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 14; // 温度过热图标对应的指示灯
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 15; // 机油图标对应的指示灯

    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 0; // 里程 第 5 个数码管， B 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 1;// 里程 第 5 个数码管， C 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 2;// 里程 第 5 个数码管， D 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 3;// 里程 第 5 个数码管， E 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 4;// 里程 第 5 个数码管， F 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 5;// 里程 第 5 个数码管， G 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 6; // mile 字样对应的指示灯
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 7; // km 字样对应的指示灯
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 8; // 左转向指示灯
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 9; // N 字样对应的指示灯
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 10; // 时速 第 0 个数码管， 第 0 个指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 11;// 时速 第 0 个数码管， 第 1 个指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 12;// 时速 第 1 个数码管， A 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 13;// 时速 第 1 个数码管， B 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 14;// 时速 第 1 个数码管， C 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 15;// 时速 第 1 个数码管， D 段指示灯(数码管从左边第0个开始排序)

    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 0; // 里程 第 2 个数码管， A 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 1;// 里程 第 2 个数码管， B 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 2;// 里程 第 2 个数码管， C 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 3;// 里程 第 2 个数码管， D 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 4;// 里程 第 2 个数码管， E 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 5;// 里程 第 2 个数码管， F 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 6;// 里程 第 2 个数码管， G 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 7;// 里程 第 3 个数码管， A 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 8;// 里程 第 3 个数码管， B 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 9;// 里程 第 3 个数码管， C 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 10;// 里程 第 3 个数码管， D 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 11;// 里程 第 3 个数码管， E 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 12;// 里程 第 3 个数码管， F 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 13;// 里程 第 3 个数码管， G 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 14; // 里程 小数点对应的指示灯
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 15; // 里程 第 4 个数码管， A 段指示灯(数码管从左边第0个开始排序)

    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 0; // ODO 字样对应的指示灯
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 1; // 油量边框，第 0 个指示灯(从面板左下方第 0 个开始，顺时针排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 2; // 里程 第 0 个数码管， A 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 3; // 里程 第 0 个数码管， B 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 4; // 里程 第 0 个数码管， C 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 5; // 里程 第 0 个数码管， D 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 6; // 里程 第 0 个数码管， E 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 7; // 里程 第 0 个数码管， F 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 8; // 里程 第 0 个数码管， G 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 9; // 里程 第 1 个数码管， A 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 10; // 里程 第 1 个数码管， B 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 11; // 里程 第 1 个数码管， C 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 12;// 里程 第 1 个数码管， D 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 13;// 里程 第 1 个数码管， E 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 14; // 里程 第 1 个数码管， F 段指示灯(数码管从左边第0个开始排序)
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 15; // 里程 第 1 个数码管， G 段指示灯(数码管从左边第0个开始排序)
}
#endif
