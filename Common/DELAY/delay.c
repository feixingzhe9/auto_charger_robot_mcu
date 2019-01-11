#include "delay.h"


void delay_init(void)
{
    /* 设置SysTick :HCLK/HCLK_Div8 = 72M/8 */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
}


void delay_ms(uint16_t xms)
{
    uint32_t reload;

    reload = SystemCoreClock/8000;/* 延时1ms的重载值 */
    reload *= xms;/* 延时xms的重载值 */
    SysTick->LOAD = (reload & 0xFFFFFF) - 1;/* 加载SysTick重载值 */
    SysTick->VAL = 0;/* 计数值清零 */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;/* 开始倒数计数 */
    while(!((SysTick->CTRL) & (1 << 16)));/* 等待时间到达 */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;/* 关闭滴答计数器 */
    SysTick->VAL = 0x00;/* 清空计数器 */
}


void delay_us(uint32_t xus)
{
    uint32_t reload;

    reload = SystemCoreClock/8000000;
    reload *= xus;
    SysTick->LOAD = (reload & 0xFFFFFF) - 1;
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while(!((SysTick->CTRL) & (1 << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0x00;
}

