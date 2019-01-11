#include "global.h"

POWER_CONTROL power_ctl = {CONTROL_STOP,0,POWER_OFF,ERR_NONE,0,0,0};
uint8_t timer_1s_flag = TIME_NOT_UP;


ir_rcv_info_t ir_info = {0};
ir_signal_intensity_t ir_signal_intensity = {0};
