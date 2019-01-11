#ifndef __SWITCH_H
#define __SWITCH_H

#include "stm32f10x.h"
#include "bitband.h"
#include "global.h"

void swtich_init(void);
uint8_t switch_scan(void);

#endif
