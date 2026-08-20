
// =====================================================================
//                                                      SSOP28
//                                                   -------------
//                                            VDD  -|01         28|-  GND
//                                            P20  -|02         27|-  P30/Uart0_烧录TX
//     灵敏度电容       tk_cap          TK_CAP/P21  -|03         26|-  P31/Uart0_烧录RX
//                                   ADC0/TK0/P10  -|04         25|-  P32
//            TX            Uart1_TX/ADC1/TK1/P11  -|05         24|-  P33
//            RX            Uart1_RX/ADC2/TK2/P12  -|06         23|-  P00/TK19
//                                   ADC3/TK3/P13  -|07         22|-  P01/TK18
//       ABS  ABS_Chk(低有效)         ADC4/TK4/P14  -|08         21|-  P02/TK17         PD_Chk(低有效)发动机
//       雾灯  WD_Chk(高有效)    PWM0/ADC5/TK5/P15  -|09         20|-  P03/TK16
//       小灯  XD_Chk(低有效)    PWM1/ADC6/TK6/P16  -|10         19|-  P04/TK15
//                                   ADC7/TK7/P17  -|11         18|-  P05/TK14         AIP3368_DI_L
//                                        TK8/P37  -|12         17|-  P06/TK13/PWM5    AIP3368_DCK
//                                        TK9/P36  -|13         16|-  P07/TK12/PWM4    AIP3368_LAT
//                                  PWM2/TK10/P35  -|14         15|-  P34/TK11/PWM3    AIP3368_DI_R
//                                                   -------------

// #ifndef _MAIN_C_
// #define _MAIN_C_
/*********************************************************************************************************************/
#include "ca51f_config.h"
#include "includes\ca51f3sfr.h"
#include "includes\ca51f3xsfr.h"
#include "includes\gpiodef_f3.h"

#include "Library\includes\delay.h"
#include "Library\includes\flash.h"

#include "Library\includes\system_clock.h"
#include "Library\Includes\rtc.h"
#include "Library\Includes\pwm.h"
#include "includes\system.h"
#include "Library\includes\adc.h"
#include "Library\includes\wdt.h"
/*********************************************************************************************************************/
#include "ticks_module.h"
#include "aip3368h_module.h"
#include "aip3368h_app.h"
#include "uart_process_data_packets.h"

#include <stdio.h>
#include "uart.h"
#include "timer0.h"

#include "user_config.h"

#include "aip3368.h"

/********************************************************************************************************
说明：
------------------------------------------------------------------------------------------------------------
1.ts_configuration.h是触摸的配置文件，可在配置文件中设置触摸通道、灵敏度等。详见ts_configuration.h中的说明。
------------------------------------------------------------------------------------------------------------
2.如果需要使用UART来调试，可在ca51f_config.h中打开宏定义UART0_EN
------------------------------------------------------------------------------------------------------------
3.触摸库对外的数据接口为KeysFlagSN，用户可根据KeysFlagSN的值来判断触摸键是否按下（如果触摸键按下，KeysFlagSN
的对应位一直为1，触摸键松开，对应位变为0），TS_Key是根据KeysFlagSN的值生成的按键消息，用户如不使用可忽略。
------------------------------------------------------------------------------------------------------------
4.使能了触摸省电模式后，在无按键时，在倒计时时间结束后会进入省电模式，进入省电模式前会关闭触摸中断，在触摸省
电模式下，CPU会进入STOP模式，在被唤醒后，程序在省电函数（TS_EnterSleepMode）的循环内运行（注意，触摸省电模式
程序不会在主循环内运行），在触摸省电模式下如果还要响应其他中断，用户需在进入触摸省电模式前自行开启其他中断，
如果需要退出省电模式，用户只需要在省电函数循环内加入条件判断，跳出循环即可。

变量TS_SleepEn是控制触摸省电模式的开关，如果希望程序在某些条件下不进入省电模式，只需在此条件下设置TS_SleepEn = 0；
------------------------------------------------------------------------------------------------------------
5.触摸外挂电容(即TK_CAP引脚连接电容)范围：10nF~47nF,建议值：22nF（即223）
------------------------------------------------------------------------------------------------------------
6.触摸引脚串联电阻范围：0~5K，建议值：1K欧姆，如果有抗对讲机等电磁干扰要求， 此电阻至少3K以上。
********************************************************************************************************/

/*
说明：在本例程中，Mcu_FeedDog() 函数需在主循环中调用；所设定的 (Feed_Wdt_Flag == 0xA5) 这一条件请勿取消或修改。
*/

/******************************************************************************************************/
//************************************************************************
#define com_rx P11
#define com_tx P12

// #define		com_sun_chk		P10
// #define		com_jg_chk		P13
#define com_abs_chk P14
#define com_wd_chk  P15
#define com_xd_chk  P16
#define com_pd_chk  P02

#define com_tk1 P35

// #define com_test P32

//------------------------
idata unsigned char shake_cnt = 0;
// idata unsigned char rpm_var = 0;
// idata unsigned int rpm_tmp = 0;
// idata unsigned char show_rpm_now = 0;
// idata unsigned char deb_tmp = 0;
idata unsigned char gear_num = 0;

idata unsigned char show_gas_var = 0;
idata unsigned char show_gas_tmp = 0;
idata unsigned char show_gas_cnt = 0;
idata unsigned char battery_tmp = 0;
idata unsigned char speed_tmp = 0;
idata unsigned char show_speed_var = 0;
idata unsigned char show_tmp = 0;
// idata unsigned char speed_show = 0;
idata unsigned char kmh_tmp = 0;
idata unsigned char mph_tmp = 0;
idata unsigned long km_mile_tmp = 0;
idata unsigned char u8_tmp = 0;
// idata unsigned char powup_cnt = 0;
// idata unsigned char powup2_stp = 0;

idata unsigned char chg_cnt = 0;
idata unsigned char chg_deb = 0;
idata unsigned char chg_shake_cnt = 0;

idata unsigned char hour_var = 0;
idata unsigned char min_var = 0;
idata unsigned char sec_var = 0;
// idata unsigned char x1sec_cnt = 0;

// idata unsigned char u0_recv_data = 0;
// idata unsigned char u0_cmd_status = 0;
// idata unsigned char uart0_recv_cmd_index = 0;
// idata unsigned char uart0_recv_len = 0;
idata unsigned char ram_a = 0;
idata unsigned char ram_b = 0;
idata unsigned char ram_c = 0;
idata unsigned char ram_d = 0;
// idata unsigned char last_level = 0;
// idata unsigned char volpow_var = 0;
// idata unsigned char volpow_base = 0;
// idata unsigned int base_energy = 0;
// idata unsigned int ten_percent = 0;

// idata unsigned int volpow_tmp = 0;
// idata unsigned char phone_len = 0;
// idata unsigned char phone_len_tmp = 0;

idata unsigned char send_cmd = 0;

idata unsigned int pwm_tmp = 0;
idata unsigned int pwm_var = 0;
idata unsigned int u16_tmp = 0;

idata unsigned char tmp_a = 0;
idata unsigned char tmp_b = 0;
idata unsigned char tmp_c = 0;
idata unsigned char key_tmp = 0;
idata unsigned char key_buf = 0;
idata unsigned char key_flag = 0;
idata unsigned char key_deb = 0;
idata unsigned char key_val1 = 0;
idata unsigned char key_val2 = 0;
idata unsigned char longpress_cnt = 0;
idata unsigned char long_deb = 0;
idata unsigned char shake_deb = 0;
idata unsigned char chg_stp = 0;
idata unsigned char enter_chg_cnt = 0;

xdata unsigned char ad_cnt = 0;
xdata unsigned char ad_data_tmp = 0;
xdata unsigned char ad_tmp = 0;
xdata unsigned int ad_sum = 0;

unsigned char act_cnt = 0;
unsigned char act_tmp = 0;
unsigned char powup_stp = 0;

volatile u8 uart0_buff[20];

unsigned char i;
idata unsigned char UserDataBuff[10];
idata unsigned char BuffTemp[13];
// idata unsigned char Phone_buff[15];

// idata unsigned char Ring_buff[15];
// idata unsigned char Call_buff[15];

//------------------------
bit powup_en = 0;
bit deal_showdat_en = 0;
bit chg_init_en = 0;
bit shake_show_en = 0;
bit chg_shake_show_en = 0;
bit keyled_en = 0;
bit felh_en = 0;
bit gear_en = 0;
bit chk_ad_en = 0;
bit fog_en = 0;
bit light_en = 0;
bit far_light_en = 0;
bit top_en = 0;
bit x1000r_min_en = 0;
bit r_en = 0;
bit frame_speed_en = 0;
bit fix_en = 0;
bit warm_water_temperature_en = 0;
bit warm_oil_temperature_en = 0;
bit warm_battery_en = 0;
bit warm_engine_en = 0;
bit brake_en = 0;
bit phone_en = 0;
bit bt_en = 0;
bit mp3_en = 0;
bit chg_en = 0;
bit chg_time_en = 0;
bit chg_hour_en = 0;
bit chg_show_en = 0;
bit shake_en = 0;
bit km_en = 0;
bit odo_en = 0;
bit send_en = 0;
bit save_en = 0;
bit init_en = 0;
bit slow_en = 0;

bit fade_en = 0;
bit fade_sta_en = 0;

bit key_deb_en = 0;
bit longpress_en = 0;
bit jg_en = 0;
bit wd_en = 0;
bit xd_en = 0;
bit pd_en = 0;
bit fe_en = 0;

/******************************************************************************************************/

#define PWM3DIV_V                                                              \
    (FOSC /                                                                    \
     12000) //当前示例选用系统时钟作为时钟源。若PWM时钟采用其他频率的时钟源，则需根据选择的时钟源，调整参数。
#define PWM3DUT_V                                                              \
    (PWM3DIV_V /                                                               \
     2) //本示例中占空比为50%，实际应用时，请根据具体产品需求进行调整。

#define PWM_100_VAL 1
#define PWM_90_VAL  184
#define PWM_80_VAL  369
#define PWM_70_VAL  553
#define PWM_60_VAL  738
#define PWM_50_VAL  922
#define PWM_40_VAL  1107
#define PWM_30_VAL  1291
#define PWM_20_VAL  1474
#define PWM_0_VAL   1845

#define SEG_A (1 << 0)
#define SEG_B (1 << 1)
#define SEG_C (1 << 2)
#define SEG_D (1 << 3)
#define SEG_E (1 << 4)
#define SEG_F (1 << 5)
#define SEG_G (1 << 6)

#define SHOW_0 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define SHOW_1 (SEG_B | SEG_C)
#define SHOW_2 (SEG_A | SEG_B | SEG_D | SEG_E | SEG_G)
#define SHOW_3 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G)
#define SHOW_4 (SEG_B | SEG_C | SEG_F | SEG_G)
#define SHOW_5 (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define SHOW_6 (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define SHOW_7 (SEG_A | SEG_B | SEG_C)
#define SHOW_8 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define SHOW_9 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)
#define SHOW_H (SEG_G)

const unsigned char SHOW_NUM[11] = {SHOW_0, SHOW_1, SHOW_2, SHOW_3,
                                    SHOW_4, SHOW_5, SHOW_6, SHOW_7,
                                    SHOW_8, SHOW_9, SHOW_H};
const unsigned char Chg_Tab[5] = {SHOW_1, SHOW_2, SHOW_4, SHOW_4, SHOW_2};

#define CHG_EXIT_TIME_VAL 50

#define SET_TIMIE_TO_MAIN 0x8E

// #define		VOLPOW_LV0_VAL		0x02
// #define		VOLPOW_LV1_VAL		0x0B
// #define		VOLPOW_LV2_VAL		0x14
// #define		VOLPOW_LV3_VAL		0x1D
// #define		VOLPOW_LV4_VAL		0x27
// #define		VOLPOW_LV5_VAL		0x35
// #define		VOLPOW_LV6_VAL		0x4E
// #define		VOLPOW_LV7_VAL		0x61
// #define		VOLPOW_LV8_VAL		0x75
// #define		VOLPOW_LV9_VAL		0x7D
// #define		VOLPOW_LV10_VAL		0x88

#define VOLPOW_LV0_VAL  500
#define VOLPOW_LV1_VAL  10000
#define VOLPOW_LV2_VAL  20000
#define VOLPOW_LV3_VAL  25000
#define VOLPOW_LV4_VAL  30000
#define VOLPOW_LV5_VAL  35000
#define VOLPOW_LV6_VAL  40000
#define VOLPOW_LV7_VAL  43000
#define VOLPOW_LV8_VAL  46000
#define VOLPOW_LV9_VAL  50000
#define VOLPOW_LV10_VAL 55000

#define FLASH_SIZE 32

/******************************************************************************************************/
void idle_gpio_init(void);
void Deal_PowUp_Pro(void);
void Deal_ShowDat_Pro(void);
void Set_Pwm_Var();
void Set_Show_Num(unsigned char tmp);
void Rpm_Show_Pro(unsigned char tmp);
void Speed_Show_Pro(unsigned char tmp);
void VolPow_Show_Pro(unsigned char tmp);
void Get_Energy_Level(unsigned int energy);
void Show_KMH_Pro();
void Show_MPH_Pro();
void Set_Show_MPH_KMH(unsigned char data_tmp);
void Set_Show_Km_Mile();
void Deal_AD_Pro();
void Scan_Key();
void Chk_LongPress_Pro();
void Send_Cmd_Pro(void);
void Recv_BT_Pro(void);
void Save_Pro();
void Pwm_Init();

/******************************************************************************************************/
void Mcu_FeedDog(void)
{
    // if (Feed_Wdt_Flag == 0xA5) {
    //     WDFLG = 0xA5;
    //     Feed_Wdt_Flag = 0x00;
    // }
}

unsigned char read_inner_trim(void)
{
    unsigned char value;
    FSCMD = 0x80;
    PTSH = 0x00;
    PTSL = 0x24;
    FSCMD = 0x81;
    value = FSDAT;
    FSCMD = 0;
    return value;
}

