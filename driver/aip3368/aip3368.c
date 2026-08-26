#include "aip3368.h"

#include "gpiodef_f3.h" // GPIO_Init()
#include "ca51f3xsfr.h" // 含有引脚功能控制寄存器定义
#include "ca51f3sfr.h" // 含有引脚数据寄存器定义
#include <string.h> // memset
#include "user_typedef.h"
#include "user_config.h" // USER_DEBUG_ENABLE

// REVIEW 测试时使用 u16 , 实际使用可以改为 u8
static volatile u8 aip3368h_refresh_cnt = 0;
// static volatile u16 aip3368h_refresh_cnt = 0;

// 时速面板对应的显存
volatile u16 aip3368h_speed_panel_display_buff[AIP3368H_SPEED_PANEL_IC_NUM] = {
    0};

// 发动机转速面板对应的显存
volatile u16 aip3368h_engine_speed_panel_display_buff
    [AIP3368H_ENGINE_SPEED_PANEL_IC_NUM] = {0};

// 放在1ms的定时器中
void aip3368h_refresh_time_add(void)
{
    // 防止计数溢出
    if (aip3368h_refresh_cnt < ((u16)-1)) {
        aip3368h_refresh_cnt++;
    }
}

static void aip3368h_module_send_data_to_all_dev(const u16 *buff_group_1,
                                                 const u8 len_group_1,
                                                 const u16 *buff_group_2,
                                                 const u8 len_group_2)
{
    volatile u8 i;
    volatile u8 j;
    volatile u16 dat_group_1;
    volatile u16 dat_group_2;

    /*
        两组级联 共用 DCK 和 LAT，
        因此一帧的时钟数必须等于最大的级联数（AIP3368_MAX_IC_NUM = 8 个字）。

        移位寄存器型级联：先发的数据会填充到最远端的芯片，
        发多了的字会从级联链末端移出而丢失（不是“截取前一段再转发”）。
        所以级联数较少的一组，数据必须从帧的“末尾”开始发送：
        字偏移 offset = AIP3368_MAX_IC_NUM - 本组级联数。

        例如：时速面板 6 颗，发动机转速面板 8 颗，
        时速面板 offset = 8 - 6 = 2，其 6 个字占帧的第 2~7 个位置，
        恰好落在 6 颗芯片上，前面 2 个字补 0（被移出丢失，无影响）。
        发动机转速面板 offset = 0，8 个字正好占满整帧。
    */
    volatile u8 offset_group_1 = AIP3368_MAX_IC_NUM - len_group_1;
    volatile u8 offset_group_2 = AIP3368_MAX_IC_NUM - len_group_2;

    // 开始
    DCK = 0;
    LAT = 0;
    aip3368h_delay();

    // 一帧完整数据
    for (i = 0; i < AIP3368_MAX_IC_NUM; i++) {
        // 本组数据在帧中的位置：i < offset 的部分补 0（会被移出级联链）
        dat_group_1 = 0;
        dat_group_2 = 0;

        if (i >= offset_group_1) {
            dat_group_1 = buff_group_1[i - offset_group_1];
        }

        // if (i < len_group_1) {
        //     dat_group_1 = buff_group_1[i];
        // }

        if (i >= offset_group_2) {
            dat_group_2 = buff_group_2[i - offset_group_2];
        }

        // if (i < len_group_2) {
        //     dat_group_2 = buff_group_2[i];
        // }

        for (j = 0; j < 16; j++) {
            DIO_GROUP_1 = dat_group_1 & (u16)0x8000 ? 1 : 0;
            DIO_GROUP_2 = dat_group_2 & (u16)0x8000 ? 1 : 0;

            aip3368h_delay();
            DCK = 1;
            aip3368h_delay();

            dat_group_1 <<= 1;
            dat_group_2 <<= 1;

            DCK = 0;
            aip3368h_delay();
        }
    }

    // 结束
    LAT = 1;
    aip3368h_delay();
    LAT = 0;
    aip3368h_delay();
    DIO_GROUP_1 = 0;
    DIO_GROUP_2 = 0;
}

#define AIP3368H_FLASH_TEST_ENABLE 0

// 根据显存中的数据，更新显示
void aip3368h_module_display(void)
{
    // 刷新间隔 单位：ms
    if (aip3368h_refresh_cnt < 25) {
        // if (aip3368h_refresh_cnt < 500) {
        return;
    } else {
        aip3368h_refresh_cnt = 0;
    } 

#if USER_DEBUG_ENABLE
// printf("aip3368h_module_display\n");
#endif

#if AIP3368H_FLASH_TEST_ENABLE

    // printf("aip3368h_speed_panel_display_buff[0] == 0x%04x\n",
    //        aip3368h_speed_panel_display_buff[0]);

    // 闪烁测试
    if (aip3368h_speed_panel_display_buff[0] == (u16)0x0000) {
        memset(aip3368h_speed_panel_display_buff, 0xFF,
               sizeof(aip3368h_speed_panel_display_buff));
        memset(aip3368h_engine_speed_panel_display_buff, 0xFF,
               sizeof(aip3368h_engine_speed_panel_display_buff));
    } else {
        memset(aip3368h_speed_panel_display_buff, 0x00,
               sizeof(aip3368h_speed_panel_display_buff));
        memset(aip3368h_engine_speed_panel_display_buff, 0x00,
               sizeof(aip3368h_engine_speed_panel_display_buff));
    }

#endif

    aip3368h_module_send_data_to_all_dev(
        aip3368h_speed_panel_display_buff, AIP3368H_SPEED_PANEL_IC_NUM,
        aip3368h_engine_speed_panel_display_buff,
        AIP3368H_ENGINE_SPEED_PANEL_IC_NUM);
}

void aip3368h_module_init(void)
{
    // 显示驱动芯片有记忆功能（数据锁存），每次上电应该清空显存
    memset(aip3368h_speed_panel_display_buff, 0x00,
           sizeof(aip3368h_speed_panel_display_buff));
    memset(aip3368h_engine_speed_panel_display_buff, 0x00,
           sizeof(aip3368h_engine_speed_panel_display_buff));

    // DCK
    GPIO_Init(P06F, OUTPUT);
    // DIO
    GPIO_Init(P05F, OUTPUT);
    GPIO_Init(P34F, OUTPUT);
    // LAT
    GPIO_Init(P07F, OUTPUT);

    DIO_GROUP_1 = 0;
    DIO_GROUP_2 = 0;
    DCK = 0;
    LAT = 0;

    // 显示驱动芯片带有记忆功能，上电之后需要先写入一次全为0的数据，再写入实际数据
    aip3368h_module_send_data_to_all_dev(
        aip3368h_speed_panel_display_buff, AIP3368H_SPEED_PANEL_IC_NUM,
        aip3368h_engine_speed_panel_display_buff, AIP3368H_SPEED_PANEL_IC_NUM);
}
