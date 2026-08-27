// encoding = utf-8
#ifndef _FLASH_C_
#define _FLASH_C_
/*********************************************************************************************************************/
#include "ca51f_config.h"
#include "includes\ca51f3sfr.h"
#include "includes\ca51f3xsfr.h"
#include "includes\gpiodef_f3.h"

#include "includes\system.h"
#include "Library\includes\flash.h"
#include <intrins.h>
/*********************************************************************************************************************/

/***********************************************************************************
函 数 名：Data_Area_Erase_Sector										 	
功能描述：擦除数据区空间的一个扇区						
函数说明：								
输    入：unsigned char SectorNumber	扇区号									
返    回：无																	
***********************************************************************************/
void Data_Area_Sector_Erase(unsigned char SectorNumber)
{
	unsigned int SectorAddress;
	FSCMD 	= 	0;
	SectorAddress = 0x80*SectorNumber;
	LOCK  = CMD_DATA_AREA_UNLOCK;						//数据区解锁
	PTSH = (unsigned char)(SectorAddress>>8);			//填写扇区地址
	PTSL = (unsigned char)(SectorAddress);				//填写扇区地址	
	FSCMD = CMD_DATA_AREA_ERASE_SECTOR;					//执行擦除扇区操作
	LOCK  = CMD_FLASH_LOCK;						    	//对FLASH加锁
} 

/***********************************************************************************
函 数 名：Data_Area_Write_Byte											 	
功能描述：向FLASH数据区写入一个字节数据																	
输    入：unsigned int Address	数据区空间写入地址											
					unsigned char Data		写入数据												
返    回：无																	
***********************************************************************************/
void Data_Area_Write_Byte(unsigned int Address,unsigned char Data)
{
	FSCMD 	= 	0;
	LOCK  = CMD_DATA_AREA_UNLOCK;						//数据区解锁
	PTSH = (unsigned char)(Address>>8);					//填写高位地址
	PTSL = (unsigned char)Address;        				//填写低位地址
	FSCMD = CMD_DATA_AREA_WIRTE;						//执行写操作
	FSDAT = Data;										//装载数据	
	FSCMD 	= 	0;
	LOCK  = CMD_FLASH_LOCK;								//对FLASH加锁
} 

/***********************************************************************************
函 数 名：Data_Area_Mass_Write										 	
功能描述：向FLASH数据区批量写入数据																	
输    入：unsigned int wAddress	  数据区空间写入起始地址											
					unsigned char *pData		数据指针，指向写入数据缓存数组
					unsigned char Length		写入数据长度												
返    回：无																	
***********************************************************************************/
void Data_Area_Mass_Write(unsigned int Address,unsigned char *pData,unsigned int Length)
{
	unsigned int i;
	FSCMD 	= 	0;
	LOCK  = CMD_DATA_AREA_UNLOCK;						//数据区解锁
	PTSH = (unsigned char)(Address>>8);					//填写高位地址
	PTSL = (unsigned char)Address;        				//填写低位地址
	FSCMD = CMD_DATA_AREA_WIRTE;						//执行写操作
	for(i = 0; i < Length; i++)
	{
		FSDAT = *pData++;								//装载数据	
	}
	FSCMD 	= 	0;
	LOCK  = CMD_FLASH_LOCK;						    	//对FLASH加锁
} 

/***********************************************************************************
函 数 名：Data_Area_Read_Byte											 	
功能描述：从FLASH数据区读出一字节数据																
输    入：unsigned int Address	数据区空间读地址																							
返    回：读取的一字节数据																	
***********************************************************************************/
unsigned char Data_Area_Read_Byte(unsigned int Address)
{	
	unsigned char Data_Temp;
	FSCMD 	= 	0;
	PTSH = (unsigned char)(Address>>8);					//填写高位地址
	PTSL = (unsigned char)Address;        				//填写低位地址
	FSCMD = CMD_DATA_AREA_READ;							//执行读操作
	Data_Temp = FSDAT;
	FSCMD 	= 	0;
	LOCK  = CMD_FLASH_LOCK;						    	//对FLASH加锁
	return Data_Temp;
} 

/***********************************************************************************
函 数 名：Data_Area_Mass_Read										 	
功能描述：从FLASH数据区批量读出数据																	
输    入：unsigned int Address	  数据区空间读起始地址											
					unsigned char *pData		数据指针，指向读出数据缓存数组
					unsigned char Length		读数据长度												
返    回：无																	
***********************************************************************************/
void Data_Area_Mass_Read(unsigned int Address,unsigned char *pData,unsigned int Length)
{
	unsigned int i;
	FSCMD 	= 	0;
	PTSH = (unsigned char)(Address>>8);					//填写高位地址
	PTSL = (unsigned char)Address;        				//填写低位地址
	FSCMD = CMD_DATA_AREA_READ;							//执行读操作
	for(i = 0; i < Length; i++)
	{												
		*pData++ = FSDAT;
	}
	FSCMD 	= 	0;
	LOCK  = CMD_FLASH_LOCK;						    	//对FLASH加锁
} 
#endif 