//***************************************************************
void SystemInit(void)
{
#ifdef LVD_RST_ENABLE
    LVDCON = 0xE1; //设置LVD复位电压为2V
#endif

#if (SYSCLK_SRC == PLL)
    Sys_Clk_Set_PLL(PLL_Multiple);
#endif

//备注：需默认开启看门狗功能，并将其设置为复位模式
#if (RTC_CLK_SELECT == IRCL)
    CKCON |= ILCKE; //使能IRCL
    WDCON = WDTS(WDTS_IRCL) |
            WDRE(WDRE_reset); //设置看门狗时钟源为IRCL，模式为复位模式
#elif (RTC_CLK_SELECT == XOSCL)
    CKCON |= XLCKE;
    while (!(CKCON & XLSTA))
        ;
    WDCON = WDTS(WDTS_XOSCL) |
            WDRE(WDRE_reset); //设置看门狗时钟源为XOSCL，模式为复位模式
#endif

    WDVTHH = 0;  //看门狗复位阈值高八位设置 当前值为5s
    WDVTHL = 75; //看门狗复位阈值低八位设置
    WDFLG = 0xA5;

    ADCFGH =
        (ADCFGH & 0xC0) | VTRIM(read_inner_trim()); // 加载ADC内部基准校准值
}

//***************************************************************
void AD_Init()
{
    /**********设置ADC引脚功能*************************************************************/
    P10F = P10_ADC0_SETTING; //设置P10为ADC引脚功能
    // 	P11F = P11_ADC1_SETTING;	//设置P11为ADC引脚功能
    // 	P12F = P12_ADC2_SETTING;	//设置P12为ADC引脚功能
    // 	P13F = P13_ADC3_SETTING;	//设置P13为ADC引脚功能
    // 	P14F = P14_ADC4_SETTING;	//设置P14为ADC引脚功能
    // 	P15F = P15_ADC5_SETTING;	//设置P15为ADC引脚功能
    // 	P16F = P16_ADC6_SETTING;	//设置P16为ADC引脚功能
    // 	P17F = P17_ADC7_SETTING;	//设置P17为ADC引脚功能

    // 		P10F = P10_ADC_VREF_SETTING;	//如果ADC参考电压选择为外部参考电压，需要设置P10为外部参考电压引脚，并且ADC0通道不能使用。
    /***************************************************************************************/

    /**********初始化ADC寄存器（设置ADC时钟、选择ADC参考电压、设置ADC运放、加载内部1.5V校准值）***********/
    // 	ADCON = AST(0) | ADIE(0) | HTME(7) | VSEL(ADC_REF_INNER);		//设置ADC参考电压为内部1.5V
    ADCON =
        AST(0) | ADIE(0) | HTME(7) | VSEL(ADC_REF_VDD); //设置ADC参考电压为VDD
    // 	ADCON = AST(0) | ADIE(0) | HTME(7) | VSEL(ADC_REF_Outer);		//设置ADC参考电压为外部参考电压

    ADCFGH = AOPS(NO_AMP) | VTRIM(read_inner_trim()); //加载内部1.5V校准值
    /**************************************************************************************************/

    /**********设置ADC通道和时钟分频*********************************************************************/
    ADCFGL = ACKD(7) | ADCHS(ADC_CH0); //选择ADC0通道
    // 	ADCFGL = ACKD(7) | ADCHS(ADC_CH1);	//选择ADC1通道
    // 	ADCFGL = ACKD(7) | ADCHS(ADC_CH2);	//选择ADC2通道
    // 	ADCFGL = ACKD(7) | ADCHS(ADC_CH3);	//选择ADC3通道
    // 	ADCFGL = ACKD(7) | ADCHS(ADC_CH4);	//选择ADC4通道
    // 	ADCFGL = ACKD(7) | ADCHS(ADC_CH5);	//选择ADC5通道
    // 	ADCFGL = ACKD(7) | ADCHS(ADC_CH6);	//选择ADC6通道
    // 	ADCFGL = ACKD(7) | ADCHS(ADC_CH7);	//选择ADC7通道
}

//***************************************************************
void my_loop(void)
{
    systimer_flag_update();

    if (systimer_flag_is_valid(SYSTIME_FLAG_100MS)) {
        // chk_ad_en = 1;
        if (chg_deb) {
            chg_deb--;
            if (chg_deb == 0) {
                chg_en = 0; //退出调整模式
                powup_en = 0;
                save_en = 1;
                // send_en = 1;
                // send_cmd = SET_TIMIE_TO_MAIN;
            }
        }
        // if(sec_var >= 60){		//每60s询问主机时间
        // 	if(send_en == 0){
        // 		send_en = 1;
        // 		sec_var = 0;
        // 		if(chg_en == 0){
        // 			send_cmd = 0x35;
        // 		}
        // 	}
        // }
        // else{
        // 	x1sec_cnt++;
        // 	if(x1sec_cnt >= 10){
        // 		x1sec_cnt = 0;
        // 		sec_var++;
        // 	}
        // }

        // if(chg_show_deb){
        // 	chg_show_deb--;
        // }
        // if(long_deb){
        // 	long_deb--;
        // 	if(long_deb == 0){
        // 		long_deb = 3;
        // 		if(key_val1&0x01){
        // 			Uart0_PutChar(0xA5);
        // 			Uart0_PutChar(0x04);
        // 			Uart0_PutChar(0x02);
        // 			Uart0_PutChar(0xAB);
        // 		}
        // 		else if(key_val1&0x02){
        // 			Uart0_PutChar(0xA5);
        // 			Uart0_PutChar(0x04);
        // 			Uart0_PutChar(0x01);
        // 			Uart0_PutChar(0xAA);
        // 		}
        // 	}
        // }
    }

    uart_process_data_packets();
    // Recv_BT_Pro();
    // ledseg_show_test();
    if (systimer_flag_is_valid(SYSTIME_FLAG_50MS)) {
        // if(systimer_flag_is_valid(SYSTIME_FLAG_60MS)){
        aip3368h_module_display();
        // Set_Pwm_Var();
    }

    Deal_PowUp_Pro();

    Deal_ShowDat_Pro();

    Scan_Key();
    Chk_LongPress_Pro();
#if 0
	Deal_AD_Pro();
#endif
    Send_Cmd_Pro();
    Save_Pro();
}

void main(void)
{ 

    SystemInit();
    // idle_gpio_init(); // 将所有未使用的GPIO设置为高阻状态+内部上拉，减少功耗
    EA = 1;

    uart1_init();
    timer0_init();


    // tick_timer_config();

    // tick_timer_config(); //定时器1初始化
    aip3368h_module_init();
    // aip3368h_module_display();

#if 0
	AD_Init();
#endif

    // powup_en = 1;
    // powup_stp = 0;
    // act_tmp = 0;

    // save_en = 1;
    // init_en = 1;

    while (1) {
        // Mcu_FeedDog(); //喂狗
        WDFLG = 0xA5; // 喂狗

        // my_loop();

        aip3368h_module_display();
    }
}

/******************************************************************************************************/
void idle_gpio_init(void)
{
    GPIO_Init(P00F, HIGH_Z | PU_EN);
    GPIO_Init(P01F, HIGH_Z | PU_EN);
    GPIO_Init(P03F, HIGH_Z | PU_EN);
    GPIO_Init(P04F, HIGH_Z | PU_EN);
    // GPIO_Init(P05F, HIGH_Z | PU_EN); // DIO
    // GPIO_Init(P06F, HIGH_Z | PU_EN); // DCK
    // GPIO_Init(P07F, HIGH_Z | PU_EN); // LAT
    // GPIO_Init(P34F, HIGH_Z | PU_EN); // PDM

    GPIO_Init(P20F, HIGH_Z | PU_EN);
    GPIO_Init(P21F, HIGH_Z | PU_EN);
    GPIO_Init(P16F, HIGH_Z | PU_EN);
    GPIO_Init(P17F, HIGH_Z | PU_EN);
    GPIO_Init(P36F, HIGH_Z | PU_EN);
    GPIO_Init(P37F, HIGH_Z | PU_EN);
    GPIO_Init(P32F, OUTPUT);
    GPIO_Init(P33F, HIGH_Z | PU_EN);

    GPIO_Init(P10F, INPUT);

    GPIO_Init(P11F, INPUT); // TX
    GPIO_Init(P12F, INPUT); // RX
    // GPIO_Init(P30F, HIGH_Z | PU_EN);
    // GPIO_Init(P31F, OUTPUT);
    //检测口
    GPIO_Init(P13F, INPUT | PU_EN);
    GPIO_Init(P14F, INPUT | PU_EN);
    GPIO_Init(P15F, INPUT | PD_EN);
    GPIO_Init(P02F, INPUT | PU_EN);
    //触摸
    GPIO_Init(P35F, HIGH_Z);

    // P16C = 0x50;
}

//=======================================================================
void Pwm_Init()
{
//  以打开PWM3为例，设置的重点：每组PWM的时钟源是共同设置的，时钟源通过PWM0、PWM2、PWM4对应的控制寄存器PWMCON的PWMCKS来选择。
#if (1)
    INDEX = PWM_CH2; //设置INDEX值对应PWM2
    PWMCON = TIE(0) | ZIE(0) | PIE(0) | NIE(0) | MS(0) |
             CKS_SYS; //设置PWM时钟源为系统时钟
    //	PWMCON = TIE(0) | ZIE(0) | PIE(0) | NIE(0) | MS(0) | CKS_IH ;		//设置PWM时钟源为IRCH
    // 	PWMCON = TIE(0) | ZIE(0) | PIE(0) | NIE(0) | MS(0) | CKS_IL ;		//设置PWM时钟源为IRCL
    // 	PWMCON = TIE(0) | ZIE(0) | PIE(0) | NIE(0) | MS(0) | CKS_XL ;		//设置PWM时钟源为XOSCL
    // 	PWMCON = TIE(0) | ZIE(0) | PIE(0) | NIE(0) | MS(0) | CKS_TF ;		//设置PWM时钟源为TFRC
    // 	PWMCON = TIE(0) | ZIE(0) | PIE(0) | NIE(0) | MS(0) | CKS_PLL;		//设置PWM时钟源为PLL
    INDEX = PWM_CH3;
    PWMCON = TIE(0) | ZIE(0) | PIE(0) | NIE(0) | MS(0) | MOD(0);
    PWMCFG = TOG(0) | 0;

    //设置PWMDIV、PWMDUT
    PWMDIVH = (unsigned char)(PWM3DIV_V >> 8);
    PWMDIVL = (unsigned char)(PWM3DIV_V);
    PWMDUTH = (unsigned char)(PWM3DUT_V >> 8);
    PWMDUTL = (unsigned char)(PWM3DUT_V);

    P34F = P34_PWM3_SETTING; //设置P3.4为PWM引脚功能

    PWMUPD |= (1 << PWM_CH3); //PWMDIV、PWMDUT更新使能
    while (PWMUPD)
        ;                    //等待更新完成
    PWMEN |= (1 << PWM_CH3); //PWM1使能
#endif
}

//=======================================================================
void Set_Pwm_Var()
{
    INDEX = PWM_CH3;
    PWMDUTH = (pwm_tmp >> 8);
    PWMDUTL = pwm_tmp;
    PWMUPD = (1 << PWM_CH3);
}

/******************************************************************************************************/
/*
void Recv_BT_Pro()
{
	if (uart0_rxbuffer_get_count() == 0)
    {
        return;
    }
	u0_recv_data = uart0_rxbuffer_get();
	// Uart0_PutChar(u0_cmd_status);

	if(u0_cmd_status == 0){
		// Uart0_PutChar(u0_recv_data);
		if(u0_recv_data == 0xAA){
			uart0_buff[0] = u0_recv_data;
			uart0_recv_cmd_index = 1;
			u0_cmd_status = 1;
		}
		// else{
			// Uart0_PutChar(u0_recv_data);
		// }
	}
	else if(u0_cmd_status == 1){
		uart0_buff[1] = u0_recv_data;
		uart0_recv_cmd_index = 2;
		if(u0_recv_data > 20){
			// 校验和错误
				u0_cmd_status = 0;
				// Uart0_PutChar(0x33);
		}
		else{
			uart0_recv_len = u0_recv_data;
			u0_cmd_status = 2;
		}
	}
	else if(u0_cmd_status == 2){
		uart0_buff[uart0_recv_cmd_index++] = u0_recv_data;

		if(uart0_recv_cmd_index >= uart0_recv_len){
			// 如果接收到的数据包已经接收完毕
			// 计算校验和
			ram_a = 0;
			for(ram_b=0;ram_b<uart0_recv_len-1;ram_b++){
				ram_a += uart0_buff[ram_b];
				// Uart0_PutChar(uart0_buff[ram_b]);
			}
			if(ram_a != uart0_buff[uart0_recv_len-1]){
				// 校验和错误
				u0_cmd_status = 0;
				// Uart0_PutChar(0x33);
			}
			else{
				// 校验和正确
				u0_cmd_status = 3;
				// Uart0_PutChar(0x55);
				goto  Deal_BT_Data_Pro;
			}
		}
	}
	else if(u0_cmd_status == 3){
		// ram_a = 0;
		// for(ram_b=0;ram_b<uart0_recv_len;ram_b++){
		// 	Uart0_PutChar(uart0_buff[ram_b]);
		// }
Deal_BT_Data_Pro:
		u0_cmd_status = 0;	// 重新接收数据

		switch(uart0_buff[2])
		{
			case 0xA6://手机时间
				hour_var = uart0_buff[3];
				min_var = uart0_buff[4];
				sec_var = uart0_buff[5];
				
				send_en = 1;
				send_cmd = SET_TIMIE_TO_MAIN;
				break;
			case 0xA7:
				if(uart0_buff[3] == 0x02){		//蓝牙状态
					//返回蓝牙状态
					// Uart0_PutChar(uart0_buff[4]);
					if(uart0_buff[4] < 6){
						bt_en = 0;
						phone_en = 0;
					}
					else{
						bt_en = 1;
						if((uart0_buff[4] == 8) || (uart0_buff[4] == 9)){
							phone_en = 1;
						}
						else{
							phone_en = 0;
						}
					}
					// Uart0_PutChar(bt_en);
				}
				else if(uart0_buff[3] == 0x04){		//音乐能量值
					volpow_tmp = (uart0_buff[4]<<8)+uart0_buff[5];

					Get_Energy_Level(volpow_tmp);
				}
				break;
			case 0xA9://来电号码
				phone_len = uart0_buff[3];
				if(uart0_buff[4] == 0x00){
					Phone_buff[0] = uart0_buff[5];
					Phone_buff[1] = uart0_buff[6];
					Phone_buff[2] = uart0_buff[7];
					Phone_buff[3] = uart0_buff[8];
					Phone_buff[4] = uart0_buff[9];
				}
				else if(uart0_buff[4] == 0x05){
					Phone_buff[5] = uart0_buff[5];
					Phone_buff[6] = uart0_buff[6];
					Phone_buff[7] = uart0_buff[7];
					Phone_buff[8] = uart0_buff[8];
					Phone_buff[9] = uart0_buff[9];
				}
				else if(uart0_buff[4] == 0x0A){
					Phone_buff[10] = uart0_buff[5];
					phone_en = 1;
				}

				// for(ram_a = 5;ram_a < 10;ram_a++){
				// 	Phone_buff[phone_len_tmp] = uart0_buff[ram_a];
				// 	phone_len_tmp++;
				// 	if(phone_len_tmp >= phone_len){
				// 		phone_len_tmp = 0;
				// 		phone_en = 1;
				// 		// Uart0_PutChar(phone_en);
				// 		break;
				// 	}
				// }
				break;
			case 0xAA://致电号码
				phone_len = uart0_buff[3];
				if(uart0_buff[4] == 0x00){
					Phone_buff[0] = uart0_buff[5];
					Phone_buff[1] = uart0_buff[6];
					Phone_buff[2] = uart0_buff[7];
					Phone_buff[3] = uart0_buff[8];
					Phone_buff[4] = uart0_buff[9];
				}
				else if(uart0_buff[4] == 0x05){
					Phone_buff[5] = uart0_buff[5];
					Phone_buff[6] = uart0_buff[6];
					Phone_buff[7] = uart0_buff[7];
					Phone_buff[8] = uart0_buff[8];
					Phone_buff[9] = uart0_buff[9];
				}
				else if(uart0_buff[4] == 0x0A){
					Phone_buff[10] = uart0_buff[5];
					phone_en = 1;
				}

				// for(ram_a = 5;ram_a < 10;ram_a++){
				// 	Phone_buff[phone_len_tmp] = uart0_buff[ram_a];
				// 	phone_len_tmp++;
				// 	if(phone_len_tmp >= phone_len){
				// 		phone_len_tmp = 0;
				// 		phone_en = 1;
				// 		break;
				// 	}
				// }
				break;
			default:
				break;
		}
	}
}
*/

