#include "aip3368h_display.h"
#include "aip3368.h"

#include <string.h>
#include <stdio.h>

// static volatile u16 test_buff[8] = {0};

void aip3368h_display_test(void)
{
    // memset(aip3368h_speed_panel_display_buff, 0xFF,
    //        sizeof(aip3368h_speed_panel_display_buff));
    // memset(aip3368h_engine_speed_panel_display_buff, 0xFF,
    //        sizeof(aip3368h_engine_speed_panel_display_buff));

#if 0

    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 0;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 1;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 2;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 3;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 4;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 5;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 6;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 7;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 8;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 9;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 10;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 11;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 12;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 13;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 14;
    aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 15;

    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 0;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 1;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 2;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 3;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 4;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 5;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 6;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 7;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 8;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 9;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 10;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 11;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 12;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 13;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 14;
    aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 15;

    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 0;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 1;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 2;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 3;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 4;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 5;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 6;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 7;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 8;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 9;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 10;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 11;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 12;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 13;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 14;
    aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 15;

    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 0;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 1;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 2;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 3;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 4;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 5;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 6;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 7;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 8;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 9;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 10;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 11;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 12;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 13;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 14;
    aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 15;

    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 0;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 1;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 2;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 3;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 4;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 5;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 6;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 7;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 8;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 9;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 10;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 11;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 12;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 13;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 14;
    aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 15;

    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 0;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 1;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 2;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 3;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 4;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 5;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 6;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 7;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 8;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 9;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 10;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 11;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 12;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 13;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 14;
    aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 15;

    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 0;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 1;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 2;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 3;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 4;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 5;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 6;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 7;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 8;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 9;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 10;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 11;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 12;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 13;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 14;
    aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 15;

    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 0;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 1;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 2;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 3;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 4;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 5;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 6;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 7;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 8;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 9;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 10;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 11;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 12;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 13;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 14;
    aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 15;
#endif

    aip3368h_speed_panel_display_buff[0] |= 0x01 << 0; // 时速面板边框，第 1 个指示灯（正面最上边是第 0 个）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 1;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 2;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 3;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 4;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 5;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 6;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 7;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 8;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 9;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 10;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 11;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 12;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 13;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 14;
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 15;

    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 0;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 1;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 2;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 3;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 4;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 5;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 6;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 7;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 8;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 9;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 10;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 11;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 12;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 13;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 14;
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 15;

    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 0;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 1;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 2;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 3;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 4;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 5;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 6;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 7;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 8;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 9;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 10;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 11;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 12;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 13;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 14;
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 15;

    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 0;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 1;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 2;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 3;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 4;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 5;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 6;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 7;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 8;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 9;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 10;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 11;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 12;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 13;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 14;
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 15;

    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 0;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 1;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 2;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 3;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 4;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 5;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 6;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 7;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 8;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 9;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 10;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 11;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 12;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 13;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 14;
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 15;

    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 0;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 1;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 2;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 3;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 4;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 5;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 6;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 7;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 8;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 9;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 10;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 11;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 12;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 13;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 14;
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 15;

    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 0;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 1;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 2;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 3;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 4;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 5;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 6;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 7;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 8;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 9;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 10;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 11;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 12;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 13;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 14;
    // aip3368h_speed_panel_display_buff[6] |= 0x01 << 15;
}
