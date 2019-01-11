#ifndef __WATCH_DOG_H
#define __WATCH_DOG_H

#include "stm32f10x.h"
#include "stm32f10x_iwdg.h"

void wdg_init(void);
void wdg_enable(void);
void wdg_feed(void);

#endif