/*
void Get_Energy_Level(unsigned int energy)
{

	if(energy < VOLPOW_LV0_VAL){
		volpow_base = 1;
	}
	else if(energy < VOLPOW_LV1_VAL){
		volpow_base = 2;
	}
	else if(energy < VOLPOW_LV2_VAL){
		volpow_base = 3;
	}
	else if(energy < VOLPOW_LV3_VAL){
		volpow_base = 4;
	}
	else if(energy < VOLPOW_LV4_VAL){
		volpow_base = 5;
	}
	else if(energy < VOLPOW_LV5_VAL){
		volpow_base = 6;
	}
	else if(energy < VOLPOW_LV6_VAL){
		volpow_base = 7;
	}
	else if(energy < VOLPOW_LV7_VAL){
		volpow_base = 8;
	}
	else if(energy < VOLPOW_LV8_VAL){
		volpow_base = 9;
	}
	else if(energy < VOLPOW_LV9_VAL){
		volpow_base = 10;
	}
	else if(energy < VOLPOW_LV10_VAL){
		volpow_base = 11;
	}
	else{
		volpow_base = 12;
	}

	if((last_level == volpow_base) && (volpow_base > 1)){
		ten_percent = base_energy * 1U / 100U;	// 基准的1%
		if (ten_percent == 0) ten_percent = 1; // 避免基准为0时除以0

		if((energy - base_energy) >= ten_percent){
			if((volpow_base+1)<=4){		       //限制摆幅，以免看起来不真实
				if(volpow_var < (volpow_base+1)){
					volpow_var++;
				}
			}
			else{
				if(volpow_var < 12){
					volpow_var++;
				}
			}
		}
		else if((base_energy - energy) >= ten_percent){
			if(volpow_var > 1){
				volpow_var--;
			}
		}
	}
	else{// 强节奏-能量级变化，根据能量级变化
		last_level = volpow_var;
		volpow_var = volpow_base;
	}
	base_energy = energy;
}
*/

/******************************************************************************************************/
void Deal_PowUp_Pro()
{
    if (powup_en == 0)
        return;

    if (!systimer_flag_is_valid(SYSTIME_FLAG_25MS))
        return;
    // if(!systimer_flag_is_valid(SYSTIME_FLAG_1000MS)) return;
    // if(powup_cnt_en == 0) return;
    // powup_cnt_en = 0;

    if (chg_en) {
        if (chg_stp == 2) {
            if (chg_init_en) {
                chg_init_en = 0;
                for (act_tmp = 0; act_tmp < AIP3368H_DEV_NUM; act_tmp++) {
                    display_buff[act_tmp] = 0x0000;
                }
                act_tmp = 0;
            }

            display_buff[14] |= (7 << 7); //转速单位图标
            display_buff[12] |= (0x7F << 9);
            display_buff[11] |= (1 << 0); //转速数字
            display_buff[12] |= (0x3F << 0);
            display_buff[13] |= 0xFFF0; //转速框

            display_buff[15] |= (1 << 13); //电量图标

            display_buff[14] &= ~0x007F;
            display_buff[14] &= ~(1 << 12); //GEAR
            display_buff[14] |= Chg_Tab[gear_num];
            if ((gear_num == 1) || (gear_num == 2)) {
                display_buff[14] |= (1 << 12); //GEAR
            }

            act_tmp++;
            if (act_tmp >= 29) {
                // powup_en = 0;
            }
            Rpm_Show_Pro(act_tmp);
        }
        return;
    }

    if (powup_stp == 0) {

        pwm_var = PWM_100_VAL;
        pwm_tmp = pwm_var;
        // pwm_var = PWM_20_VAL;
        act_tmp++;
        if (act_tmp <= 18) {
            display_buff[7] >>= 1;
            if (display_buff[6] & 0x0001) {
                display_buff[7] |= 0x8000;
            }
            display_buff[6] >>= 1;
            display_buff[6] |= 0x2000;

            display_buff[12] <<= 1;
            if (display_buff[13] & 0x8000) {
                display_buff[12] |= 0x0001;
            }
            display_buff[13] <<= 1;
            display_buff[13] |= 0x0010;
        }

        display_buff[5] &= ~(1 << 2);
        display_buff[4] &= ~(1 << 3);    //1
        display_buff[5] &= ~(0x7F << 3); //8
        display_buff[5] &= ~(0x3F << 10);
        display_buff[4] &= ~(1 << 0); //8
        if (act_tmp == 1) {
            display_buff[5] |= (1 << 2);
        } else if (act_tmp == 2) {
            display_buff[5] |= (1 << 2);
            display_buff[4] |= (1 << 3);
        } else if (act_tmp == 3) {
            display_buff[5] |= (1 << 2);
            display_buff[4] |= (1 << 3);
            display_buff[5] |= (1 << 6);
        } else if (act_tmp == 4) {
            display_buff[4] |= (1 << 3);
            display_buff[5] |= (1 << 6);
            display_buff[5] |= (1 << 5);
        } else if (act_tmp == 5) {
            display_buff[5] |= (1 << 6);
            display_buff[5] |= (1 << 5);
            display_buff[5] |= (1 << 4);
        } else if (act_tmp == 6) {
            display_buff[5] |= (1 << 5);
            display_buff[5] |= (1 << 4);
            display_buff[5] |= (1 << 3);
        } else if (act_tmp == 7) {
            display_buff[5] |= (1 << 4);
            display_buff[5] |= (1 << 3);
            display_buff[5] |= (1 << 8);
        } else if (act_tmp == 8) {
            display_buff[5] |= (1 << 3);
            display_buff[5] |= (1 << 8);
            display_buff[5] |= (1 << 7);
        } else if (act_tmp == 9) {
            display_buff[5] |= (1 << 8);
            display_buff[5] |= (1 << 7);
            display_buff[5] |= (1 << 6);
        } else if (act_tmp == 10) {
            display_buff[5] |= (1 << 7);
            display_buff[5] |= (1 << 6);
            display_buff[5] |= (1 << 13);
        } else if (act_tmp == 11) {
            display_buff[5] |= (1 << 6);
            display_buff[5] |= (1 << 13);
            display_buff[5] |= (1 << 12);
        } else if (act_tmp == 12) {
            display_buff[5] |= (1 << 13);
            display_buff[5] |= (1 << 12);
            display_buff[5] |= (1 << 11);
        } else if (act_tmp == 13) {
            display_buff[5] |= (1 << 12);
            display_buff[5] |= (1 << 11);
            display_buff[5] |= (1 << 10);
        } else if (act_tmp == 14) {
            display_buff[5] |= (1 << 11);
            display_buff[5] |= (1 << 10);
            display_buff[5] |= (1 << 15);
        } else if (act_tmp == 15) {
            display_buff[5] |= (1 << 10);
            display_buff[5] |= (1 << 15);
            display_buff[5] |= (1 << 14);
        } else if (act_tmp == 16) {
            display_buff[5] |= (1 << 15);
            display_buff[5] |= (1 << 14);
            display_buff[5] |= (1 << 13);
        } else if (act_tmp == 17) {
            display_buff[5] |= (1 << 14);
            display_buff[5] |= (1 << 13);
            display_buff[5] |= (1 << 12);
        } else if (act_tmp == 18) {
            display_buff[5] |= (1 << 13);
            display_buff[5] |= (1 << 12);
            display_buff[5] |= (1 << 11);
        } else if (act_tmp == 19) {
            display_buff[5] |= (1 << 12);
            display_buff[5] |= (1 << 11);
            display_buff[5] |= (1 << 10);
        } else if (act_tmp == 20) {
            display_buff[5] |= (1 << 11);
            display_buff[5] |= (1 << 10);
            display_buff[5] |= (1 << 3);
        } else {
            act_tmp = 0;
            powup_stp = 1;

            display_buff[5] &= ~(1 << 2);
            display_buff[4] &= ~(1 << 3);    //1
            display_buff[5] &= ~(0x7F << 3); //8
            display_buff[5] &= ~(0x3F << 10);
            display_buff[4] &= ~(1 << 0); //8
        }
    } else if (powup_stp == 1) {
        act_tmp++;
        if (act_tmp == 1) {
            display_buff[5] |= (1 << 2);
            display_buff[4] |= (1 << 3); //1

            display_buff[1] |= (1 << 8);
            display_buff[4] |= (1 << 4); //左转
            display_buff[4] |= (3 << 5); //TOTAL

            display_buff[4] |= (0x1F << 11);
            display_buff[3] |= 0x0003; //

        } else if (act_tmp == 2) {
            display_buff[5] |= (0x7F << 3); //8

            display_buff[4] |= (1 << 7); //TRIP

            display_buff[3] |= (0x7F << 2); //
            display_buff[3] |= (0x7F << 9); //
        } else if (act_tmp == 3) {
            display_buff[5] |= (0x3F << 10);
            display_buff[4] |= 0x0001; //8

            display_buff[2] |= (0x7F << 0); //
            display_buff[2] |= (0x7F << 7); //
        } else if (act_tmp == 4) {
            display_buff[4] |= (1 << 1); //mph
            display_buff[4] |= (1 << 2); //kmh

            display_buff[2] |= (1 << 14); //小数点
            display_buff[2] |= (1 << 15);
            display_buff[1] |= 0x003F;   //
            display_buff[1] |= (1 << 6); //mile
            display_buff[1] |= (1 << 7); //km
        } else if (act_tmp == 5) {
            display_buff[4] |= (3 << 9); //右转
        } else if (act_tmp == 6) {
            display_buff[9] |= (7 << 3);

            display_buff[12] |= (1 << 8);
        } else if (act_tmp == 7) {
            display_buff[14] |= (1 << 14); //远光灯

            display_buff[12] |= (1 << 7); //油量图标
            display_buff[12] |= (1 << 6);
            display_buff[9] |= (3 << 6);
        } else if (act_tmp == 8) {
            display_buff[14] |= (7 << 7); //转速单位图标
            display_buff[14] |= SHOW_NUM[8];
        } else if (act_tmp == 9) {
            display_buff[14] |= (1 << 12); //GEAR
            display_buff[14] |= (1 << 10); //N档
        } else if (act_tmp == 10) {
            display_buff[15] |= (1 << 14);
            display_buff[11] |= (1 << 2);
            display_buff[11] |= (1 << 3);
            display_buff[15] |= (1 << 13); //电量图标
        } else if (act_tmp == 11) {
            display_buff[11] |= (0x39 << 1);
        } else if (act_tmp == 12) {
            display_buff[7] |= (0xFF << 4); //时速数字

            display_buff[12] |= (0x7F << 9);
            display_buff[11] |= (1 << 0); //转速数字
        } else if (act_tmp == 13) {
            display_buff[12] |= (0x3F << 0);
            display_buff[13] |= 0xFFF0; //转速框

            display_buff[6] |= (0x3FFF); //
            display_buff[7] |= (0xF000); //时速框

            Rpm_Show_Pro(30);
            Speed_Show_Pro(30);
        } else {
            act_tmp = 0;
            powup_stp = 2;

            pwm_var = PWM_20_VAL;
            // fade_en = 1;
            // fade_sta_en = 0;
        }
    } else if (powup_stp == 2) {
        if (pwm_var == pwm_tmp) {
            powup_stp = 3;
            // pwm_var = PWM_100_VAL;
        }

        // if(pwm_var < PWM_20_VAL){
        // 	// pwm_var += 14;
        // 	pwm_var++;
        // }
        // else{
        // 	pwm_var = PWM_20_VAL;
        // 	// powup_stp = 3;
        // }
        // Set_Pwm_Var();
    }
    // else if(powup_stp == 4){
    // 	if(pwm_var == pwm_tmp){
    // 		powup_stp = 4;
    // 	}
    // }
    else if (powup_stp == 3) {
        display_buff[14] &= ~0x007F;
        display_buff[14] |= SHOW_NUM[0];

        display_buff[9] &= ~(0x1F << 3);
        display_buff[11] &= ~(0x1F << 2);

        // display_buff[12] &= ~(0x3F<<0);
        // display_buff[13] &= ~0xFFF0;		//转速框
        display_buff[11] &= ~(0x1FF << 7);
        display_buff[10] &= ~(0xFFFF);
        display_buff[9] &= ~(7 << 0); //转速显示

        // display_buff[6] &= ~(0x3FFF);	//
        // display_buff[7] &= ~(0xF000);	//时速框
        display_buff[9] &= ~(0xFF << 8);
        display_buff[8] &= ~0xFFFF;
        display_buff[7] &= ~(0x0F); //时速显示

        display_buff[5] &= ~(1 << 2);
        display_buff[4] &= ~(1 << 3);    //1
        display_buff[5] &= ~(0x7F << 3); //8
        display_buff[5] &= ~(0x3F << 10);
        display_buff[4] &= ~(1 << 0); //8
        Set_Show_MPH_KMH(100);

        display_buff[4] &= ~(0x1F << 11);
        display_buff[3] &= ~0x0003;      //
        display_buff[3] &= ~(0x7F << 2); //
        display_buff[3] &= ~(0x7F << 9); //
        display_buff[2] &= ~(0x7F << 0); //
        display_buff[2] &= ~(0x7F << 7); //
        display_buff[2] &= ~(1 << 14);   //小数点
        display_buff[2] &= ~(1 << 15);
        display_buff[1] &= ~0x003F; //

        display_buff[2] |= (1 << 14); //小数点

        km_mile_tmp = 0;
        show_tmp = km_mile_tmp % 10;
        u8_tmp = SHOW_NUM[show_tmp];
        if (u8_tmp & SEG_A) {
            display_buff[2] |= (1 << 15);
        }
        display_buff[1] |= SHOW_NUM[show_tmp] >> 1;
        km_mile_tmp /= 10;
        show_tmp = km_mile_tmp % 10;
        display_buff[2] |= SHOW_NUM[show_tmp] << 7;
        km_mile_tmp /= 10;
        show_tmp = km_mile_tmp % 10;
        display_buff[2] |= SHOW_NUM[show_tmp];
        km_mile_tmp /= 10;
        show_tmp = km_mile_tmp % 10;
        display_buff[3] |= SHOW_NUM[show_tmp] << 9;
        km_mile_tmp /= 10;
        show_tmp = km_mile_tmp % 10;
        display_buff[3] |= SHOW_NUM[show_tmp] << 2;
        km_mile_tmp /= 10;
        show_tmp = km_mile_tmp % 10;
        display_buff[4] |= SHOW_NUM[show_tmp] << 11;
        u8_tmp = SHOW_NUM[show_tmp];
        if (u8_tmp & SEG_F) {
            display_buff[3] |= (1 << 0);
        }
        if (u8_tmp & SEG_G) {
            display_buff[3] |= (1 << 1);
        }

        powup_stp = 4;
        pwm_var = PWM_100_VAL;
    } else if (powup_stp == 4) {
        if (pwm_var == pwm_tmp) {
            powup_stp = 5;
            act_tmp = 0;
            u8_tmp = 0;
            ram_a = 0;
            ram_b = 0;
            ram_c = 0;
            ram_d = 0;
        }

        // if(pwm_var > PWM_0_VAL){
        // 	pwm_var -= 14;
        // }
        // else{
        // 	pwm_var = pwm_set;
        // 	powup_stp = 5;
        // 	act_tmp = 0;
        // 	u8_tmp = 0;

        // 	display_buff[9] |= (1<<7);
        // 	display_buff[11] |= (1<<2);
        // }
        // Set_Pwm_Var();
    } else if (powup_stp == 5) {
        act_tmp++;

        // if((act_tmp%2) == 0){
        ram_d++;
        Speed_Show_Pro(ram_d);
        Rpm_Show_Pro(ram_d);
        // Uart0_PutChar(ram_d);

        if ((ram_d % 4) == 0) {
            if (ram_c < 6)
                ram_c++;
            display_buff[14] &= ~0x007F;
            display_buff[14] |= SHOW_NUM[ram_c];
        }
        // }
        if ((act_tmp % 6) == 0) {
            display_buff[5] &= ~(1 << 2);
            display_buff[4] &= ~(1 << 3);    //1
            display_buff[5] &= ~(0x7F << 3); //8
            display_buff[5] &= ~(0x3F << 10);
            display_buff[4] &= ~(1 << 0); //8

            display_buff[4] &= ~(0x1F << 11);
            display_buff[3] &= ~0x0003;      //
            display_buff[3] &= ~(0x7F << 2); //
            display_buff[3] &= ~(0x7F << 9); //
            display_buff[2] &= ~(0x7F << 0); //
            display_buff[2] &= ~(0x7F << 7); //
            display_buff[2] &= ~(1 << 14);   //小数点
            display_buff[2] &= ~(1 << 15);
            display_buff[1] &= ~0x003F; //

            display_buff[2] |= (1 << 14); //小数点

            ram_a++;
            if (ram_a == 1) {
                Set_Show_MPH_KMH(111);
                km_mile_tmp = 111111;
                Set_Show_Km_Mile();
            } else if (ram_a == 2) {
                Set_Show_MPH_KMH(122);
                km_mile_tmp = 222222;
                Set_Show_Km_Mile();
            } else if (ram_a == 3) {
                Set_Show_MPH_KMH(133);
                km_mile_tmp = 333333;
                Set_Show_Km_Mile();
            } else if (ram_a == 4) {
                Set_Show_MPH_KMH(144);
                km_mile_tmp = 444444;
                Set_Show_Km_Mile();
            } else if (ram_a == 5) {
                Set_Show_MPH_KMH(155);
                km_mile_tmp = 555555;
                Set_Show_Km_Mile();
            } else if (ram_a == 6) {
                Set_Show_MPH_KMH(166);
                km_mile_tmp = 666666;
                Set_Show_Km_Mile();
            } else if (ram_a == 7) {
                Set_Show_MPH_KMH(177);
                km_mile_tmp = 777777;
                Set_Show_Km_Mile();
            } else if (ram_a == 8) {
                Set_Show_MPH_KMH(188);
                km_mile_tmp = 888888;
                Set_Show_Km_Mile();
            }
            // else if(ram_a == 9){
            else {
                Set_Show_MPH_KMH(199);
                km_mile_tmp = 999999;
                Set_Show_Km_Mile();
            }
        }
        if ((act_tmp % 12) == 0) {
            ram_b++;
            if (ram_b == 1) {
                display_buff[9] |= (1 << 6);
                display_buff[11] |= (1 << 3);
            } else if (ram_b == 2) {
                display_buff[9] |= (1 << 5);
                display_buff[11] |= (1 << 4);
            } else if (ram_b == 3) {
                display_buff[9] |= (1 << 4);
                display_buff[11] |= (1 << 5);
            }
            // else if(ram_b == 4){
            else {
                display_buff[9] |= (1 << 3);
                display_buff[11] |= (1 << 6);
            }
        }

        if (ram_d > 60) {
            powup_stp = 6;
            act_tmp = 0;
        }
    } else if (powup_stp == 6) {
        for (ram_a = 0; ram_a <= 16; ram_a++) {
            display_buff[ram_a] = 0x0000;
        }

        display_buff[7] |= (0xFF << 4); //时速数字
        display_buff[6] |= (0x3FFF);    //
        display_buff[7] |= (0xF000);    //时速框

        display_buff[14] |= (7 << 7); //转速单位图标
        display_buff[12] |= (0x7F << 9);
        display_buff[11] |= (1 << 0); //转速数字
        display_buff[12] |= (0x3F << 0);
        display_buff[13] |= 0xFFF0; //转速框

        display_buff[14] |= (1 << 12); //GEAR

        display_buff[12] |= (1 << 6);
        display_buff[12] |= (1 << 8);
        display_buff[12] |= (1 << 7); //油量图标

        display_buff[15] |= (1 << 14);
        display_buff[11] |= (1 << 1);
        display_buff[15] |= (1 << 13); //电量图标

        act_tmp++;
        if (act_tmp > 10) {
            act_tmp = 0;

            powup_en = 0;
        }
    }
}

