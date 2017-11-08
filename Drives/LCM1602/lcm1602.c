/*******************************************************************************
 * @name    : 1602点阵模块显示底层驱动
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.0
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 通过并口驱动
 * ---------------------------------------------------------------------------- 
 * @copyright
 *
 * UCORTEX版权所有，违者必究！例程源码仅供大家参考，旨在交流学习和节省开发时间，
 * 对于因为参考本文件内容，导致您的产品直接或间接受到破坏，或涉及到法律问题，作
 * 者不承担任何责任。对于使用过程中发现的问题，如果您能在WWW.UCORTEX.COM网站反
 * 馈给我们，我们会非常感谢，并将对问题及时研究并作出改善。例程的版本更新，将不
 * 做特别通知，请您自行到WWW.UCORTEX.COM下载最新版本，谢谢。
 * 对于以上声明，UCORTEX保留进一步解释的权利！
 * ----------------------------------------------------------------------------
 * @description
 *
 * LCM_RS  -> PB9  : 数据/命令选择
 * LCM_RW  -> PB10 : 读/写选择
 * LCM_E   -> PB11 : 使能信号
 * LCM_D0  -> PB0  : 数据
 * LCM_D1  -> PB1  : 数据
 * LCM_D2  -> PB2  : 数据
 * LCM_D3  -> PB3  : 数据
 * LCM_D4  -> PB4  : 数据
 * LCM_D5  -> PB5  : 数据
 * LCM_D6  -> PB6  : 数据
 * LCM_D7  -> PB7  : 数据
 * LCM_BK  -> PC13 : 背光控制，高电平亮
 *
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-03    更改人：布谷鸟
 * 版本记录：V1.0
 * 更改内容：新建
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
#include "stdlib.h"
#include "delay.h"
#include "lcm1602.h"

/**
  * @brief  1602 LCM模块初始化
  * @param  None
  * @retval None
	* @note 
  */
void LCM1602_Init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
	
 	/* 使能GPIOB,GPIOC，AFIO时钟 */
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);

  /* 配置STM32为SWD调试模式，注意要先开启AFIO时钟 */
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);	//SWD模式
	
	/* PC13配置为推挽输出，用于控制LCM背光控制，注意PC13不支持5V耐压，速度只能配置到2MHz（详情参考规格书） */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	//速度2MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);	//初始化
	GPIO_SetBits(GPIOC, GPIO_Pin_13);//打开背光
	
	/* 配置PB0..7,PB9,PB10,PB11推挽输出 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
	                              GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|
	                              GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);						//初始化
 
  LCM_E=0;
  
  delay_ms(15);
	LCM_WriteCmd(0x38,0); //三次显示模式设置，不检测忙信号
	delay_ms(5);
	LCM_WriteCmd(0x38,0);
	delay_ms(5);
	LCM_WriteCmd(0x38,0);
	delay_ms(5);

	LCM_WriteCmd(0x38,1); //显示模式设置,开始要求每次检测忙信号
	LCM_WriteCmd(0x08,1); //关闭显示,打开光标闪烁
	LCM_WriteCmd(0x01,1); //显示清屏
	LCM_WriteCmd(0x06,1); // 显示光标移动设置
	LCM_WriteCmd(0x0C,1); // 显示开及光标设置：开显示，无光标
}

/**
  * @brief  1602 LCM些数据
  * @param  data : 写入的数据
  * @retval None
	* @note 
  */
void LCM_WriteData(uint8_t data)
{
	LCM_ReadStatus();	//检测忙
	SET_DATA_OUT();
	DATAOUT(data);
	LCM_RS = 1;
	LCM_RW = 0;
	LCM_E = 1;
	delay_us(100);
	LCM_E = 0;
}

/**
  * @brief  1602 LCM模块写指令
  * @param  cmd : 写入的指令
  * @param  cmd : 0 - 不做忙检测  1 - 做忙检测
  * @retval None
	* @note 
  */
void LCM_WriteCmd(uint8_t cmd, uint8_t busy) 
{
  if (busy) LCM_ReadStatus(); //忙检测
	SET_DATA_OUT();
	DATAOUT(cmd);
	LCM_RS = 0;
	LCM_RW = 0;
	LCM_E = 1;
	delay_us(100);
	LCM_E = 0;
}

/**
  * @brief  1602 LCM读数据
  * @param  None
  * @retval 读到的数据
	* @note 
  */
uint8_t LCM_ReadData(void)
{
	uint8_t data;
	
	SET_DATA_IN();
	LCM_RS = 1;
	LCM_RW = 1;
	LCM_E = 1;
	delay_us(100);
	data=DATAIN;
	LCM_E = 0;
	return data;
}

/**
  * @brief  1602 LCM模块状态检测
  * @param  None
  * @retval None
	* @note 
  */
void LCM_ReadStatus(void)
{
	SET_DATA_IN();
	LCM_RS = 0;
	LCM_RW = 1;
	LCM_E = 1;
	delay_us(100);
	while (DATAIN & 0X80); //检测忙信号
	LCM_E = 0;
}

/**
  * @brief  字符初始位置设定
  * @param  x - 第几列 (0~15)
  * @param  y - 第几行 (0~1)
  * @retval None
	* @note 
  */
void LCM_SetXY(uint8_t x, uint8_t y)
{ 
  y &= 0X1;//y为第几行，y不能大于1
  x &= 0XF;//x为第几列，x不能大于15
  if(y==0)     x+=0X80; 
  else if(y==1)x+=0XC0; 
  LCM_WriteCmd(x,1);
}

/**
  * @brief  在指定位置显示一个字符
  * @param  x - 第几列 (0~15)
  * @param  y - 第几行 (0~1)
  * @retval None
	* @note 
  */
void LCM_ShowChar(uint8_t x, uint8_t y, uint8_t ch)
{
  LCM_SetXY(x, y);
  LCM_WriteData(ch);
}

/**
  * @brief  从指定位置开始显示字符串
  * @param  x - 第几列 (0~15)
  * @param  y - 第几行 (0~1)
  * @param *str - 字符串指针
  * @retval None
	* @note 
  */
void LCM_ShowString(uint8_t x, uint8_t y, uint8_t *str) 
{ 
  LCM_SetXY(x,y); 
	while(*str) 
  { 
    LCM_WriteData(*str); 
    str++; 
  }
}

/**
  * @brief  清屏
  * @param  none
  * @retval none
	* @note 
  */
void LCM_ClrScreen(void)
{
	LCM_WriteCmd(0x01,1); //显示清屏
}

/**
  * @brief  关闭显示和背光
  * @param  None
  * @retval None
	* @note 
  */
void LCM_Off(void)
{
	LCM_BK=0;//关背光
	LCM_WriteCmd(0x08,1);//关显示
}

/**
  * @brief  开显示和背光
  * @param  None
  * @retval None
	* @note 
  */
void LCM_On(void)
{
	LCM_WriteCmd(0x0C,1);//关显示
	LCM_BK=1;//关背光
}


/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/

