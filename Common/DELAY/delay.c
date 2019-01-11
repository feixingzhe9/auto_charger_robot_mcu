#include "delay.h"


void delay_init(void)
{
    /* ����SysTick :HCLK/HCLK_Div8 = 72M/8 */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
}


void delay_ms(uint16_t xms)
{
    uint32_t reload;

    reload = SystemCoreClock/8000;/* ��ʱ1ms������ֵ */
    reload *= xms;/* ��ʱxms������ֵ */
    SysTick->LOAD = (reload & 0xFFFFFF) - 1;/* ����SysTick����ֵ */
    SysTick->VAL = 0;/* ����ֵ���� */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;/* ��ʼ�������� */
    while(!((SysTick->CTRL) & (1 << 16)));/* �ȴ�ʱ�䵽�� */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;/* �رյδ������ */
    SysTick->VAL = 0x00;/* ��ռ����� */
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