/******************************************************************************************************/
void Deal_ShowDat_Pro()
{
    if (powup_en)
        return;

    if (deal_showdat_en == 0)
        return;
    deal_showdat_en = 0;
    //50ms

    shake_cnt++;
    if (shake_cnt < 10) {
        shake_show_en = 1;
    } else if (shake_cnt < 20) {
        shake_show_en = 0;
    } else {
        shake_cnt = 0;
    }

    chg_shake_cnt++;
    if (chg_shake_cnt < 10) {
        chg_shake_show_en = 1;
    } else if (chg_shake_cnt < 20) {
        chg_shake_show_en = 0;
    } else {
        chg_shake_cnt = 0;
    }
    //------------------------
    // fe_en = 1;
    // if(fe_en){
    // 	display_buff[12] |= 0x0140;
    // }

    if (instrument_info.left_turn_status) {
        display_buff[1] |= (1 << 8);
        display_buff[4] |= (1 << 4); //左转
    } else {
        display_buff[1] &= ~(1 << 8);
        display_buff[4] &= ~(1 << 4);
    }
    if (instrument_info.high_beam_status) {
        display_buff[14] |= (1 << 14); //远光灯
    } else {
        display_buff[14] &= ~(1 << 14);
    }
    if (instrument_info.right_turn_status) {
        display_buff[4] |= (3 << 9); //右转
    } else {
        display_buff[4] &= ~(3 << 9);
    }
    //---------------------------------
    //油量显示
    display_buff[9] &= ~(0x1F << 3);
    display_buff[12] &= ~(1 << 7); //油量图标

    display_buff[12] |= (1 << 6);
    display_buff[12] |= (1 << 8);

    if (instrument_info.oil_percent == 0xFF) {
        show_gas_cnt = 0;
        show_gas_tmp = 0;
        show_gas_var = 0;
        if (shake_show_en) {
            display_buff[12] |= (1 << 7); //油量图标

            display_buff[9] |= (1 << 7);
        }
    } else {
        if (instrument_info.oil_percent > 100)
            instrument_info.oil_percent = 100;
        display_buff[12] |= (1 << 7);

        show_gas_cnt++;
        if (show_gas_cnt >= 60) {
            show_gas_cnt = 0;

            if (instrument_info.oil_percent < 10) {
                show_gas_tmp = 0;
            } else if (instrument_info.oil_percent < 30) {
                show_gas_tmp = 1;
            } else if (instrument_info.oil_percent < 60) {
                show_gas_tmp = 2;
            } else if (instrument_info.oil_percent < 90) {
                show_gas_tmp = 3;
            } else {
                show_gas_tmp = 4;
            }

            if (show_gas_var != show_gas_tmp) {
                if (show_gas_var < show_gas_tmp) {
                    show_gas_var++;
                } else {
                    show_gas_var--;
                }
            }
        }
        switch (show_gas_var) {
        default:
        case 4:
            display_buff[9] |= (1 << 3);
        case 3:
            display_buff[9] |= (1 << 4);
        case 2:
            display_buff[9] |= (1 << 5);
        case 1:
            display_buff[9] |= (1 << 6);
        case 0:
            display_buff[9] |= (1 << 7);
            break;
        }
    }
    //---------------------------------
    //电量显示
    display_buff[15] |= (1 << 14);
    display_buff[11] |= (1 << 1);
    display_buff[11] &= ~(0x1F << 2);

    display_buff[15] &= ~(1 << 13); //电量图标

    if (warm_battery_en) {
        display_buff[15] |= (1 << 13); //电量图标
    }

    battery_tmp = instrument_info.battery_percent / 10;
    switch (battery_tmp) {
    case 10:
    case 9:
        display_buff[11] |= (1 << 6);
    case 8:
    case 7:
        display_buff[11] |= (1 << 5);
    case 6:
    case 5:
        display_buff[11] |= (1 << 4);
    case 4:
    case 3:
        display_buff[11] |= (1 << 3);
    case 2:
    case 1:
    case 0:
        display_buff[11] |= (1 << 2);
    default:
        break;
    }
    //*********************************************
    //档位显示
    display_buff[14] |= (1 << 12); //GEAR

    display_buff[14] &= ~(1 << 10); //N档
    display_buff[14] &= ~0x007F;

    if (instrument_info.gear_status == 0xFF) {

    } else if (instrument_info.gear_status == 0) {
        display_buff[14] |= (1 << 10);
    } else {
        if (instrument_info.gear_status > 6)
            instrument_info.gear_status = 6;
        display_buff[14] |= SHOW_NUM[instrument_info.gear_status];
    }
    //*********************************************
    //转速显示
    display_buff[14] |= (7 << 7); //转速单位图标
    display_buff[12] |= (0x7F << 9);
    display_buff[11] |= (1 << 0); //转速数字

    if (chg_en) {
        if (shake_en) {
            enter_chg_cnt++;
            if ((enter_chg_cnt & 0x04) == 0) {
                display_buff[12] &= ~(0x3F << 0);
                display_buff[13] &= ~0xFFF0; //转速框
            } else {
                display_buff[12] |= (0x3F << 0);
                display_buff[13] |= 0xFFF0; //转速框
            }
            if (shake_deb) {
                shake_deb--;
                if (shake_deb == 0) {
                    shake_en = 0;
                    chg_stp = 0;
                    chg_deb = 100; //10s退出调整模式
                }
            }
        } else {
            display_buff[12] &= ~(0x3F << 0);
            display_buff[13] &= ~0xFFF0; //转速框
        }
    } else {
        display_buff[12] |= (0x3F << 0);
        display_buff[13] |= 0xFFF0; //转速框
    }

    // if(instrument_info.engine_speed == 0){
    // 	display_buff[11] &= ~(0x01FF<<7);
    // 	display_buff[10] &= ~0xFFFF;
    // 	display_buff[9] &= ~(0x07<<0);
    // }
    // else{
    speed_tmp = instrument_info.engine_speed / 500;
    if ((instrument_info.engine_speed % 500) > 450) {
        speed_tmp++;
    }
    if (speed_tmp != show_speed_var) {
        if (speed_tmp > show_speed_var) {
            show_speed_var++;
        } else {
            show_speed_var--;
        }
    } else if (instrument_info.engine_speed == 0) {
        display_buff[11] &= ~(0x01FF << 7);
        display_buff[10] &= ~0xFFFF;
        display_buff[9] &= ~(0x07 << 0);
        goto Show_RPM_End;
    }
    Rpm_Show_Pro(show_speed_var);
Show_RPM_End:
    // }
    //*********************************************
    //时速
    display_buff[7] |= (0xFF << 4); //时速数字
    display_buff[6] |= (0x3FFF);    //
    display_buff[7] |= (0xF000);    //时速框

    display_buff[4] &= ~(1 << 1); //mph
    display_buff[4] &= ~(1 << 2); //kmh

    display_buff[5] &= ~(1 << 2);
    display_buff[4] &= ~(1 << 3);    //1
    display_buff[5] &= ~(0x7F << 3); //8
    display_buff[5] &= ~(0x3F << 10);
    display_buff[4] &= ~(1 << 0); //8

    if ((chg_en) && (chg_stp == 1)) {
        if (chg_shake_show_en) {
            if (km_en == 0) {
                display_buff[4] |= (1 << 1); //mph
            } else {
                display_buff[4] |= (1 << 2); //kmh
            }
        }
    } else if (km_en == 0) {
        display_buff[4] |= (1 << 1); //mph
    } else {
        display_buff[4] |= (1 << 2); //kmh
    }

    if (km_en == 0) {
        // display_buff[4] |= (1<<1);	//mph
        Show_MPH_Pro();
        Set_Show_MPH_KMH(mph_tmp);
    } else {
        // display_buff[4] |= (1<<2);	//kmh
        Show_KMH_Pro();
        Set_Show_MPH_KMH(kmh_tmp);
    }
    if (km_en == 0) {
        if (mph_tmp) {
            u8_tmp = mph_tmp / 5;
            if (u8_tmp == 0) {
                u8_tmp = 1;
            } else {
                u8_tmp += 1;
            }
        } else {
            u8_tmp = 0;
        }
        Speed_Show_Pro(u8_tmp);
    } else {
        if (kmh_tmp) {
            u8_tmp = kmh_tmp / 5;
            if (u8_tmp == 0) {
                u8_tmp = 1;
            } else {
                u8_tmp += 1;
            }
        } else {
            u8_tmp = 0;
        }
        Speed_Show_Pro(u8_tmp);
    }
    //*********************************************
    //里程
    display_buff[4] &= ~(3 << 5); //TOTAL
    display_buff[4] &= ~(1 << 7); //TRIP

    display_buff[1] &= ~(1 << 6); //mile
    display_buff[1] &= ~(1 << 7); //km

    display_buff[4] &= ~(0x1F << 11);
    display_buff[3] &= ~0x0003;      //
    display_buff[3] &= ~(0x7F << 2); //
    display_buff[3] &= ~(0x7F << 9); //
    display_buff[2] &= ~(0x7F << 0); //
    display_buff[2] &= ~(0x7F << 7); //
    display_buff[2] &= ~(1 << 14);   //小数点
    display_buff[2] &= ~(1 << 15);
    display_buff[1] &= ~0x003F; //

    if ((chg_en) && (chg_stp == 1)) {
        if (chg_shake_show_en) {
            if (km_en == 0) {
                display_buff[1] |= (1 << 6); //mile
            } else {
                display_buff[1] |= (1 << 7); //km
            }
        }
    } else if (km_en == 0) {
        display_buff[1] |= (1 << 6); //mile
    } else {
        display_buff[1] |= (1 << 7); //km
    }

    if (km_en == 0) {
        // display_buff[1] |= (1<<6);	//mile
        if (odo_en) {
            display_buff[4] |= (3 << 5); //TOTAL
            // display_buff[2] |= (1<<14);	//小数点
            km_mile_tmp = instrument_info.total_mileage_with_mile;
        } else {
            display_buff[4] |= (1 << 7);  //TRIP
            display_buff[2] |= (1 << 14); //小数点
            km_mile_tmp = instrument_info.subtotal_mileage_with_mile;
        }
    } else {
        // display_buff[1] |= (1<<7);	//km
        if (odo_en) {
            display_buff[4] |= (3 << 5); //TOTAL
            // display_buff[2] |= (1<<14);	//小数点
            km_mile_tmp = instrument_info.total_mileage_with_km;
        } else {
            display_buff[4] |= (1 << 7);  //TRIP
            display_buff[2] |= (1 << 14); //小数点
            km_mile_tmp = instrument_info.subtotal_mileage_with_km;
        }
    }

#if 0
	km_mile_tmp = 123456;
#endif

    Set_Show_Km_Mile();

    // show_tmp = km_mile_tmp%10;
    // u8_tmp = SHOW_NUM[show_tmp];
    // if(u8_tmp&SEG_A){
    // 	display_buff[2] |= (1<<15);
    // }
    // display_buff[1] |= SHOW_NUM[show_tmp]>>1;
    // km_mile_tmp /= 10;
    // show_tmp = km_mile_tmp%10;
    // display_buff[2] |= SHOW_NUM[show_tmp]<<7;

    // km_mile_tmp /= 10;
    // if(km_mile_tmp){
    // 	show_tmp = km_mile_tmp%10;
    // 	display_buff[2] |= SHOW_NUM[show_tmp];
    // }
    // km_mile_tmp /= 10;
    // if(km_mile_tmp){
    // 	show_tmp = km_mile_tmp%10;
    // 	display_buff[3] |= SHOW_NUM[show_tmp]<<9;
    // }
    // km_mile_tmp /= 10;
    // if(km_mile_tmp){
    // 	show_tmp = km_mile_tmp%10;
    // 	display_buff[3] |= SHOW_NUM[show_tmp]<<2;
    // }
    // km_mile_tmp /= 10;
    // if(km_mile_tmp){
    // 	show_tmp = km_mile_tmp%10;
    // 	display_buff[4] |= SHOW_NUM[show_tmp]<<11;
    // 	u8_tmp = SHOW_NUM[show_tmp];
    // 	if(u8_tmp&SEG_F){
    // 		display_buff[3] |= (1<<0);
    // 	}
    // 	if(u8_tmp&SEG_G){
    // 		display_buff[3] |= (1<<1);
    // 	}
    // }

    /*
	// if(keyled_en){
		display_buff[0] |= 0x0F00;
	// }
	// if(felh_en){
		display_buff[10] |= (1<<3);//L
		display_buff[8] |= (1<<3);//H
		display_buff[6] |= (1<<7);//F
		display_buff[7] |= (1<<10);//E
	// }
	// if(gear_en){
		display_buff[1] |= (3<<8);
	// }
	// if(x1000r_min_en){
		display_buff[8] |= 0x0F80;
	// }
	// if(frame_speed_en){//转速框+转速数字
		display_buff[5] = 0xFFFF;
		display_buff[4] |= 0x00FF;
		display_buff[0] |= 0x0007;
		display_buff[1] |= 0xF000;
	// }
	//------------------------
	if(instrument_info.left_turn_status){		//左转
		display_buff[7] |= (3<<5);
	}
	else{
		display_buff[7] &= ~(3<<5);
	}
	if(wd_en){		//雾灯
		display_buff[7] |= (3<<3);
		// Uart0_PutChar(key_val1&0x40);
	}
	else{
		display_buff[7] &= ~(3<<3);
	}
	if(xd_en){		//小灯
		display_buff[7] |= (3<<1);
	}
	else{
		display_buff[7] &= ~(3<<1);
	}
	if(instrument_info.high_beam_status){		//远光灯
		display_buff[8] |= (1<<15);
		display_buff[7] |= (1<<0);
	}
	else{
		display_buff[8] &= ~(1<<15);
		display_buff[7] &= ~(1<<0);
	}
	if(top_en){
		display_buff[8] |= (7<<12);
	}
	else{
		display_buff[8] &= ~(7<<12);
	}
	if(instrument_info.right_turn_status){		//右转
		display_buff[8] |= (3<<5);
	}
	else{
		display_buff[8] &= ~(3<<5);
	}
	//------------------------
	if(fix_en){
		display_buff[7] |= (1<<7);
	}
	else{
		display_buff[7] &= ~(1<<7);
	}
	if(warm_water_temperature_en){
		display_buff[7] |= (1<<9);
	}
	else{
		display_buff[7] &= ~(1<<9);
	}
	if(warm_oil_temperature_en){
		display_buff[10] |= (3<<9);
	}
	else{
		display_buff[10] &= ~(3<<9);
	}
	if(warm_battery_en){
		display_buff[10] |= (3<<11);
	}
	else{
		display_buff[10] &= ~(3<<11);
	}
	if(pd_en){		//发动机
		display_buff[10] |= (3<<13);
	}
	else{
		display_buff[10] &= ~(3<<13);
	}
	if(instrument_info.brake_status){		//刹车
		display_buff[0] |= (1<<3);
	}
	else{
		display_buff[0] &= ~(1<<3);
	}
	//---------------------------------
	//播放声音
	if(bt_en){
		display_buff[1] |= (1<<11);
	}
	else{
		display_buff[1] &= ~(1<<11);
	}
	if(mp3_en){
		display_buff[12] |= (3<<14);
	}
	else{
		display_buff[12] &= ~(3<<14);
	}
	//---------------------------------
	//外框显示,无音量时全显,有音量时,跟随能量律动
	display_buff[6] &= ~0xFF60;
	display_buff[7] &= ~0x1800;
	display_buff[8] &= ~0x0017;
	display_buff[9] &= ~0xFC03;

	if(volpow_tmp == 0){
		VolPow_Show_Pro(11);
	}
	else{
		VolPow_Show_Pro(volpow_var-1);
	}
	//---------------------------------
	//油量显示
	display_buff[7] &= ~(3<<13);//
	display_buff[7] &= ~(1<<15);
	display_buff[6] &= ~(1<<0);//
	display_buff[6] &= ~(3<<1);//
	display_buff[6] &= ~(3<<3);//

	display_buff[7] &= ~(1<<8);

	// Uart0_PutChar(show_gas_var);
	if(instrument_info.oil_percent == 0xFF){
		show_gas_cnt = 0;
		show_gas_tmp = 0;
		show_gas_var = 0;
		if(shake_show_en){
			display_buff[7] |= (1<<8);

			display_buff[7] |= (3<<13);//
		}
	}
	else{
		if(instrument_info.oil_percent > 100) instrument_info.oil_percent = 100;
		display_buff[7] |= (1<<8);

		show_gas_cnt++;
		if(show_gas_cnt >= 60){
			show_gas_cnt = 0;
			//3s变化一次

			if(instrument_info.oil_percent < 10){
				show_gas_tmp = 0;
			}
			else if(instrument_info.oil_percent < 30){
				show_gas_tmp = 1;
			}
			else if(instrument_info.oil_percent < 60){
				show_gas_tmp = 2;
			}
			else if(instrument_info.oil_percent < 90){
				show_gas_tmp = 3;
			}
			else{
				show_gas_tmp = 4;
			}

			if(show_gas_var != show_gas_tmp){
				if(show_gas_var < show_gas_tmp){
					show_gas_var++;
				}
				else{
					show_gas_var--;
				}
			}
		}
		switch(show_gas_var)
		{
			default:
			case 3:
				display_buff[6] |= (3<<3);//
			case 2:
				display_buff[6] |= (3<<1);//
			case 1:
				display_buff[7] |= (1<<15);
				display_buff[6] |= (1<<0);//
			case 0:
				display_buff[7] |= (3<<13);//
				break;
		}
	}
	//---------------------------------
	//电量显示
	display_buff[9] &= ~(0xFF<<2);//
	
	battery_tmp = instrument_info.battery_percent/10;

	switch(battery_tmp)
	{
		case 10:
		case 9:
			display_buff[9] |= (3<<8);
		case 8:
		case 7:
		case 6:
			display_buff[9] |= (3<<6);
		case 5:
		case 4:
			display_buff[9] |= (3<<4);
		case 3:
		case 2:
		case 1:
			display_buff[9] |= (3<<2);
		default:
			break;
	}
	
	//---------------------------------
	//档位显示
	display_buff[0] &= ~(1<<15);
	display_buff[1] &= ~(1<<7);
	display_buff[1] &= ~0x007F;
	
	if(instrument_info.gear_status == 0xFF){

	}
	else if(instrument_info.gear_status == 0){
		display_buff[0] |= (1<<15);
		display_buff[1] |= (1<<7);
		display_buff[1] &= ~(0x007F);
	}
	else{
		if(instrument_info.gear_status > 6) instrument_info.gear_status = 6;
		display_buff[1] |= SHOW_NUM[instrument_info.gear_status];
	}
	//*********************************************
	//转速显示
	display_buff[3] = 0;
	display_buff[4] &= 0x00FF;

	if((instrument_info.engine_speed == 0) && (volpow_tmp != 0)){
		show_speed_var = 0;
		if(volpow_tmp == 0){
			goto	Show_Speed_Pro;
		}
		else{
			switch(volpow_var-1){
				case 11:
				case 10:
					display_buff[3] |= (1<<3);
					display_buff[3] |= (1<<4);
				case 9:
					display_buff[3] |= (1<<2);
					display_buff[3] |= (1<<5);
				case 8:
					display_buff[3] |= (1<<1);
					display_buff[3] |= (1<<6);
				case 7:
					display_buff[3] |= (1<<0);
					display_buff[3] |= (1<<7);
				case 6:
					display_buff[4] |= (1<<15);
					display_buff[3] |= (1<<8);
				case 5:
					display_buff[4] |= (1<<14);
					display_buff[3] |= (1<<9);
				case 4:
					display_buff[4] |= (1<<13);
					display_buff[3] |= (1<<10);
				case 3:
					display_buff[4] |= (1<<12);
					display_buff[3] |= (1<<11);
				case 2:
					display_buff[4] |= (1<<11);
					display_buff[3] |= (1<<12);
				case 1:
					display_buff[4] |= (1<<10);
					display_buff[3] |= (1<<13);
				case 0:
					display_buff[4] |= (1<<8);
					display_buff[4] |= (1<<9);
					display_buff[3] |= (1<<14);
					display_buff[3] |= (1<<15);
					break;
				default:
					break;
			}
		}
	}
	else{
Show_Speed_Pro:
		speed_tmp = instrument_info.engine_speed/500;
		if((instrument_info.engine_speed%500) > 450){
			speed_tmp++;
		}
		// speed_tmp = speed_tmp/5;
		// Uart0_PutChar(instrument_info.engine_speed>>8);
		// Uart0_PutChar(instrument_info.engine_speed);

		if(speed_tmp != show_speed_var){
			if(speed_tmp > show_speed_var){
				show_speed_var++;
			}
			else{
				show_speed_var--;
			}
		}
		// Uart0_PutChar(show_speed_var);
		Speed_Show_Pro(show_speed_var);
	}

	
	display_buff[1] &= ~(1<<10);	//手机图标显示清除
	
	display_buff[15] &= ~0xFFFF;
	display_buff[14] &= ~0x3FFF;	//时钟显示清除
	
	display_buff[2] &= ~(1<<14);
	display_buff[0] &= ~(1<<6);	//mph
	display_buff[2] &= ~(1<<15);
	display_buff[0] &= ~(1<<7);	//km/h
	display_buff[11] &= ~(3<<14);	//miles
	display_buff[10] &= ~(1<<8);		//km

	display_buff[0] &= ~(3<<4);
	display_buff[2] &= ~0x3FFF;		//时速显示清除

	display_buff[14] &= ~0xC000;
	display_buff[13] &= ~0xFFFF;
	display_buff[12] &= ~0x3FFF;
	display_buff[11] &= ~0x3FFF;
	display_buff[10] &= ~0x00FF;	//里程显示清除

	if(phone_en){
		display_buff[1] |= (1<<10);

		switch(phone_len-1)
		{
			case 10:
			{
				if(Phone_buff[10] >= 10){
					display_buff[10] |= SHOW_NUM[10];
				}
				else{
					display_buff[10] |= SHOW_NUM[Phone_buff[10]];
				}
			}
			case 9:
			{
				if(Phone_buff[9] >= 10){
					display_buff[11] |= SHOW_NUM[10]<<7;
				}
				else{
					display_buff[11] |= SHOW_NUM[Phone_buff[9]]<<7;
				}
			}
			case 8:
			{
				if(Phone_buff[8] >= 10){
					display_buff[11] |= SHOW_NUM[10];
				}
				else{
					display_buff[11] |= SHOW_NUM[Phone_buff[8]];
				}
			}
			case 7:
			{
				if(Phone_buff[7] >= 10){
					display_buff[12] |= SHOW_NUM[10]<<7;
				}
				else{
					display_buff[12] |= SHOW_NUM[Phone_buff[7]]<<7;
				}
			}
			case 6:
			{
				if(Phone_buff[6] >= 10){
					display_buff[12] |= SHOW_NUM[10];
				}
				else{
					display_buff[12] |= SHOW_NUM[Phone_buff[6]];
				}
			}
			case 5:
			{
				if(Phone_buff[5] >= 10){
					display_buff[13] |= SHOW_NUM[10]<<7;
				}
				else{
					display_buff[13] |= SHOW_NUM[Phone_buff[5]]<<7;
				}
			}
			case 4:
			{
				if(Phone_buff[4] >= 10){
					display_buff[13] |= SHOW_NUM[10];
				}
				else{
					display_buff[13] |= SHOW_NUM[Phone_buff[4]];
				}
			}
			case 3:
			{
				if(Phone_buff[3] >= 10){
					display_buff[14] |= SHOW_NUM[10]<<7;
				}
				else{
					display_buff[14] |= SHOW_NUM[Phone_buff[3]]<<7;
				}
			}
			case 2:
			{
				if(Phone_buff[2] >= 10){
					display_buff[14] |= SHOW_NUM[10];
				}
				else{
					display_buff[14] |= SHOW_NUM[Phone_buff[2]];
				}
			}
			case 1:
			{
				if(Phone_buff[1] >= 10){
					display_buff[15] |= SHOW_NUM[10]<<7;
				}
				else{
					display_buff[15] |= SHOW_NUM[Phone_buff[1]]<<7;
				}
			}
			case 0:
			{
				if(Phone_buff[0] >= 10){
					display_buff[15] |= SHOW_NUM[10];
				}
				else{
					display_buff[15] |= SHOW_NUM[Phone_buff[0]];
				}
			}
			default:
			break;
		}

		//时速显示
		if(km_en){
			Show_KMH_Pro();
		}
		else{
			Show_MPH_Pro();
		}
		return;
	}
	
	//*********************************************
	//时钟显示
	// display_buff[15] &= ~0xFFFF;
	// display_buff[14] &= ~0x3FFF;
	
	display_buff[15] |= (1<<15);
	if(chg_time_en){
		if(shake_show_en){
			display_buff[15] |= (1<<14);		//冒号
		}
		if(chg_hour_en){
			if(chg_shake_show_en){
				show_tmp = hour_var/10;
				if(show_tmp == 0){
					display_buff[15] |= SHOW_NUM[0];		//0
				}
				else if(show_tmp == 1){
					display_buff[15] |= SHOW_NUM[1];		//1
				}
				else{
					display_buff[15] |= SHOW_NUM[2];		//2
				}
				show_tmp = hour_var%10;
				display_buff[15] |= SHOW_NUM[show_tmp]<<7;
			}
			show_tmp = min_var/10;
			display_buff[14] |= SHOW_NUM[show_tmp];
			show_tmp = min_var%10;
			display_buff[14] |= SHOW_NUM[show_tmp]<<7;
		}
		else{
			if(chg_shake_show_en){
				show_tmp = min_var/10;
				display_buff[14] |= SHOW_NUM[show_tmp];
				show_tmp = min_var%10;
				display_buff[14] |= SHOW_NUM[show_tmp]<<7;
			}
			show_tmp = hour_var/10;
			if(show_tmp == 0){
				display_buff[15] |= SHOW_NUM[0];		//0
			}
			else if(show_tmp == 1){
				display_buff[15] |= SHOW_NUM[1];		//1
			}
			else{
				display_buff[15] |= SHOW_NUM[2];		//2
			}
			show_tmp = hour_var%10;
			display_buff[15] |= SHOW_NUM[show_tmp]<<7;
		}
	}
	else{
		show_tmp = hour_var/10;
		if(show_tmp == 0){
			display_buff[15] |= SHOW_NUM[0];		//0
		}
		else if(show_tmp == 1){
			display_buff[15] |= SHOW_NUM[1];		//1
		}
		else{
			display_buff[15] |= SHOW_NUM[2];		//2
		}
		show_tmp = hour_var%10;
		display_buff[15] |= SHOW_NUM[show_tmp]<<7;
		
		show_tmp = min_var/10;
		display_buff[14] |= SHOW_NUM[show_tmp];
		show_tmp = min_var%10;
		display_buff[14] |= SHOW_NUM[show_tmp]<<7;
		
		if(shake_show_en){		//冒号
			display_buff[15] |= (1<<14);
		}
		else{
			display_buff[15] &= ~(1<<14);
		}
	}

	//*********************************************
	//公里数
	// display_buff[2] &= ~(1<<14);
	// display_buff[0] &= ~(1<<6);	//mph
	// display_buff[2] &= ~(1<<15);
	// display_buff[0] &= ~(1<<7);	//km/h
	// display_buff[11] &= ~(3<<14);	//miles
	// display_buff[10] &= ~(1<<8);		//km

	// display_buff[0] &= ~(3<<4);
	// display_buff[2] &= ~0x3FFF;

	// display_buff[14] &= ~0xC000;
	// display_buff[13] &= ~0xFFFF;
	// display_buff[12] &= ~0x3FFF;
	// display_buff[11] &= ~0x3FFF;
	// display_buff[10] &= ~0x00FF;

	if(chg_show_en){
		if(chg_shake_show_en){
			if(km_en){
				display_buff[2] &= ~(1<<14);
				display_buff[0] &= ~(1<<6);		//mph
				display_buff[2] |= (1<<15);
				display_buff[0] |= (1<<7);		//km/h
				display_buff[11] &= ~(3<<14);	//miles
				display_buff[10] |= (1<<8);		//km
			}
			else{
				display_buff[2] |= (1<<14);
				display_buff[0] |= (1<<6);		//mph
				display_buff[2] &= ~(1<<15);
				display_buff[0] &= ~(1<<7);		//km/h
				display_buff[11] |= (3<<14);	//miles
				display_buff[10] &= ~(1<<8);	//km
			}
		}
		// return;
	}
	else if(km_en){
		display_buff[2] |= (1<<15);
		display_buff[0] |= (1<<7);		//km/h
		display_buff[10] |= (1<<8);		//km
	}
	else{
		display_buff[2] |= (1<<14);
		display_buff[0] |= (1<<6);	//mph
		display_buff[11] |= (3<<14);	//miles
	}

	
	if(km_en){						//公里每小时/公里数显示
		// display_buff[2] |= (1<<15);
		// display_buff[0] |= (1<<7);		//km/h
		// display_buff[10] |= (1<<8);		//km
		
		// //公里每小时显示
		// if(instrument_info.speed_with_km != kmh_tmp){
		// 	if(instrument_info.speed_with_km > kmh_tmp){
		// 		kmh_tmp++;
		// 	}
		// 	else{
		// 		kmh_tmp--;
		// 	}
		// }
		
		// u8_tmp = kmh_tmp;
		// show_tmp = u8_tmp%10;
		// display_buff[2] |= SHOW_NUM[show_tmp]<<7;
		// u8_tmp /= 10;
		// show_tmp = u8_tmp%10;
		// display_buff[2] |= SHOW_NUM[show_tmp];
		// u8_tmp /= 10;
		// if(u8_tmp){
		// 	display_buff[0] &= ~(3<<4);
		// }
		// else if((display_buff[2]&0x007F) == SHOW_0){
		// 	display_buff[2] &= 0xFF80;
		// }
		Show_KMH_Pro();
		
		//公里显示
		if(odo_en){			//ODO显示,总里程
			display_buff[13] |= (3<<14);		//ODO

			km_mile_tmp = instrument_info.total_mileage_with_km;
		}
		else{			//TRIP显示,小计里程
			display_buff[14] |= (3<<14);		//TRLP
			display_buff[10] |= (1<<7);		//小数点

			km_mile_tmp = instrument_info.subtotal_mileage_with_km;
		}
	}
	else{							//英里每小时/英里数显示
		// display_buff[2] |= (1<<14);
		// display_buff[0] |= (1<<6);	//mph
		// display_buff[11] |= (3<<14);	//miles
		
		// //英里每小时显示
		// if(instrument_info.speed_with_mile != mph_tmp){
		// 	if(instrument_info.speed_with_mile > mph_tmp){
		// 		mph_tmp++;
		// 	}
		// 	else{
		// 		mph_tmp--;
		// 	}
		// }
		
		// u8_tmp = kmh_tmp;
		// show_tmp = u8_tmp%10;
		// display_buff[2] |= SHOW_NUM[show_tmp]<<7;
		// u8_tmp /= 10;
		// show_tmp = u8_tmp%10;
		// display_buff[2] |= SHOW_NUM[show_tmp];
		// u8_tmp /= 10;
		// if(u8_tmp){
		// 	display_buff[0] &= ~(3<<4);
		// }
		// else if((display_buff[2]&0x007F) == SHOW_0){
		// 	display_buff[2] &= 0xFF80;
		// }
		
		Show_MPH_Pro();
		
		//英里显示
		if(odo_en){			//ODO显示,总里程
			display_buff[13] |= (3<<14);		//ODO
			
			km_mile_tmp = instrument_info.total_mileage_with_mile;
	//         Uart0_PutChar(instrument_info.total_mileage_with_mile >> 24);
    //         Uart0_PutChar(instrument_info.total_mileage_with_mile >> 16);
    //         Uart0_PutChar(instrument_info.total_mileage_with_mile >> 8);
    //         Uart0_PutChar(instrument_info.total_mileage_with_mile);
	// Uart0_PutChar(km_mile_tmp >> 24);
    // Uart0_PutChar(km_mile_tmp >> 16);
    // Uart0_PutChar(km_mile_tmp >> 8);
    // Uart0_PutChar(km_mile_tmp);
		}
		else{			//TRIP显示,小计里程
			display_buff[14] |= (3<<14);		//TRLP
			display_buff[10] |= (1<<7);		//小数点
			
			km_mile_tmp = instrument_info.subtotal_mileage_with_mile;
		}
	}
	// km_mile_tmp = 12345678;
	//         Uart0_PutChar(instrument_info.total_mileage_with_km >> 24);
    //         Uart0_PutChar(instrument_info.total_mileage_with_km >> 16);
    //         Uart0_PutChar(instrument_info.total_mileage_with_km >> 8);
    //         Uart0_PutChar(instrument_info.total_mileage_with_km);
	// Uart0_PutChar(km_mile_tmp >> 24);
    // Uart0_PutChar(km_mile_tmp >> 16);
    // Uart0_PutChar(km_mile_tmp >> 8);
    // Uart0_PutChar(km_mile_tmp);

	show_tmp = km_mile_tmp%10;
	display_buff[10] |= SHOW_NUM[show_tmp];
	km_mile_tmp /= 10;
	show_tmp = km_mile_tmp%10;
	display_buff[11] |= SHOW_NUM[show_tmp]<<7;
	km_mile_tmp /= 10;
	show_tmp = km_mile_tmp%10;
	display_buff[11] |= SHOW_NUM[show_tmp];
	km_mile_tmp /= 10;
	show_tmp = km_mile_tmp%10;
	display_buff[12] |= SHOW_NUM[show_tmp]<<7;
	km_mile_tmp /= 10;
	show_tmp = km_mile_tmp%10;
	display_buff[12] |= SHOW_NUM[show_tmp];
	km_mile_tmp /= 10;
	show_tmp = km_mile_tmp%10;
	display_buff[13] |= SHOW_NUM[show_tmp]<<7;
	km_mile_tmp /= 10;
	show_tmp = km_mile_tmp%10;
	display_buff[13] |= SHOW_NUM[show_tmp];

	if((display_buff[13]&0x007F) == SHOW_0){
		display_buff[13] &= ~0x007F;

		show_tmp = display_buff[13]>>7;
		if((show_tmp&0x007F) == SHOW_0){
			display_buff[13] &= ~0x3F80;

			if((display_buff[12]&0x007F) == SHOW_0){
				display_buff[12] &= ~0x007F;

				show_tmp = display_buff[12]>>7;
				if((show_tmp&0x007F) == SHOW_0){
					display_buff[12] &= ~0x3F80;

					if((display_buff[11]&0x007F) == SHOW_0){
						display_buff[11] &= ~0x007F;

						if(odo_en){
							show_tmp = display_buff[11]>>7;
							if((show_tmp&0x007F) == SHOW_0){
								display_buff[11] &= ~0x3F80;
							}
						}
					}
				}
			}
		}
	}
	*/
}

