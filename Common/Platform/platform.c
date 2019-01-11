#include "platform.h"
#include "misc.h"


volatile uint32_t sys_tick_cnt = 0;


uint32_t os_get_time(void)
{
    return sys_tick_cnt;
}


void systick_init(void)
{
    uint32_t ticks = SystemCoreClock/8000;

    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

    SysTick->LOAD  = (ticks & SysTick_LOAD_RELOAD_Msk) - 1;/* set reload register */
    SysTick->VAL   = 0;/* Load the SysTick Counter Value */
    SysTick->CTRL  = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;/* Enable SysTick IRQ and SysTick Timer */
}


void platform_mcu_reset(void)
{
    NVIC_SystemReset();
}


void priority_group_config(uint32_t prio)
{
    NVIC_PriorityGroupConfig(prio);
}
