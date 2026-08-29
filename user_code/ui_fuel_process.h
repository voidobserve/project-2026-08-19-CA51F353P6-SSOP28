#ifndef __UI_FUEL_PROCESS_H__
#define __UI_FUEL_PROCESS_H__

#include "user_typedef.h"

// 油量挡位(包括低油量提示的挡位)
#define FUEL_LEVEL_MAX (6)

/*
	将油量挡位与油量百分比建立映射关系

	0 ~ 16,包括 16 , 低油量提示
	17 ~ 33,包括 33 , 1 格油量
	34 ~ 50,包括 50 , 2 格油量
	51 ~ 66,包括 66 , 3 格油量
	67 ~ 83,包括 83 , 4 格油量
	84 ~ 100,包括 100 , 5 格油量
*/ 
enum
{
    FUEL_LEV_0 = 100 / FUEL_LEVEL_MAX,     // 低油量,或者是没有接油量
    FUEL_LEV_1 = 100 * 2 / FUEL_LEVEL_MAX, // 至少有 1 格油量
    FUEL_LEV_2 = 100 * 3 / FUEL_LEVEL_MAX,
    FUEL_LEV_3 = 100 * 4 / FUEL_LEVEL_MAX,
    FUEL_LEV_4 = 100 * 5 / FUEL_LEVEL_MAX,
    FUEL_LEV_5 = 100 * 6 / FUEL_LEVEL_MAX,
};
 
// 显示时，更新油量的时间，单位：ms
#define FUEL_UPDATE_TIME ((u16)5000)
// 显示时，如果油量格数很接近，需要延长油量更新时间
#define FUEL_UPDATE_TIME_EXTEND ((u16)20000)
 
void ui_fuel_process_cnt_add(void);
void ui_fuel_process(void);

#endif