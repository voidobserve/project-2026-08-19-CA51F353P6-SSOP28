#ifndef _LCD_C_
#define _LCD_C_
/*********************************************************************************************************************/
#include "ca51f_config.h"

#include "includes\ca51f3sfr.h"
#include "includes\ca51f3xsfr.h"
#include "includes\gpiodef_f3.h"
#include "includes\system.h"
#include "Library\includes\lcd.h"
#include <intrins.h>
/*********************************************************************************************************************/
/***********************************************************************************
函数名：LCD_init
功能说明：LCD寄存器初始化
输入参数： len		LCD时钟源选择
						dmod	驱动电流选择
						bias	偏压设置
						ldrv	驱动电压等级设置
						lxdiv LCD时钟分频系数
返回值：	无						
***********************************************************************************/
void LCD_init(LEN_TypeDef len,DMOD_TypeDef dmod,BIAS_TypeDef bias,LDRV_TypeDef ldrv,unsigned int lxdiv)
{
	LXDIVH = (unsigned char)(lxdiv>>8);
	LXDIVL = (unsigned char)(lxdiv);	
	LXCFG =	 DMOD(dmod) | BIAS(bias) | LDRV(ldrv);
	LXCON =  LEN(len) | LMOD(LMOD_lcd);	 
}

/***********************************************************************************
函数名：LCD_WriteLram
功能说明：写入数据到显示缓存
输入参数： 	laddr	显示缓存地址
						ldata	显示缓存数据
返回值：	无						
***********************************************************************************/
void LCD_WriteLram(unsigned char laddr, unsigned char ldata)
{
	INDEX = laddr;
	LXDAT = ldata;
}

/***********************************************************************************
函数名：LCD_ReadLram
功能说明：从显示缓存读取数据
输入参数： 	laddr		显示缓存地址
返回值：	显示缓存数据			
***********************************************************************************/
unsigned char LCD_ReadLram(unsigned char laddr)
{
	INDEX = laddr;
	return LXDAT;
}
/*********************************************************************************************************************/
#endif