//******************************************
#if 0
void Set_Show_Num(unsigned char tmp)
{
	// if(tmp >= 2){
		tmp -= 2;
	// }
	// else{
	// 	tmp = 0;
	// }
	
	if(tmp >= 9) tmp = 9;
	display_buff[0] |= (3<<4);
	display_buff[2] |= SHOW_NUM[tmp];
	display_buff[2] |= SHOW_NUM[tmp]<<7;

	display_buff[1] |= SHOW_NUM[tmp];

	display_buff[15] |= SHOW_NUM[tmp];
	display_buff[15] |= SHOW_NUM[tmp]<<7;
	display_buff[14] |= SHOW_NUM[tmp];
	display_buff[14] |= SHOW_NUM[tmp]<<7;
	display_buff[13] |= SHOW_NUM[tmp];
	display_buff[13] |= SHOW_NUM[tmp]<<7;
	display_buff[12] |= SHOW_NUM[tmp];
	display_buff[12] |= SHOW_NUM[tmp]<<7;
	display_buff[11] |= SHOW_NUM[tmp];
	display_buff[11] |= SHOW_NUM[tmp]<<7;
	display_buff[10] |= SHOW_NUM[tmp];
	
	switch(tmp)
	{
		default:
		case 3:
			display_buff[6] |= (3<<3);
			display_buff[9] |= (3<<8);
		case 2:
			display_buff[6] |= (3<<1);
			display_buff[9] |= (3<<6);
		case 1:
			display_buff[6] |= (1<<0);
			display_buff[7] |= (1<<15);
			display_buff[9] |= (3<<4);
		case 0:
			display_buff[7] |= (3<<13);
			display_buff[9] |= (3<<2);
		break;
	}
}
#endif

