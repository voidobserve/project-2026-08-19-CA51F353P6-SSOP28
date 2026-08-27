#ifndef __PIN_LEVEL_SCAN_H__
#define __PIN_LEVEL_SCAN_H__
 
#include "user_config.h"

#if PIN_LEVEL_SCAN_ENABLE
 
#define PIN_DETECT_ABS (P14) // ABS 检测脚
#define PIN_DETECT_ENGINE_ERR (P15) // 发动机故障 检测脚


// 定义信号有效时，检测脚对应的电平
  
#define SIGNAL_VALID_LEV_OF_ABS (0) 
#define SIGNAL_VALID_LEV_OF_ENGINE_ERR (0) 

void pin_level_scan_init(void); // 扫描引脚的配置（初始化）
void pin_level_handle(void);        // 扫描引脚的电平
#endif

#endif
