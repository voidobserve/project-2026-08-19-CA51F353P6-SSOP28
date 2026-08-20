#ifndef AIP3368H_APP_H
#define AIP3368H_APP_H

#if 0
// display_buff:缓存, aip3368h依靠移位锁存填充点灯寄存器，所以最后发送的2个字节(u16)才是第一个芯片的16个LED控制，并且高位->低位 == OUT0->OUT15

// 速度SP ------------------------------------------------
#define LEDCOM_SP_LV01 (9)
#define LEDCOM_SP_LV02 (9)
#define LEDCOM_SP_LV03 (9)
#define LEDCOM_SP_LV04 (9)
#define LEDCOM_SP_LV05 (9)
#define LEDCOM_SP_LV06 (9)
#define LEDCOM_SP_LV07 (9)
#define LEDCOM_SP_LV08 (9)
#define LEDCOM_SP_LV09 (9)
#define LEDCOM_SP_LV10 (9)
#define LEDCOM_SP_LV11 (9)
#define LEDCOM_SP_LV12 (9)
#define LEDCOM_SP_LV13 (9)
#define LEDCOM_SP_LV14 (9)
#define LEDCOM_SP_LV15 (9)
#define LEDCOM_SP_LV16 (9)

#define LEDSEG_SP_LV01 (0x8000)
#define LEDSEG_SP_LV02 (0x4000)
#define LEDSEG_SP_LV03 (0x2000)
#define LEDSEG_SP_LV04 (0x1000)
#define LEDSEG_SP_LV05 (0x0800)
#define LEDSEG_SP_LV06 (0x0400)
#define LEDSEG_SP_LV07 (0x0200)
#define LEDSEG_SP_LV08 (0x0100)
#define LEDSEG_SP_LV09 (0x0080)
#define LEDSEG_SP_LV10 (0x0040)
#define LEDSEG_SP_LV11 (0x0020)
#define LEDSEG_SP_LV12 (0x0010)
#define LEDSEG_SP_LV13 (0x0008)
#define LEDSEG_SP_LV14 (0x0004)
#define LEDSEG_SP_LV15 (0x0002)
#define LEDSEG_SP_LV16 (0x0001)

void ledseg_clear_speed_level(void);
void ledseg_show_speed_level(u8 level);
void ledseg_show_test(void);
#endif

#endif