//******************************************
void Rpm_Show_Pro(unsigned char tmp)
{
    display_buff[11] &= ~(0x01FF << 7);
    display_buff[10] &= ~0xFFFF;
    display_buff[9] &= ~(0x07 << 0);
    switch (tmp) {
    default:
    case 28:
    case 27:
        display_buff[11] |= (1 << 7);
    case 26:
        display_buff[11] |= (1 << 8);
    case 25:
        display_buff[11] |= (1 << 9);
    case 24:
        display_buff[11] |= (1 << 10);
    case 23:
        display_buff[11] |= (1 << 11);
    case 22:
        display_buff[11] |= (1 << 12);
    case 21:
        display_buff[11] |= (1 << 13);
    case 20:
        display_buff[11] |= (1 << 14);
    case 19:
        display_buff[11] |= (1 << 15);
    case 18:
        display_buff[10] |= (1 << 0);
    case 17:
        display_buff[10] |= (1 << 1);
    case 16:
        display_buff[10] |= (1 << 2);
    case 15:
        display_buff[10] |= (1 << 3);
    case 14:
        display_buff[10] |= (1 << 4);
    case 13:
        display_buff[10] |= (1 << 5);
    case 12:
        display_buff[10] |= (1 << 6);
    case 11:
        display_buff[10] |= (1 << 7);
    case 10:
        display_buff[10] |= (1 << 8);
    case 9:
        display_buff[10] |= (1 << 9);
    case 8:
        display_buff[10] |= (1 << 10);
    case 7:
        display_buff[10] |= (1 << 11);
    case 6:
        display_buff[10] |= (1 << 12);
    case 5:
        display_buff[10] |= (1 << 13);
    case 4:
        display_buff[10] |= (1 << 14);
    case 3:
        display_buff[10] |= (1 << 15);
    case 2:
        display_buff[9] |= (1 << 0);
    case 1:
        display_buff[9] |= (1 << 1);
    case 0:
        display_buff[9] |= (1 << 2);
        break;
    }
}

