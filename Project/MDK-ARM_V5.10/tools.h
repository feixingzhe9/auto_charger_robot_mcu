#ifndef __TOOLS_H
#define __TOOLS_H

#include "global.h"
#include "vl6180x.h"
#include "remote.h"
#include "usart.h"
#include "switch.h"
#include "timer.h"

uint8_t calculate_length(void);
void update_status(void);
void com_receive(void);

#endif

