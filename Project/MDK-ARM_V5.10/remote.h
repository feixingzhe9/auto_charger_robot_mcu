#ifndef __REMOTE_H
#define __REMOTE_H

#include "stm32f10x.h"
#include "bitband.h"
#include "usart.h"
#include "tools.h"


void remote_init(void);
uint8_t remote_scan(void);
void remote_calculate(uint8_t scan_value);

#endif