//******************************************
void Speed_Show_Pro(unsigned char tmp)
{
    display_buff[9] &= ~(0xFF << 8);
    display_buff[8] &= ~0xFFFF;
    display_buff[7] &= ~(0x0F); //时速显示
    switch (tmp) {
    default:
    case 28:
        display_buff[9] |= (1 << 8);
    case 27:
        display_buff[9] |= (1 << 9);
    case 26:
        display_buff[9] |= (1 << 10);
    case 25:
        display_buff[9] |= (1 << 11);
    case 24:
        display_buff[9] |= (1 << 12);
    case 23:
        display_buff[9] |= (1 << 13);
    case 22:
        display_buff[9] |= (1 << 14);
    case 21:
        display_buff[9] |= (1 << 15);
    case 20:
        display_buff[8] |= (1 << 0);
    case 19:
        display_buff[8] |= (1 << 1);
    case 18:
        display_buff[8] |= (1 << 2);
    case 17:
        display_buff[8] |= (1 << 3);
    case 16:
        display_buff[8] |= (1 << 4);
    case 15:
        display_buff[8] |= (1 << 5);
    case 14:
        display_buff[8] |= (1 << 6);
    case 13:
        display_buff[8] |= (1 << 7);
    case 12:
        display_buff[8] |= (1 << 8);
    case 11:
        display_buff[8] |= (1 << 9);
    case 10:
        display_buff[8] |= (1 << 10);
    case 9:
        display_buff[8] |= (1 << 11);
    case 8:
        display_buff[8] |= (1 << 12);
    case 7:
        display_buff[8] |= (1 << 13);
    case 6:
        display_buff[8] |= (1 << 14);
    case 5:
        display_buff[8] |= (1 << 15);
    case 4:
        display_buff[7] |= (1 << 0);
    case 3:
        display_buff[7] |= (1 << 1);
    case 2:
        display_buff[7] |= (1 << 2);
    case 1:
        display_buff[7] |= (1 << 3);
    case 0:
        break;
    }
}

// void VolPow_Show_Pro(unsigned char tmp)
// {
// 		switch(tmp){
// 			case 11:
// 				display_buff[6] |= (1<<5);
// 				display_buff[9] |= (1<<10);
// 			case 10:
// 				display_buff[6] |= (1<<6);
// 				display_buff[8] |= (1<<4);
// 			case 9:
// 				display_buff[6] |= (1<<8);
// 				display_buff[8] |= (1<<2);
// 			case 8:
// 				display_buff[6] |= (1<<9);
// 				display_buff[8] |= (1<<1);
// 			case 7:
// 				display_buff[6] |= (1<<10);
// 				display_buff[8] |= (1<<0);
// 			case 6:
// 				display_buff[6] |= (1<<11);
// 				display_buff[9] |= (1<<15);
// 			case 5:
// 				display_buff[6] |= (1<<12);
// 				display_buff[9] |= (1<<14);
// 			case 4:
// 				display_buff[6] |= (1<<13);
// 				display_buff[9] |= (1<<13);
// 			case 3:
// 				display_buff[6] |= (1<<14);
// 				display_buff[9] |= (1<<12);
// 			case 2:
// 				display_buff[6] |= (1<<15);
// 				display_buff[9] |= (1<<11);
// 			case 1:
// 				display_buff[7] |= (1<<12);
// 				display_buff[9] |= (1<<1);
// 			case 0:
// 				display_buff[7] |= (1<<11);
// 				display_buff[9] |= (1<<0);
// 				break;
// 			default:
// 				break;
// 		}
// }

void Show_KMH_Pro()
{
    // Uart0_PutChar(instrument_info.speed_with_km>>8);
    // Uart0_PutChar(instrument_info.speed_with_km);

    //公里每小时显示
    if (instrument_info.speed_with_km == 0) {
        if (kmh_tmp) {
            kmh_tmp--;
        }
    } else {
        // tmp_c = instrument_info.speed_with_km*100+instrument_info.speed_with_km*5;

        if (instrument_info.speed_with_km <= 250) {
            tmp_c = instrument_info.speed_with_km * 1.07;
        } else {
            tmp_c = instrument_info.speed_with_km;
        }

        if (tmp_c != kmh_tmp) {
            if (slow_en == 0) {
                if (tmp_c > kmh_tmp) {
                    kmh_tmp++;
                } else {
                    kmh_tmp--;
                }
            } else {
                if (tmp_c > kmh_tmp) {
                    tmp_a = tmp_c - kmh_tmp;
                } else {
                    tmp_a = kmh_tmp - tmp_c;
                }

                tmp_b++;
                if (tmp_a > 5) {
                    slow_en = 0;
                    tmp_b = 0;
                } else if (tmp_a > 2) {
                    if (tmp_b > 60) {
                        tmp_b = 0;
                        if (tmp_c > kmh_tmp) {
                            kmh_tmp++;
                        } else if (tmp_c < kmh_tmp) {
                            kmh_tmp--;
                        }
                    }
                } else {
                    if (tmp_b > 100) {
                        tmp_b = 0;
                        if (tmp_c > kmh_tmp) {
                            kmh_tmp++;
                        } else if (tmp_c < kmh_tmp) {
                            kmh_tmp--;
                        }
                    }
                }
            }
        } else {
            slow_en = 1;
            tmp_b = 0;
        }
    }

#if 0
		kmh_tmp = 123;
#endif

    // u8_tmp = kmh_tmp;
    // show_tmp = u8_tmp%10;
    // display_buff[5] |= SHOW_NUM[show_tmp]<<10;
    // u16_tmp = SHOW_NUM[show_tmp]<<9;
    // if(u16_tmp&0x8000){
    // 	display_buff[4] |= 0x0001;
    // }
    // u8_tmp /= 10;
    // show_tmp = u8_tmp%10;
    // // display_buff[5] |= SHOW_NUM[show_tmp]<<3;
    // u8_tmp /= 10;
    // if(u8_tmp){
    // 	display_buff[5] |= (1<<2);
    // 	display_buff[4] |= (1<<3);
    // 	display_buff[5] |= SHOW_NUM[show_tmp]<<3;
    // }
    // else if(show_tmp){
    // 	display_buff[5] |= SHOW_NUM[show_tmp]<<3;
    // }
}

void Show_MPH_Pro()
{
    //英里每小时显示
    if (instrument_info.speed_with_mile == 0) {
        if (mph_tmp) {
            mph_tmp--;
        }
    } else {
        // tmp_c = instrument_info.speed_with_mile;
        if (instrument_info.speed_with_mile <= 250) {
            tmp_c = instrument_info.speed_with_mile * 1.07;
        } else {
            tmp_c = instrument_info.speed_with_mile;
        }

        if (tmp_c != mph_tmp) {
            if (slow_en == 0) {
                if (tmp_c > mph_tmp) {
                    mph_tmp++;
                } else {
                    mph_tmp--;
                }
            } else {
                if (tmp_c > mph_tmp) {
                    tmp_a = tmp_c - mph_tmp;
                } else {
                    tmp_a = mph_tmp - tmp_c;
                }

                tmp_b++;
                if (tmp_a > 5) {
                    slow_en = 0;
                    tmp_b = 0;
                } else if (tmp_a > 2) {
                    if (tmp_b > 60) {
                        tmp_b = 0;
                        if (tmp_c > mph_tmp) {
                            mph_tmp++;
                        } else if (tmp_c < mph_tmp) {
                            mph_tmp--;
                        }
                    }
                } else {
                    if (tmp_b > 100) {
                        tmp_b = 0;
                        if (tmp_c > mph_tmp) {
                            mph_tmp++;
                        } else if (tmp_c < mph_tmp) {
                            mph_tmp--;
                        }
                    }
                }
            }
        } else {
            slow_en = 1;
            tmp_b = 0;
        }
    }

#if 0
		mph_tmp = 123;
#endif

    // u8_tmp = mph_tmp;
    // show_tmp = u8_tmp%10;
    // display_buff[5] |= SHOW_NUM[show_tmp]<<10;
    // u16_tmp = SHOW_NUM[show_tmp]<<9;
    // if(u16_tmp&0x8000){
    // 	display_buff[4] |= 0x0001;
    // }
    // u8_tmp /= 10;
    // show_tmp = u8_tmp%10;
    // // display_buff[5] |= SHOW_NUM[show_tmp]<<3;
    // u8_tmp /= 10;
    // if(u8_tmp){
    // 	display_buff[5] |= (1<<2);
    // 	display_buff[4] |= (1<<3);
    // 	display_buff[5] |= SHOW_NUM[show_tmp]<<3;
    // }
    // else if(show_tmp){
    // 	display_buff[5] |= SHOW_NUM[show_tmp]<<3;
    // }
}

///////////////////////////////////////////////////////////////////////
void Set_Show_MPH_KMH(unsigned char data_tmp)
{
    u8_tmp = data_tmp;
    show_tmp = u8_tmp % 10;
    display_buff[5] |= SHOW_NUM[show_tmp] << 10;
    u16_tmp = SHOW_NUM[show_tmp] << 9;
    if (u16_tmp & 0x8000) {
        display_buff[4] |= 0x0001;
    }
    u8_tmp /= 10;
    show_tmp = u8_tmp % 10;
    // display_buff[5] |= SHOW_NUM[show_tmp]<<3;
    u8_tmp /= 10;
    if (u8_tmp) {
        display_buff[5] |= (1 << 2);
        display_buff[4] |= (1 << 3);
        display_buff[5] |= SHOW_NUM[show_tmp] << 3;
    } else if (show_tmp) {
        display_buff[5] |= SHOW_NUM[show_tmp] << 3;
    }
}

///////////////////////////////////////////////////////////////////////
void Set_Show_Km_Mile()
{
    show_tmp = km_mile_tmp % 10;
    u8_tmp = SHOW_NUM[show_tmp];
    if (u8_tmp & SEG_A) {
        display_buff[2] |= (1 << 15);
    }
    display_buff[1] |= SHOW_NUM[show_tmp] >> 1;

    km_mile_tmp /= 10;
    if (display_buff[2] & (1 << 14)) {
        show_tmp = km_mile_tmp % 10;
        display_buff[2] |= SHOW_NUM[show_tmp] << 7;
    } else if (km_mile_tmp) {
        show_tmp = km_mile_tmp % 10;
        display_buff[2] |= SHOW_NUM[show_tmp] << 7;
    }

    km_mile_tmp /= 10;
    if (km_mile_tmp) {
        show_tmp = km_mile_tmp % 10;
        display_buff[2] |= SHOW_NUM[show_tmp];
    }
    km_mile_tmp /= 10;
    if (km_mile_tmp) {
        show_tmp = km_mile_tmp % 10;
        display_buff[3] |= SHOW_NUM[show_tmp] << 9;
    }
    km_mile_tmp /= 10;
    if (km_mile_tmp) {
        show_tmp = km_mile_tmp % 10;
        display_buff[3] |= SHOW_NUM[show_tmp] << 2;
    }
    km_mile_tmp /= 10;
    if (km_mile_tmp) {
        show_tmp = km_mile_tmp % 10;
        display_buff[4] |= SHOW_NUM[show_tmp] << 11;
        u8_tmp = SHOW_NUM[show_tmp];
        if (u8_tmp & SEG_F) {
            display_buff[3] |= (1 << 0);
        }
        if (u8_tmp & SEG_G) {
            display_buff[3] |= (1 << 1);
        }
    }
}

