#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

#define SYSTICK_PERIOD    1 /* 1ms */


void delay_init(void);
void delay_ms(uint16_t xms);
void delay_us(uint32_t xus);

extern void systick_init(void);
extern uint32_t os_get_time(void);

#endif

