#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "stm32f10x.h"

#define SYSTICK_PERIOD       1      //1ms
void platform_mcu_reset(void);
uint32_t os_get_time(void);
void SysTickInit(void);
uint32_t GetTick(void);

#endif

