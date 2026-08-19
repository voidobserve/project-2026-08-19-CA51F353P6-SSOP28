#include "user_config.h"

#if 1 //AIP3368H_MODULE

#define DIO P05
#define DCK P06
#define LAT P07
// #define PDM                     P34
#define DIO_R P34

xdata u16 display_buff[AIP3368H_DEV_NUM_L];
xdata u16 display_buff_r[AIP3368H_DEV_NUM_R];

// #define aip3368h_delay()  do { Dly_us(1); } while(0)
#define aip3368h_delay() // 延时函数，根据需要决定使用，目前测试48Mhz主频无定时中断不需要延时也能正常点亮
// static void aip3368h_delay(void)
// {
//     volatile u16 count = 1;
//     delay(count);
// }

//===================================================================
static void aip3368h_module_send_data_to_one_dev(u16 dat)
{
    u16 i;
    // PDM = 1;

    for (i = 0; i < 16; i++) {
        DIO = dat & 0x8000 ? 1 : 0;

        aip3368h_delay();
        DCK = 1;
        aip3368h_delay();
        dat <<= 1;
        DCK = 0;
        aip3368h_delay();
    }
    // PDM = 0;
}

static void aip3368h_module_send_data_to_all_dev(u16 *buff, u8 len)
{
    u8 i;

    // EA = 0;
    // 开始
    DCK = 0;
    LAT = 0;
    // PDM = 1;
    aip3368h_delay();

    // 一帧完整数据
    for (i = 0; i < len; i++) {
        aip3368h_module_send_data_to_one_dev(buff[i]);
    }

    // 结束
    // PDM = 1;
    LAT = 1;
    aip3368h_delay();
    LAT = 0;
    // PDM = 0;
    aip3368h_delay();
    DIO = 0;

    // EA = 1;
}

//===================================================================
static void aip3368h_module_send_data_to_one_dev_r(u16 dat)
{
    u16 i;
    // PDM = 1;

    for (i = 0; i < 16; i++) {
        DIO_R = dat & 0x8000 ? 1 : 0;

        aip3368h_delay();
        DCK = 1;
        aip3368h_delay();
        dat <<= 1;
        DCK = 0;
        aip3368h_delay();
    }
    // PDM = 0;
}

static void aip3368h_module_send_data_to_all_dev_r(u16 *buff, u8 len)
{
    u8 i;

    // EA = 0;
    // 开始
    DCK = 0;
    LAT = 0;
    // PDM = 1;
    aip3368h_delay();

    // 一帧完整数据
    for (i = 0; i < len; i++) {
        aip3368h_module_send_data_to_one_dev_r(buff[i]);
    }

    // 结束
    // PDM = 1;
    LAT = 1;
    aip3368h_delay();
    LAT = 0;
    // PDM = 0;
    aip3368h_delay();
    DIO_R = 0;

    // EA = 1;
}

#define AIP3368H_FLASH_TEST_ENABLE 0
void aip3368h_module_display(void)
{
    // if (!systimer_flag_is_valid(SYSTIME_FLAG_50MS))
    //     return;

#if AIP3368H_FLASH_TEST_ENABLE
    // 闪烁测试
    if (display_buff[0] == 0x0000)
        memset(display_buff, 0xFF, sizeof(display_buff));
    else
        memset(display_buff, 0x00, sizeof(display_buff));
#endif

    aip3368h_module_send_data_to_all_dev(display_buff, AIP3368H_DEV_NUM_L);
    aip3368h_module_send_data_to_all_dev_r(display_buff_r, AIP3368H_DEV_NUM_R);
}

void aip3368h_module_clear(void)
{
    memset(display_buff, 0x00, sizeof(display_buff));
    aip3368h_module_send_data_to_all_dev(display_buff, AIP3368H_DEV_NUM);
}

void aip3368h_module_init(void)
{
    // memset(display_buff, 0xFF, sizeof(display_buff));
    for (act_tmp = 0; act_tmp < AIP3368H_DEV_NUM_L; act_tmp++) {
        display_buff[act_tmp] = 0x0000;
        // display_buff[act_tmp]=act_tmp;
    }
    for (act_tmp = 0; act_tmp < AIP3368H_DEV_NUM_R; act_tmp++) {
        display_buff_r[act_tmp] = 0x0000;
        // display_buff_r[act_tmp]=act_tmp;
    }

    // memset(display_buff, 0xFF, sizeof(display_buff));
    // memset(display_buff_r, 0xFF, sizeof(display_buff_r));

    GPIO_Init(P05F, OUTPUT);
    GPIO_Init(P06F, OUTPUT);
    GPIO_Init(P07F, OUTPUT);
    GPIO_Init(P34F, OUTPUT);
    DIO = 0;
    DCK = 0;
    LAT = 0;
    // PDM = 1;
    aip3368h_module_send_data_to_all_dev(display_buff, AIP3368H_DEV_NUM_L);
    aip3368h_module_send_data_to_all_dev_r(display_buff_r, AIP3368H_DEV_NUM_R);
}

void aip3368h_module_uninit(void)
{
    aip3368h_module_clear(); // 清屏
    GPIO_Init(P05F, 0);
    GPIO_Init(P06F, 0);
    GPIO_Init(P07F, 0);
    GPIO_Init(P34F, 0);
}

#endif
