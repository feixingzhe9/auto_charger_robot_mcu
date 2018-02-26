#include "switch.h"

/**
  * @brief  按键IO初始化
  * @param  None
  * @retval None
	* @note   按键连接到GPIOA.0，低电平亮，高电平灭
  */
static void switch1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
       
  /* 使能GPIOA时钟 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);

  /* 配置GPIOA.00为下拉输入 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief  按键IO初始化
  * @param  None
  * @retval None
	* @note   按键连接到GPIOA.0，低电平亮，高电平灭
  */
static void switch2_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
       
  /* 使能GPIOA时钟 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);

  /* 配置GPIOA.00为下拉输入 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void swtich_init(void)
{
	switch1_init();
	switch2_init();
}

static uint8_t switch1_scan(void)
{
	static uint8_t switch1_value = 0;
	uint8_t switch1_state = 0;
	static uint8_t switch1_value_buff[SWITCH_VALUE_NUM] = {SWITCH_OUT};
	uint8_t i;
	
	for(i=0;i<SWITCH_VALUE_NUM-1;i++)
	{
		switch1_value_buff[i] = switch1_value_buff[i+1];
	}
	switch1_value_buff[SWITCH_VALUE_NUM-1] = SWITCH1_SCAN;
	
	for(i=0;i<SWITCH_VALUE_NUM-1;i++)
	{
		if(switch1_value_buff[i] != switch1_value_buff[i+1])
		{
			switch1_state = 1;
		}
	}	
	if(switch1_state == 0)
	{
		switch1_value = switch1_value_buff[0];
	}
//	switch1_value = SWITCH1_SCAN;
	return switch1_value;
}

static uint8_t switch2_scan(void)
{
	uint8_t switch2_value;
	uint8_t switch2_state = 0;
	static uint8_t switch2_value_buff[SWITCH_VALUE_NUM] = {SWITCH_OUT};
	uint8_t i;
	
	for(i=0;i<SWITCH_VALUE_NUM-1;i++)
	{
		switch2_value_buff[i] = switch2_value_buff[i+1];
	}
	switch2_value_buff[SWITCH_VALUE_NUM-1] = SWITCH2_SCAN;
	
	for(i=0;i<SWITCH_VALUE_NUM-1;i++)
	{
		if(switch2_value_buff[i] != switch2_value_buff[i+1])
		{
			switch2_state = 1;
		}
	}	
	if(switch2_state == 0)
	{
		switch2_value = switch2_value_buff[0];
	}
//	switch2_value = SWITCH2_SCAN;
	return switch2_value;
}

uint8_t switch_scan(void)
{
	uint8_t switch1_value;
	uint8_t switch2_value;
	
	switch1_value = switch1_scan();
	switch2_value = switch2_scan();
    
    //return SWITCH_ON ;////test code
    
	if((SWITCH_OUT == switch1_value)&&(SWITCH_OUT == switch2_value))
	{
		return SWITCH_ON ;
	}
	if((SWITCH_OUT != switch1_value)&&(SWITCH_OUT == switch2_value))
	{
		return SWITCH1_ON ;
	}
	if((SWITCH_OUT == switch1_value)&&(SWITCH_OUT != switch2_value))
	{
		return SWITCH2_ON ;
	}
	if((SWITCH_OUT != switch1_value)&&(SWITCH_OUT != switch2_value))
	{
		return SWITCH_OFF ;
	}
	return 4;
}
