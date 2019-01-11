#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"
#include "delay.h"


extern __IO uint16_t capture_status;
extern __IO uint16_t capture_rise_val;
extern __IO uint16_t capture_fall_val;

void timer3_init(uint16_t arr, uint16_t psc);
void time4_ch1_pwm_init(uint16_t arr,uint16_t psc);
void send_remote_data(uint8_t sys_num, uint8_t key_num);

#endif

