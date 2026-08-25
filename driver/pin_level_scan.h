#ifndef __PIN_LEVEL_SCAN_H__
#define __PIN_LEVEL_SCAN_H__
 
#include "user_config.h"

#if PIN_LEVEL_SCAN_ENABLE

#define PIN_DETECT_GEAR_N (P23) // N 档 检测脚
#define PIN_DETECT_GEAR_1 (P10) // 1 档 检测脚
#define PIN_DETECT_GEAR_2 (P21) // 2 档 检测脚
#define PIN_DETECT_GEAR_3 (P13) // 3 档 检测脚
#define PIN_DETECT_GEAR_4 (P20) // 4 档 检测脚
#define PIN_DETECT_GEAR_5 (P31) // 5 档 检测脚
#define PIN_DETECT_GEAR_6 (P17) // 6 档 检测脚

#define PIN_DETECT_BREAKDOWN (P22)  // 检测故障的引脚
#define PIN_DETECT_LEFT_TURN (P24)  // 左转向 检测脚
#define PIN_DETECT_RIGHT_TURN (P14) // 右转向 检测脚
#define PIN_DETECT_HIGH_BEAM (P30)  // 大灯（远光灯）检测脚

// 定义信号有效时，检测脚对应的电平
#define SIGNAL_VALID_LEV_OF_GEAR_N (0)
#define SIGNAL_VALID_LEV_OF_GEAR_1 (0)
#define SIGNAL_VALID_LEV_OF_GEAR_2 (0)
#define SIGNAL_VALID_LEV_OF_GEAR_3 (0)
#define SIGNAL_VALID_LEV_OF_GEAR_4 (0)
#define SIGNAL_VALID_LEV_OF_GEAR_5 (0)
#define SIGNAL_VALID_LEV_OF_GEAR_6 (0)

#define SIGNAL_VALID_LEV_OF_LEFT_TURN (1)
#define SIGNAL_VALID_LEV_OF_RIGHT_TURN (1)
#define SIGNAL_VALID_LEV_OF_HIGH_BEAM (1)
// 信号有效时，发动机故障检测脚对应的电平
#define SIGNAL_VALID_LEV_OF_BREAKDOWN (0)

void pin_level_scan_config(void); // 扫描引脚的配置（初始化）
void pin_level_scan(void);        // 扫描引脚的电平
#endif

#endif
