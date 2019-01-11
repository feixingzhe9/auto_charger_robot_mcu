#include "switch.h"

#define SWITCH_X_0          0
#define SWITCH_X_1          1
#define SWITCH_NUM          2

#define RCC_SWITCH0         RCC_APB2Periph_GPIOA
#define GPIO_SWITCH0        GPIOA
#define GPIO_Pin_SWITCH0    GPIO_Pin_0
#define BIT_SWITCH0         0

#define RCC_SWITCH1         RCC_APB2Periph_GPIOA
#define GPIO_SWITCH1        GPIOA
#define GPIO_Pin_SWITCH1    GPIO_Pin_1
#define BIT_SWITCH1         1

#define VAL_NUM             30
#define SWITCH_IN           0
#define SWITCH_OUT          1

#define SWITCH0_INIT       {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
#define SWITCH1_INIT       {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}


static void switch_x_init(uint8_t index)
{
    uint32_t rcc_x =0;
    GPIO_TypeDef * port_x = NULL;
    uint16_t pin_x = 0xFF;

    if(SWITCH_X_0 == index)
    {
        rcc_x  = RCC_SWITCH0;
        port_x = GPIO_SWITCH0;
        pin_x  = GPIO_Pin_SWITCH0;
    }
    else if(SWITCH_X_1 == index)
    {
        rcc_x  = RCC_SWITCH1;
        port_x = GPIO_SWITCH1;
        pin_x  = GPIO_Pin_SWITCH1;
    }

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(rcc_x, ENABLE);

    GPIO_InitStructure.GPIO_Pin = pin_x;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(port_x, &GPIO_InitStructure);
}


void swtich_init(void)
{
    switch_x_init(SWITCH_X_0);
    switch_x_init(SWITCH_X_1);
}


static uint8_t switch_x_scan(uint8_t index)
{
    uint8_t i = 0;
    uint8_t switch_st = 0;
    GPIO_TypeDef *port_x[SWITCH_NUM] = {GPIO_SWITCH0, GPIO_SWITCH1};
    uint16_t bit_x[SWITCH_NUM] = {BIT_SWITCH0, BIT_SWITCH1};

    static uint8_t switch_val[SWITCH_NUM] = {SWITCH_OUT, SWITCH_OUT};
    static uint8_t switch_buf[SWITCH_NUM][VAL_NUM] = {SWITCH0_INIT, SWITCH1_INIT};

    for(i = 0; i < VAL_NUM-1; i++)
    {
        switch_buf[index][i] = switch_buf[index][i+1];
    }
    switch_buf[index][VAL_NUM-1] = GPIOin(port_x[index], bit_x[index]);

    for(i = 0; i < VAL_NUM-1; i++)
    {
        if(switch_buf[index][i] != switch_buf[index][i+1])
        {
            switch_st = 1;
        }
    }

    if(switch_st == 0)
    {
        switch_val[index] = switch_buf[index][0];
    }

    return switch_val[index];
}


uint8_t switch_scan(void)
{
    uint8_t switch0_val = 1;
    uint8_t switch1_val = 1;

    switch0_val = switch_x_scan(SWITCH_X_0);
    switch1_val = switch_x_scan(SWITCH_X_1);

    if((SWITCH_OUT == switch0_val)&&(SWITCH_OUT == switch1_val))
    {
        return SWITCH_ON;
    }
    else if((SWITCH_OUT != switch0_val)&&(SWITCH_OUT == switch1_val))
    {
        return SWITCH1_ON;
    }
    else if((SWITCH_OUT == switch0_val)&&(SWITCH_OUT != switch1_val))
    {
        return SWITCH2_ON;
    }
    else if((SWITCH_OUT != switch0_val)&&(SWITCH_OUT != switch1_val))
    {
        return SWITCH_OFF;
    }

    return SWITCH_NONE;
}

