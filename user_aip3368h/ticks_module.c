#include "user_config.h"

#define INT_TIME			    1000     //定时时间，单位为us
#define	TH_VAL                  (unsigned char)((0x10000 - (INT_TIME * (FOSC / 1000)) / 12000) >> 8)
#define	TL_VAL                  (unsigned char)(0x10000 - (INT_TIME * (FOSC / 1000)) / 12000)

static volatile u8 tick_timer_flag = 0;  // bit0:5ms, bit1:10ms, bit2:30ms, bit3:50ms, bit4:100ms, bit5:200ms, bit6:500ms, bit7:1000ms
static volatile u8 tick_timer_flag_tmp = 0;

static volatile u16 tick_1ms_count = 0;
static volatile u8 x25ms_cnt = 0;
static volatile u8 x100ms_cnt = 0;

void TIMER1_ISR(void) interrupt 3   // 每1000us中断一次
{   // 周期中断
	TH1 = TH_VAL;
    TL1 = TL_VAL;

    if(pwm_var != pwm_tmp){
        if(pwm_var > pwm_tmp){
            pwm_tmp += 2;
            if(pwm_var < pwm_tmp){
                pwm_var = pwm_tmp;
            }
        }
        else{
            pwm_tmp -= 2;
            if(pwm_var > pwm_tmp){
                pwm_tmp = pwm_var;
            }
        }
        INDEX = PWM_CH3;
        PWMDUTH = (pwm_tmp>>8);
        PWMDUTL = pwm_tmp;
        PWMUPD = (1<<PWM_CH3);
    }

    tick_1ms_count++;
    if(tick_1ms_count >= 5){
        tick_1ms_count = 0;
        //5ms
        tick_timer_flag |= SYSTIME_FLAG_5MS;
        key_deb_en = 1;
        chk_ad_en = 1;

        x25ms_cnt++;
        if(x25ms_cnt >= 5){
            x25ms_cnt = 0;
            //25ms
            tick_timer_flag |= SYSTIME_FLAG_25MS;

            x100ms_cnt++;
            if((x100ms_cnt&0x01) == 0){
                //50ms
                 tick_timer_flag |= SYSTIME_FLAG_50MS;
                 deal_showdat_en = 1;
            }
            if(x100ms_cnt >= 4){
                x100ms_cnt = 0;
                //100ms
                tick_timer_flag |= SYSTIME_FLAG_100MS;
            }
        }
    }

    // tick_1ms_count++;
    // if ((tick_1ms_count % 5) == 0)
    // {
    //     tick_timer_flag |= SYSTIME_FLAG_5MS;
    //     if ((tick_1ms_count % 10) == 0)
    //     {
    //         tick_timer_flag |= SYSTIME_FLAG_10MS;
    //         key_deb_en = 1;
    //         chk_ad_en = 1;
    //     }
    //     if ((tick_1ms_count % 25) == 0)
    //     {
    //         tick_timer_flag |= SYSTIME_FLAG_25MS;
    //         if ((tick_1ms_count % 50) == 0)
    //         {
    //             tick_timer_flag |= SYSTIME_FLAG_50MS;
    //             // com_test = ~com_test;
    //             deal_showdat_en = 1;
    //         }
    //         if ((tick_1ms_count % 100) == 0)
    //         {
    //             tick_timer_flag |= SYSTIME_FLAG_100MS;
    //             // powup_cnt_en = 1;
    //         }
    //         if ((tick_1ms_count % 200) == 0)
    //         {
    //             tick_timer_flag |= SYSTIME_FLAG_200MS;
    //         }
    //         if ((tick_1ms_count % 500) == 0)
    //         {
    //             tick_timer_flag |= SYSTIME_FLAG_500MS;
    //         }
    //         if ((tick_1ms_count % 1000) == 0)
    //         {
    //             tick_timer_flag |= SYSTIME_FLAG_1000MS;
    //             tick_1ms_count = 0;
    //         }
    //     }
    // }
}

u8 systimer_flag_is_valid(u8 flag)
{
    if (tick_timer_flag_tmp & flag)
        return 1;
    return 0;
}

void systimer_flag_update(void)
{
    bit tr1 = TR1;
    TR1 = 0;  // 关闭定时器1中断，防止tick_timer_flag被中断修改
    tick_timer_flag_tmp = tick_timer_flag;
    tick_timer_flag &= ~SYSTIME_FLAG_ALL;
    TR1 = tr1;  // 恢复定时器1中断关闭前的状态
}

void systimer_set_flag(u8 flag)
{
    tick_timer_flag |= flag;
}

void tick_timer_config(void)
{
	TR1 = 0;                        // 定时器1失能
	ET1 = 0;                        // 定时器1中断失能
    // memset(&sys_timer, 0, sizeof(sys_timer));
    tick_1ms_count = 0;
    tick_timer_flag &= ~SYSTIME_FLAG_ALL;

	TMOD = (TMOD&0xCF)|0x10;        // 模式选择: 定时器1，模式1
	TH1 = TH_VAL;                   // 高8位装初值
	TL1 = TL_VAL;                   // 低8位装初值
	TR1 = 1;                        // 定时器1使能
	ET1 = 1;                        // 定时器1中断使能
//	PT1 = 1;                        // 设置定时器1中断优先级为高优先级
}

void tick_timer_config_close(void)
{
    TR1 = 0;                        // 定时器1停止
    ET1 = 0;                        // 定时器1中断禁止
}

void idle_mode_timer_wakeup_config(void)
{
#define SLEEP_INT_TIME              1000000    //定时时间，单位为us
#define	SLEEP_TH_VAL                (unsigned char)((0x10000 - (SLEEP_INT_TIME * (131000 / 1000)) / 12000) >> 8)
#define	SLEEP_TL_VAL                (unsigned char)(0x10000 - (SLEEP_INT_TIME * (131000 / 1000)) / 12000)
	TR1 = 0;                        // 定时器1失能
	ET1 = 0;                        // 定时器1中断失能

	TMOD = (TMOD&0xCF)|0x10;        // 模式选择: 定时器1，模式1
	TH1 = SLEEP_TH_VAL;             // 高8位装初值
	TL1 = SLEEP_TL_VAL;             // 低8位装初值
	TR1 = 1;                        // 定时器1使能
	ET1 = 1;                        // 定时器1中断使能
//	PT1 = 1;                        // 设置定时器1中断优先级为高优先级
}
