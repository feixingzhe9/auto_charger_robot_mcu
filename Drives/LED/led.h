#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"
#include "bitband.h"

void led_init(void);
void led_indicator(uint32_t tick);

#endif
