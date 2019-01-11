#include "watchdog.h"
#include <stdio.h>

#define WDG_FEED_PERIOD     2000

void wdg_init(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);
    IWDG_SetReload(WDG_FEED_PERIOD/32);
    IWDG_ReloadCounter();
}


void wdg_enable(void)
{
    IWDG_Enable();
}


void wdg_feed(void)
{
    IWDG_ReloadCounter();
}
