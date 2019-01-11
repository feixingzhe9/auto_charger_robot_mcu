#include "led.h"

#define RCC_LED             RCC_APB2Periph_GPIOB
#define GPIO_LED            GPIOB
#define GPIO_Pin_LED        GPIO_Pin_1

#define LED_PERIOD          50

void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_LED, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_LED;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIO_LED, &GPIO_InitStructure);

    GPIO_ResetBits(GPIO_LED, GPIO_Pin_LED);
}


static void led_on(void)
{
    GPIO_ResetBits(GPIO_LED, GPIO_Pin_LED);
}


static void led_off(void)
{
    GPIO_SetBits(GPIO_LED, GPIO_Pin_LED);
}


void led_indicator(uint32_t tick)
{
    static uint32_t start_tick = 0;
    static uint8_t cnt = 0;

    if(start_tick == 0)
    {
        start_tick = tick;
    }
    if(tick - start_tick >= LED_PERIOD)
    {
        if(cnt++ % 2)
        {
            led_on();
        }
        else
        {
            led_off();
        }
        start_tick = tick;
    }
}
