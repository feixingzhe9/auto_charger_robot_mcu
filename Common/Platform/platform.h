#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "stm32f10x.h"



#define SYSTICK_PERIOD       1      //1ms

void platform_mcu_reset(void);
uint32_t os_get_time(void);
void systick_init(void);
void priority_group_config(uint32_t prio);
uint32_t get_tick(void);

#endif

