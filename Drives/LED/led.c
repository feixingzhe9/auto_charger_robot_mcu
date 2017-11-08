/*******************************************************************************
 * @name    : LED控制驱动文件
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.1
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 配置STM32 PB12作为输出口，用来控制LED指示灯
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
 * LED -> PB12
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-03    更改人：布谷鸟
 * 版本记录：V1.0
 * 更改内容：新建
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-03    更改人：布谷鸟
 * 版本记录：V1.1
 * 更改内容：改为位带操作模式
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
#include "led.h"

/**
  * @brief  LED IO初始化
  * @param  None
  * @retval None
	* @note   LED链接到GPIOB.12，低电平亮，高电平灭
  */
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
       
  /* 使能GPIOB时钟 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /* 配置GPIOB.12为推挽输出模式 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_12);//设置GPIOB.12为高电平，关闭LED
}

/**
  * @brief  点亮LED指示灯
  * @param  None
  * @retval None
	* @note   LED链接到GPIOB.12，低电平亮，高电平灭
  */
void LED_On(void)
{
	LED = 0;
}

/**
  * @brief  关闭LED指示灯
  * @param  None
  * @retval None
	* @note   LED链接到GPIOB.12，低电平亮，高电平灭
  */
void LED_Off(void)
{
	LED = 1;
}

/**
  * @brief  LED指示灯反转
  * @param  None
  * @retval None
	* @note   LED链接到GPIOB.12，低电平亮，高电平灭
  */
void LED_Toggle(void)
{
	LED = !LED;
}

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
