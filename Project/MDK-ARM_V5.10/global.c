#include "global.h"

POWER_CONTROL power_ctl = {CONTROL_STOP,0,POWER_OFF,ERR_NONE,0,0};

uint8_t COM1_RX_STATE = COM1_RX_NONE;

uint8_t timer_1s_flag = TIME_NOT_UP;
