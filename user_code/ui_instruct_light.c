#include "ui_instruct_light.h"
#include "instrument.h"
#include "aip3368h_display.h"

void ui_instruct_light_process(void)
{
    aip3368h_display_left_turn_light(instrument.left_turn_valid);
    aip3368h_display_right_turn_light(instrument.right_turn_valid);
    aip3368h_display_temp_of_water_err_light(
        instrument.temp_of_water_err_valid);
    aip3368h_display_high_beam_light(instrument.high_beam_valid);
}
