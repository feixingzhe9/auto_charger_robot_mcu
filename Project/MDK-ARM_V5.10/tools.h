#ifndef __TOOLS_H
#define __TOOLS_H
#include "global.h"
#include "vl6180x.h"
#include "remote.h"
#include "usart.h"
#include "switch.h"
#include "timer.h"

void set_straight(void);
void set_left(void);
void set_right(void);
void set_toward(void);
void set_stop(void);
void set_high_w(uint8_t dir);

uint8_t calculate_length(uint8_t cnt);

void calculate_speed(uint8_t vl6180x_status,uint8_t id);

void update_status(uint8_t id);



#endif
