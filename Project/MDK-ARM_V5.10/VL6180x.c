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
  HAL_OK       = 0x00,
  HAL_ERROR    = 0x01,
  HAL_BUSY     = 0x02,
  HAL_TIMEOUT  = 0x03
};

void GPIO0_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//使能GPIOB外设时钟
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC, ENABLE );	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC,GPIO_Pin_2);//根据以上配置初始化GPIO

	//设置SCL和SDA空闲状态为高电平
//	I2C_SCL = 1;
//	I2C_SDA = 1;
//	
//	I2C_SetSpeed(400);//设置I2C访问速度为400Kbps
}

void VL6180x_init(void)
{
	GPIO0_Init();
	GPIOout(GPIOC, 2) = 1;

	VL6180x_InitData(VL6180X_ADDRESS);
  VL6180x_Prepare(VL6180X_ADDRESS);
}

uint8_t VL6180x_start(void)
{
	uint8_t status;
	uint8_t pdata1;
	status = single_read(VL6180X_ADDRESS,0x000,&pdata1);
//		printf("status = %d\r\n",status);
	if(status != I2C_SUCCESS)
	{
		return 1;
	}
	single_write(VL6180X_ADDRESS, 0x015, 0x01);
	while(single_read(VL6180X_ADDRESS,RESULT__RANGE_STATUS,&pdata1));
	single_write(VL6180X_ADDRESS, 0x018, 0x01);
	
	return 0;
}


volatile uint16_t range_value = MAX_RANGE;

uint8_t VL6180x_end(void)
{
	uint8_t status;
	uint8_t pdata1 = 0;
	uint16_t pdata_3 = 0;
		
	int alpha =(int)(0.85*(1<<16));    /* range distance running average cofs */
  static uint16_t range = 0;             /* range average distance */
	
	status = single_read(VL6180X_ADDRESS,0x000,&pdata1);
//		printf("status = %d\r\n",status);
	if(status != I2C_SUCCESS)
	{
		range_value = MAX_RANGE;
		return 1;
	}
	
	single_read(VL6180X_ADDRESS,RESULT__INTERRUPT_STATUS_GPIO,&pdata1);
//		printf("RESULT__INTERRUPT_STATUS_GPIO = %d\r\n",pdata1);
	if(pdata1 == 4)
	{
		single_read(VL6180X_ADDRESS,RESULT__RANGE_VAL,&pdata1);
		if(pdata1 == 255)
		{
			range_value = MAX_RANGE;
			return 0;
		}
		pdata_3 = 3 * pdata1;
		range = ((range * alpha + pdata_3 * ((1 << 16) - alpha)) >> 16 );
		range_value = range;
//		range_value = range + 24;
		if(range_value >= MAX_RANGE)
		{
			range_value = MAX_RANGE;
		}
//			pdata1 = 0;
//			pdata_3 = 0;
	}
	else
	{
		range_value = MAX_RANGE;
	}
	return 0;
}








