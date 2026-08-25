#include <string.h> // memset()

#include "user_config.h"
#include "instrument.h"
// #include "user_flash.h"

volatile instrument_t instrument = {0};
static volatile u8 is_intrument_save_enable = 0; // 是否允许保存数据到 flash
// 延时写入flash的时间计数
static volatile u16 instrument_info_save_time_cnt = 0;

void instrument_info_init(void)
{
    save_info_t save_info;
    // 从flash中读出数据
    flash_read(FLASH_START_ADDR, (u8 *)&save_info, sizeof(save_info_t));
    if (USER_FLASH_DATA_VALID_VAL == save_info.is_save_data_valid) {
        // 如果保存在flash中的数据有效
#if USER_DEBUG_ENABLE
        // printf("save data valid\n");
#endif

        memcpy(&instrument.save_info, &save_info, sizeof(save_info_t));
    } else {
        // 如果保存在flash中的数据无效，全局变量 instrument 中的元素 默认全部为0
#if USER_DEBUG_ENABLE
        // printf("save data invalid\n");
#endif

#if 0
        instrument.save_info.is_display_total_mileage = 1; // 默认显示大计里程
        // 表示存储的数据有效
        instrument.save_info.is_save_data_valid = USER_FLASH_DATA_VALID_VAL;
        // 默认使用公制单位
        instrument.save_info.distance_unit_type = DISTANCE_UNIT_TYPE_METRIC;
#endif

        instrument_info_save_enable(); // 将数据写回flash
    }
}

void instrument_info_save(void)
{
#if USE_INTERNAL_FLASH_SAVE_DATA

    instrument.save_info.is_save_data_valid =
        USER_FLASH_DATA_VALID_VAL; // 表示数据有效，让下一次上电读出数据时，验证该标志位
    // 先擦除扇区再写入
    flash_erase_sector(FLASH_START_ADDR);
    flash_program(FLASH_START_ADDR, (u8 *)&instrument.save_info,
                  sizeof(save_info_t));

#if USER_DEBUG_ENABLE
// printf("instrument info save\n");
#endif

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
