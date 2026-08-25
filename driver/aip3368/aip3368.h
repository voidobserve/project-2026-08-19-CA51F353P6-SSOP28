#ifndef __AIP3368_H__
#define __AIP3368_H__

#include "user_typedef.h"

/*
	时速面板对应的芯片数量
	有多少个芯片级联
*/
#define AIP3368H_SPEED_PANEL_IC_NUM (7)
/*
	发动机转速面板对应的芯片数量
	有多少个芯片级联
*/
#define AIP3368H_ENGINE_SPEED_PANEL_IC_NUM (8)

#if (AIP3368H_SPEED_PANEL_IC_NUM >= AIP3368H_ENGINE_SPEED_PANEL_IC_NUM)
#define AIP3368_MAX_IC_NUM AIP3368H_SPEED_PANEL_IC_NUM
#else
#define AIP3368_MAX_IC_NUM AIP3368H_ENGINE_SPEED_PANEL_IC_NUM
#endif

/*
	串行数据输入端
	时速面板对应的芯片级联的DIO
*/
#define DIO_GROUP_1 P05
/*
	串行数据输入端
	发动机转速面板对应的芯片级联的DIO
*/
#define DIO_GROUP_2 P34 // 串行数据输入端
#define DCK         P06 // 串行时钟信号的输入端
#define LAT         P07 // 数据锁存
#define PDM         -1  // 输出使能控制端口（未使用）

extern volatile u16
    aip3368h_speed_panel_display_buff[AIP3368H_SPEED_PANEL_IC_NUM];
extern volatile u16
    aip3368h_engine_speed_panel_display_buff[AIP3368H_ENGINE_SPEED_PANEL_IC_NUM];

extern void Dly_us(unsigned int n);
// 延时函数，根据需要决定使用，测试48Mhz主频无定时中断不需要延时也能正常点亮
#define aip3368h_delay()
// #define aip3368h_delay() Delay_50us(1)

void aip3368h_refresh_time_add();

void aip3368h_module_init(void);
void aip3368h_module_display(void);

#endif