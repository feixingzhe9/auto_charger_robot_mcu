/*******************************************************************************
 * @name    : 按键驱动文件
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.1
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 按键输入底层代码
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
 * KEY -> PA0
 *
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-03    更改人：布谷鸟
 * 版本记录：V1.0
 * 更改内容：新建
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-03    更改人：布谷鸟
 * 版本记录：V1.1
 * 更改内容：按键检测改为位带操作
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
#include "key.h"

/**
  * @brief  按键IO初始化
  * @param  None
  * @retval None
	* @note   按键连接到GPIOA.00，低电平亮，高电平灭
  */
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
       
  /* 使能GPIOA时钟 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /* 配置GPIOA.00为下拉输入 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/**
  * @brief  按键扫描程序
  * @param  polling:按键扫描模式
  *     @arg polling=0,不支持连续按键
  *     @arg polling=1,支持连续按键
  * @retval None
	* @note   
  */
uint8_t KEY_Scan(uint8_t polling)
{
	static uint8_t key_pre = 0;//前一次按键值
	
	if(polling) key_pre=0;//如果支持连续按键，不需要记录前一次按键值
	
	/* 如果检测到有按键按下，并且支持连续按键 */
	if(polling && KEY) return KEY_CENTER;
	
	if(KEY == 0)//如果没有检测到按键按下
	{
		if(key_pre>KEY_JITTER_COUNT)//如果上一次扫描是有按键按下的，说明按键抬起
		{
			key_pre = 0;//清除上一次按键值
			return KEY_CENTER;//返回有效按键
		}
		else return KEY_NONE;
	}
	else//检测到按键按下
	{
		if(key_pre<0xff)key_pre++;//key_pre累加
	}
	
	return KEY_NONE;
}

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
