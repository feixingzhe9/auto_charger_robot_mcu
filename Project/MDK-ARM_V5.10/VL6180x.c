#include "VL6180x.h"
#include "string.h"
#include "i2c.h"
#include "usart.h"
#include "delay.h"
#include "global.h"
#include "vl6180x_api.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>


enum
{
    HAL_OK      = 0x00,
    HAL_ERROR   = 0x01,
    HAL_BUSY    = 0x02,
    HAL_TIMEOUT = 0x03
};


volatile uint16_t range_value = MAX_RANGE;


void vl6180x_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC, GPIO_Pin_2);
}


void vl6180x_init(void)
{
    vl6180x_gpio_init();
    GPIOout(GPIOC, 2) = 1;

    VL6180x_InitData(VL6180X_ADDRESS);
    VL6180x_Prepare(VL6180X_ADDRESS);
}


uint8_t vl6180x_start(void)
{
    uint8_t status;
    uint8_t pdata1;

    status = i2c_single_byte_read(VL6180X_ADDRESS, 0x000, &pdata1);
    if(status != I2C_SUCCESS)
    {
        return 1;
    }

    i2c_single_byte_write(VL6180X_ADDRESS, 0x015, 0x01);
    while(i2c_single_byte_read(VL6180X_ADDRESS,RESULT__RANGE_STATUS, &pdata1));

    i2c_single_byte_write(VL6180X_ADDRESS, 0x018, 0x01);

    return 0;
}


uint8_t vl6180x_finish(void)
{
    uint8_t status;
    uint8_t pdata1 = 0;
    uint16_t pdata_3 = 0;
    static uint16_t range = 0;/* range average distance */
    int alpha =(int)(0.85 * (1 << 16));/* range distance running average cofs */

    status = i2c_single_byte_read(VL6180X_ADDRESS, 0x000, &pdata1);
    if(status != I2C_SUCCESS)
    {
        range_value = MAX_RANGE;
        return 1;
    }

    i2c_single_byte_read(VL6180X_ADDRESS, RESULT__INTERRUPT_STATUS_GPIO, &pdata1);
    if(pdata1 == 4)
    {
        i2c_single_byte_read(VL6180X_ADDRESS, RESULT__RANGE_VAL, &pdata1);
        if(pdata1 == 255)
        {
            range_value = MAX_RANGE;
            return 0;
        }
        pdata_3 = 3 * pdata1;
        range = ((range * alpha + pdata_3 * ((1 << 16) - alpha)) >> 16);
        range_value = range;
        if(range_value >= MAX_RANGE)
        {
            range_value = MAX_RANGE;
        }
    }
    else
    {
        range_value = MAX_RANGE;
    }

    return 0;
}