///////////////////////////////////////////////////////////////////////
void Scan_Key()
{
    // if(com_jg_chk){
    // 	key_tmp |= 0x80;
    // }
    // else{
    // 	key_tmp &= ~0x80;
    // }
    if (com_wd_chk) {
        key_tmp |= 0x40;
        // Uart0_PutChar(0x40);
    } else {
        key_tmp &= ~0x40;
        // Uart0_PutChar(0x04);
    }
    if (com_xd_chk) {
        key_tmp |= 0x20;
    } else {
        key_tmp &= ~0x20;
    }
    if (com_pd_chk) {
        key_tmp |= 0x10;
        // Uart1_PutChar(0x33);
    } else {
        key_tmp &= ~0x10;
        // Uart1_PutChar(0x44);
    }

    if (key_buf == key_tmp) {
        if (key_buf == key_flag) {
            key_deb = 0;
            return;
        }
        // if(!systimer_flag_is_valid(SYSTIME_FLAG_10MS)) return;
        if (key_deb_en == 0)
            return;
        key_deb_en = 0;
        //5ms
        key_deb++;
        if (key_deb < 6)
            return;
        key_deb = 0;

        key_tmp = key_buf ^ key_flag;
        key_val1 = key_tmp & key_buf;
        key_val2 = key_tmp & key_flag;
        key_flag = key_buf;

        // if(key_val1){
        // 	Uart1_PutChar(0x11);
        // }
        // if(key_val2){
        // 	Uart1_PutChar(0x22);
        // }

        // tk_dly_cnt = 0;
        //--------------------
        if (key_val1 & 0x80) {
            jg_en = 0;
            // Uart0_PutChar(key_val1&0x80);
        }
        if (key_val1 & 0x40) {
            wd_en = 1;
        }
        if (key_val1 & 0x20) {
            xd_en = 0;
            // Uart0_PutChar(key_val1&0x20);
        }
        if (key_val1 & 0x10) {
            pd_en = 0;
            // Uart1_PutChar(0x33);
        }
        if (key_val2 & 0x80) {
            jg_en = 1;
            // Uart0_PutChar(key_val2&0x80);
        }
        if (key_val2 & 0x40) {
            wd_en = 0;
        }
        if (key_val2 & 0x20) {
            xd_en = 1;
            // Uart0_PutChar(key_val2&0x20);
        }
        if (key_val2 & 0x10) {
            pd_en = 1;
            // Uart1_PutChar(0x44);
        }

        //按下
        if (key_val1 & 0x01) {
            longpress_en = 1;
            longpress_cnt = 0;
        }
        // else if(key_val1 & 0x02){
        // 	longpress_en = 1;
        // 	longpress_cnt = 0;
        // }
        // else if(key_val1 & 0x04){
        // 	longpress_en = 1;
        // 	longpress_cnt = 0;
        // }
        // else if(key_val1 & 0x08){
        // 	longpress_en = 1;
        // 	longpress_cnt = 0;
        // }

        //------------------------
        //松手
        if (key_val2 & 0x01) {
            if (longpress_en) {
                longpress_en = 0;
                //短按松手
                if (chg_en) {
                    if (shake_en) {
                        shake_en = 0;
                        chg_en = 0;
                    } else if (chg_stp == 0) {
                        odo_en = ~odo_en;
                        chg_deb = 100; //10s退出调整模式
                    } else if (chg_stp == 1) {
                        km_en = ~km_en;
                        chg_shake_cnt = 0;
                        chg_shake_show_en = 1;
                        chg_deb = 100; //10s退出调整模式
                    } else if (chg_stp == 2) {
                        gear_num++;
                        if (gear_num >= 5)
                            gear_num = 0;
                        chg_deb = 100; //10s退出调整模式
                    }
                }
            } else {
                //长按松手
                if (chg_en) {
                    if (shake_en) {
                        shake_deb = 32; //50ms计时单位
                    }
                }
            }
        }
    } else {
        key_buf = key_tmp;
        key_deb = 0;
    }
}

//======================================================================
void Chk_LongPress_Pro()
{
    if (longpress_en == 0)
        return;

    if (!systimer_flag_is_valid(SYSTIME_FLAG_25MS))
        return;

    longpress_cnt++;
    if (longpress_cnt < 40)
        return;
    longpress_cnt = 0;
    longpress_en = 0;

    if (key_val1 & 0x01) {
        if (chg_en == 0) {
            chg_en = 1;
            shake_en = 1;
            shake_deb = 0;
            chg_stp = 0;
        } else if (chg_stp == 0) {
            if (odo_en) {
                chg_stp = 1;
            } else {
                //清小计里程
                instrument_info.subtotal_mileage_with_km = 0;
                instrument_info.subtotal_mileage_with_mile = 0;
                send_en = 1;
                send_cmd = 0x99;
            }
        } else if (chg_stp == 1) {
            chg_stp = 2;
            powup_en = 1;
            act_tmp = 0;
            chg_init_en = 1;
        } else {
            chg_en = 0; //退出调整模式
            powup_en = 0;
            save_en = 1;
        }
    }

    // if(key_val1&0x08){
    // 	// Uart0_PutChar(0xA5);
    // 	// Uart0_PutChar(0x01);
    // 	// Uart0_PutChar(0x48);
    // 	// tmp_a = ~(0xA5+0x01+0x48);
    // 	// Uart0_PutChar(tmp_a);

    // 	if(odo_en == 0){
    // 		longpress_en = 0;
    // 		//清小计里程
    // 		instrument_info.subtotal_mileage_with_km = 0;
    // 		instrument_info.subtotal_mileage_with_mile = 0;
    // 		send_en = 1;
    // 		send_cmd = 0x99;
    // 		return;
    // 	}

    // 	if(chg_en == 0){
    // 		longpress_en = 0;
    // 		chg_deb = CHG_EXIT_TIME_VAL;
    // 		// chg_show_deb = 0;
    // 		chg_en = 1;
    // 		chg_cnt = 0;
    // 		chg_show_en = 1;
    // 		chg_time_en = 0;
    // 	}
    // 	else{
    // 		chg_deb = CHG_EXIT_TIME_VAL;
    // 		// chg_show_deb = 0;
    // 		chg_cnt++;
    // 		if(chg_cnt == 2){
    // 			longpress_en = 0;
    // 			chg_show_en = 0;
    // 			chg_time_en = 1;
    // 			chg_hour_en = 0;
    // 		}
    // 		else if(chg_cnt == 4){
    // 			longpress_en = 0;
    // 			chg_show_en = 0;
    // 			chg_time_en = 1;
    // 			chg_hour_en = 1;
    // 		}
    // 		else if(chg_cnt == 6){
    // 			longpress_en = 0;
    // 			chg_en = 0;
    // 			chg_deb = 0;
    // 			// chg_show_deb = 0;
    // 			chg_show_en = 0;
    // 			chg_time_en = 0;

    // 			send_en = 1;
    // 			send_cmd = SET_TIMIE_TO_MAIN;
    // 		}
    // 	}
    // 	return;
    // }
    // longpress_en = 0;
}

//******************************************************
//亮度越大,AD值越小
void Deal_AD_Pro()
{
    if (powup_en)
        return;

    if (chk_ad_en == 0)
        return;
    chk_ad_en = 0;
    //5ms
    ADCON |= AST(1); //启动ADC转换
    while (!(ADCON & ADIF))
        ;          //等待ADC转换完成
    ADCON |= ADIF; //清除ADC中断标志
    // AD_Value = ADCDH*256 + ADCDL;						//读取AD值
    // AD_Value >>= 4;

    ad_data_tmp = ADCDH;

    ad_cnt++;
    if (ad_cnt < 16) {
        ad_sum += ad_data_tmp;
        return;
    }
    ad_cnt = 0;
    ad_sum += ad_data_tmp;
    ad_sum >>= 4;

    ad_tmp = ad_sum;
    if (ad_tmp > 230) {
        pwm_var = PWM_20_VAL;
    } else if (ad_tmp > 200) {
        pwm_var = PWM_30_VAL;
    } else if (ad_tmp > 170) {
        pwm_var = PWM_40_VAL;
    } else if (ad_tmp > 140) {
        pwm_var = PWM_50_VAL;
    } else if (ad_tmp > 110) {
        pwm_var = PWM_60_VAL;
    } else if (ad_tmp > 80) {
        pwm_var = PWM_70_VAL;
    } else if (ad_tmp > 50) {
        pwm_var = PWM_80_VAL;
    } else if (ad_tmp > 20) {
        pwm_var = PWM_90_VAL;
    } else {
        pwm_var = PWM_100_VAL;
    }
    ad_sum = 0;
}

//******************************************************
#if 1
void Send_Cmd_Pro()
{
    if (send_en == 0)
        return;
    send_en = 0;

    // if(send_cmd == SET_TIMIE_TO_MAIN){		//校正主机时间
    // show_hour_var = hour_var;
    // show_min_var = min_var;

    // Uart1_PutChar(0xA5);
    // Uart1_PutChar(0x0B);
    // Uart1_PutChar(SET_TIMIE_TO_MAIN);
    // Uart1_PutChar(0x07);
    // Uart1_PutChar(0xD0);
    // Uart1_PutChar(0x01);
    // Uart1_PutChar(0x01);
    // Uart1_PutChar(hour_var);
    // Uart1_PutChar(min_var);
    // Uart1_PutChar(sec_var);
    // tmp_a = 0xA5+0x0B+SET_TIMIE_TO_MAIN+0x07+0xD0+0x01+0x01 + hour_var + min_var + sec_var;
    // Uart1_PutChar(tmp_a);
    // }
    // else if(send_cmd == 0x35){	//询问主机,获取时间
    // Uart1_PutChar(0xA5);
    // Uart1_PutChar(0x04);
    // Uart1_PutChar(0x8F);
    // Uart1_PutChar(0x38);
    // }
    // else if(send_cmd == 0x8B){	//询问主机,获取大计里程
    // 	Uart1_PutChar(0xA5);
    // 	Uart1_PutChar(0x04);
    // 	Uart1_PutChar(0x8B);
    // 	Uart1_PutChar(0x34);
    // }
    // else if(send_cmd == 0x8C){	//询问主机,获取小计里程
    // 	Uart1_PutChar(0xA5);
    // 	Uart1_PutChar(0x04);
    // 	Uart1_PutChar(0x8C);
    // 	Uart1_PutChar(0x35);
    // }
    // else if(send_cmd == 0x99){	//询问主机,清除小计里程
    if (send_cmd == 0x99) { //询问主机,清除小计里程
        uart1_send_byte(0xA5);
        uart1_send_byte(0x04);
        uart1_send_byte(0x99);
        uart1_send_byte(0x42);
    }
}
#endif

//******************************************************
void Save_Pro()
{
    if (save_en == 0)
        return;
    save_en = 0;

    //划分FLASH最后256字节为数据空间，注意：不同型号芯片FLASH容量不同，定义的FLASH_SIZE也不同，应用时要根据实际FLASH容量定义
    PADRD = FLASH_SIZE * 4 - 1;

    Data_Area_Mass_Read(0, BuffTemp,
                        2); //读扇区0 16位ID，16位ID用以识别是否初次上电
    if (BuffTemp[0] != 0xAA || BuffTemp[1] != 0x55) //若ID不相符，表示是初次上电
    {
        Data_Area_Mass_Read(128, BuffTemp,
                            2); //读取16位ID，16位ID用以识别是否初次上电
        if (BuffTemp[0] == 0xAA &&
            BuffTemp[1] == 0x55) //若扇区0 ID不相符，则判断扇区1 ID
        {
            goto READ_SECOND_SECTOR_PARA; //若相符，跳转读取扇区1数据
        }

        //若扇区0、1 ID都不相符， 表示是初次上电
    FIRST_POWER_ON:
        //初次上电，初始化用户数据为 0 ~ 9
        UserDataBuff[0] = 0x00;
        UserDataBuff[1] = 0x00;
        UserDataBuff[2] = 0x02;
        UserDataBuff[3] = 0x03;
        UserDataBuff[4] = 0x04;
        UserDataBuff[5] = 0x05;
        UserDataBuff[6] = 0x06;
        UserDataBuff[7] = 0x07;
        UserDataBuff[8] = 0x08;
        UserDataBuff[9] = 0x09;

        BuffTemp[0] = 0xAA; //设置ID为0xAA55
        BuffTemp[1] = 0x55;
        BuffTemp[12] = 0;

        // Uart0_PutChar(0x33);

        for (i = 0; i < 10; i++) {
            BuffTemp[i + 2] = UserDataBuff[i];
            BuffTemp[12] += UserDataBuff[i]; //计算校验码
        }

        //在擦除扇区或写入数据时，如果出现芯片掉电或复位，会导致用户数据丢失，以下采用双扇区存储的方式，可避免数据丢失。
        Data_Area_Sector_Erase(0); //擦除数据扇区
        Data_Area_Mass_Write(0, BuffTemp,
                             13); //用户数据、ID、校验码写入数据空间

        Data_Area_Sector_Erase(1); //擦除数据扇区
        Data_Area_Mass_Write(128, BuffTemp,
                             13); //用户数据、ID、校验码写入数据空间
    } else {
        unsigned char CheckSum;
        Data_Area_Mass_Read(0, BuffTemp, 13); //读取用户数据、ID、校验码
        CheckSum = 0;
        for (i = 0; i < 10; i++) {
            CheckSum += BuffTemp[2 + i]; //计算校验码
        }
        if (CheckSum != BuffTemp[12]) //判断检验码是否相符
        {
        READ_SECOND_SECTOR_PARA:
            //如果校验码不相符，则读取第二扇区数据。
            Data_Area_Mass_Read(128, BuffTemp, 13); //读取用户数据、ID、校验码
            CheckSum = 0;
            for (i = 0; i < 10; i++) {
                CheckSum += BuffTemp[2 + i]; //计算校验码
            }
            if (CheckSum != BuffTemp[12]) //判断检验码是否相符
            {
                goto FIRST_POWER_ON;
            }
        }

        for (i = 0; i < 10; i++) {
            UserDataBuff[i] = BuffTemp[i + 2]; //读出的数据装载到用户数据数组
        }

        if (init_en) {
            init_en = 0;
            odo_en = UserDataBuff[0] & 0x01;
            km_en = UserDataBuff[0] & 0x02;
            gear_num = UserDataBuff[1];
            // Uart0_PutChar(UserDataBuff[0]);
        } else {
            if (odo_en) {
                UserDataBuff[0] |= 1;
            } else {
                UserDataBuff[0] &= ~0x01;
            }
            if (km_en) {
                UserDataBuff[0] |= 2;
            } else {
                UserDataBuff[0] &= ~0x02;
            }
            UserDataBuff[1] = gear_num;
            // Uart0_PutChar(UserDataBuff[0]);
        }

        // for(i = 0; i < 10; i++)
        // {
        // 	UserDataBuff[i]++;							//用户数据累加
        // }

        BuffTemp[0] = 0xAA;
        BuffTemp[1] = 0x55;
        BuffTemp[12] = 0;
        for (i = 0; i < 10; i++) {
            BuffTemp[i + 2] = UserDataBuff[i];
            BuffTemp[12] += UserDataBuff[i];
        }

        Data_Area_Sector_Erase(0); //擦除数据扇区
        Data_Area_Mass_Write(0, BuffTemp,
                             13); //用户数据、ID、校验码写入数据空间

        Data_Area_Sector_Erase(1); //擦除数据扇区
        Data_Area_Mass_Write(128, BuffTemp,
                             13); //用户数据、ID、校验码写入数据空间
    }
}

// #endif
