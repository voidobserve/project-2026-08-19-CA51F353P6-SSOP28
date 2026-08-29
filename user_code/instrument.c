#include <string.h> // memset()

#include "user_config.h"
#include "instrument.h"

#include "ca51f_config.h" // 包含芯片flash大小的定义
#include "flash.h"        // 芯片官方提供的flash接口
#include "ca51f3xsfr.h"   // PADRD 寄存器定义

#include "key_event_process.h"

#if USER_DEBUG_ENABLE
#include <stdio.h>
#endif

#define FLASH_SECTOR_0_ADDR (0)
#define FLASH_SECTOR_1_ADDR (128)

volatile instrument_t instrument = {0};
static volatile u8 is_intrument_save_enable = 0; // 是否允许保存数据到 flash
// 延时写入flash的时间计数
static volatile u16 instrument_info_save_time_cnt = 0;

// 使用两个区域保存数据，避免flash读取和写入丢失的情况
volatile save_info_t save_info_area_1 = {0};
volatile save_info_t save_info_area_2 = {0};

void instrument_info_init(void)
{
    u8 check_sum_1 = 0; // 存放第一个区域数据的校验和
    u8 check_sum_2 = 0; // 存放第二个区域数据的校验和
    /*
        存放写入flash的次数，用于验证出错时，
        找到校验和通过的、写入次数较大的区域来进行初始化
    */
    u32 write_cnt = 0;
    /*
        两个区域的数据不一致，又或者是校验和没有通过，但是至少写入了一次，
        用该变量判断是用哪个区域的数据来进行初始化
        0：无效，都不使用
        1：使用第一个区域的数据
        2：使用第二个区域的数据
    */
    u8 use_data_area_index_to_init = 0;

    // 划分FLASH最后256字节为数据空间
    // 当前芯片一个扇区为128字节
    PADRD = CHIP_FLASH_SIZE * 4 - 1;

    // 从flash中读出数据
    Data_Area_Mass_Read(FLASH_SECTOR_0_ADDR, (u8 *)&save_info_area_1,
                        sizeof(save_info_t));
    Data_Area_Mass_Read(FLASH_SECTOR_1_ADDR, (u8 *)&save_info_area_2,
                        sizeof(save_info_t));

    // 遍历存储在flash中的数据的元素个数，不包括校验和
    check_sum_1 += save_info_area_1.write_cnt;
    check_sum_1 += save_info_area_1.is_display_total_mileage;
    check_sum_1 += save_info_area_1.distance_unit_type;

    check_sum_2 += save_info_area_2.write_cnt;
    check_sum_2 += save_info_area_2.is_display_total_mileage;
    check_sum_2 += save_info_area_2.distance_unit_type;

    if ((memcmp(&save_info_area_1, &save_info_area_2, sizeof(save_info_t)) ==
         0) &&
        check_sum_1 == save_info_area_1.check_sum &&
        check_sum_2 == save_info_area_2.check_sum &&
        save_info_area_1.write_cnt != 0) {
        /*
            两个区域的数据相同，校验和验证通过，并且至少写入了一次，
            说明其中存放的数据有效
        */
#if USER_DEBUG_ENABLE
        printf("read save data pass\n");
#endif

        instrument.distance_unit_type = save_info_area_1.distance_unit_type;
        instrument.is_display_total_mileage =
            save_info_area_1.is_display_total_mileage;
    } else {
        // 两个区域的数据不一致，或者是没有写入过数据，又或者是校验和没有通过
#if USER_DEBUG_ENABLE
        printf("read save data err\n");
#endif
        // 找到校验和通过的，并且写入次数较大的区域，作为有效数据
        if (check_sum_1 == save_info_area_1.check_sum) {
            if (0 != save_info_area_1.write_cnt) {
                write_cnt = save_info_area_1.write_cnt;
                use_data_area_index_to_init = 1;
            }
        }

        if (check_sum_2 == save_info_area_2.check_sum) {
            if (0 != save_info_area_2.write_cnt &&
                save_info_area_2.write_cnt > write_cnt) {
                write_cnt = save_info_area_2.write_cnt;
                use_data_area_index_to_init = 2;
            }
        }

        switch (use_data_area_index_to_init) {
        case 1:
            instrument.distance_unit_type = save_info_area_1.distance_unit_type;
            instrument.is_display_total_mileage =
                save_info_area_1.is_display_total_mileage;
            memcpy(&save_info_area_2, &save_info_area_1, sizeof(save_info_t));
            break;
        case 2:
            instrument.distance_unit_type = save_info_area_2.distance_unit_type;
            instrument.is_display_total_mileage =
                save_info_area_2.is_display_total_mileage;
            memcpy(&save_info_area_1, &save_info_area_2, sizeof(save_info_t));
            break;
        default:
            // 存放数据的区域都无效，初始化为默认值

            instrument.distance_unit_type = DISTANCE_UNIT_TYPE_METRIC;
            instrument.is_display_total_mileage = 1;

            save_info_area_1.distance_unit_type = instrument.distance_unit_type;
            save_info_area_1.is_display_total_mileage =
                instrument.is_display_total_mileage;
            save_info_area_1.write_cnt = 1;

            check_sum_1 = 0;
            check_sum_1 += save_info_area_1.write_cnt;
            check_sum_1 += save_info_area_1.is_display_total_mileage;
            check_sum_1 += save_info_area_1.distance_unit_type;
            memcpy(&save_info_area_2, &save_info_area_1, sizeof(save_info_t));
            break;
        }

        // 初始化完成后，将数据写回flash
        instrument_info_save_enable();
    }

    // 每次上电，初始化非掉电保存的数据
    instrument.key_event = KEY_EVENT_NONE;
    instrument.gear = GEAR_UNKNOWN;

    // TEST ONLY
    // instrument.distance_unit_type = DISTANCE_UNIT_TYPE_IMPERIAL;
}

void instrument_info_save(void)
{
    u8 check_sum = 0;

    save_info_area_1.write_cnt++;
    save_info_area_1.distance_unit_type = instrument.distance_unit_type;
    save_info_area_1.is_display_total_mileage =
        instrument.is_display_total_mileage;

    check_sum += save_info_area_1.write_cnt;
    check_sum += save_info_area_1.is_display_total_mileage;
    check_sum += save_info_area_1.distance_unit_type;
    save_info_area_1.check_sum = check_sum;

    memcpy(&save_info_area_2, &save_info_area_1, sizeof(save_info_t));

    Data_Area_Sector_Erase(0);
    Data_Area_Mass_Write(FLASH_SECTOR_0_ADDR, (u8 *)&save_info_area_1,
                         sizeof(save_info_t));
    Data_Area_Sector_Erase(1);
    Data_Area_Mass_Write(FLASH_SECTOR_1_ADDR, (u8 *)&save_info_area_2,
                         sizeof(save_info_t));

#if USER_DEBUG_ENABLE
    printf("instrument_info_save\n");
#endif
}

// 使能延时写入flash的操作
void instrument_info_save_enable(void)
{
    instrument_info_save_time_cnt = 0;
    is_intrument_save_enable = 1;
}

// 递增延时写入flash的时间
void instrument_info_save_time_add(void)
{
    if (is_intrument_save_enable) {
        instrument_info_save_time_cnt++;
    }
}

void instrument_info_save_handle(void)
{
    if (is_intrument_save_enable &&
        instrument_info_save_time_cnt >= INSTRUMENT_INFO_SAVE_TIME) {
        is_intrument_save_enable = 0;
        instrument_info_save_time_cnt = 0;
        instrument_info_save();
    }
}
