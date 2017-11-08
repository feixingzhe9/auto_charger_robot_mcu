/*******************************************************************************
 * @name    : 蜂鸣器PWM控制驱动
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.1
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 将PB8配置为PWM输出，用于控制蜂鸣器
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
 * TIM4_CH3 -> PB8 控制蜂鸣器
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
 * 更改内容：改为位带操作模式
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
#include "beep.h"
#include "timer.h"

//定时器时钟源Fck_cnt = 72000000/(1+psc)=1000000Hz
//定时器频率(freq) = 72000000/((1+psc)*(arr+1)) = 1000000/2000=500Hz
//占空比(duty) = compare/(arr+1) = 1000/2000 = 50%
uint16_t beep_arr = 1999;//蜂鸣器pwm的重载值
uint16_t beep_psc = 71;//蜂鸣器pwm的预分频值，
uint16_t beep_compare = 1000;//蜂鸣器pwm的比较值（决定占空比）

/**
  * @brief 初始化蜂鸣器
  * @param none
  * @retval none
  * @note
  *    将TIM4_CH3作为蜂鸣器PWM控制
  *    频率范围是: 1MHz~15Hz，初始化后频率是500Hz
	*		 初始化后，占空比是: 0%，因此蜂鸣器不发声
  */
void BEEP_Init(void)
{
	TIM4_PWM_Init(beep_arr,beep_psc);
	BEEP_Off();
}

/**
  * @brief 关闭蜂鸣器
  * @param none
  * @retval none
  * @note
  *    将PWM占空比设置为0%
  */
void BEEP_Off(void)
{
	BEEP_SetDuty(0);
}

/**
  * @brief 打开蜂鸣器
  * @param none
  * @retval none
  * @note
  *   将PWM频率设置为500Hz
	*		将占空比设置为50%
  */
void BEEP_On(void)
{
	BEEP_SetFreq(500);
	BEEP_SetDuty(50);
}

/**
  * @brief 设置蜂鸣器PWM频率
  * @param freq : 设置的频率（单位Hz）范围(15 ~ 65535)
  * @retval none
  * @note
  *    定时器时钟源频率 Fck_cnt = PCLK/(psc+1) = 1MHz
  *    频率 Freq = PCLK/((arr+1)*(psc+1))
  */
void BEEP_SetFreq(uint16_t freq)
{
	if(freq<15)return;//能设置的最低频率必须大于15
	
	beep_arr = 72000000/((1+beep_psc)*freq)-1;//根据频率计算重载值ARR
	beep_compare = (beep_arr+1)/2;	//将占空比设置为50%

	TIM4->ARR = beep_arr;//设置重载值	
	TIM4->CCR3 = beep_compare;//设置比较值
}

/**
  * @brief 获取当前蜂鸣器PWM频率
  * @param none
  * @retval 当前PWM的频率
  * @note
  *    定时器计数频率 = PCLK / ( psc + 1 )
  *    定时器中断周期 = ( arr + 1 )*( psc + 1) / PCLK
  */
uint16_t BEEP_GetFreq(void)
{
	uint16_t freq;
	freq = 72000000/((beep_arr+1)*(beep_psc+1));
	return freq;
}

/**
  * @brief 设置蜂鸣器PWM占空比
  * @param duty : 占空比，如50%，则参数为50
  * @retval none
  * @note
	* 		duty = 100*CCR3/(ARR+1)
  */
void BEEP_SetDuty(uint8_t duty)
{
	beep_compare = duty*(beep_arr+1)/100;//计算比较值
	TIM4->CCR3 = beep_compare;//更新比较值
}

/**
  * @brief 获取当前蜂鸣器PWM占空比
  * @param none
  * @retval 当前PWM的占空比
  * @note
	* 		duty = 100*CCR3/(ARR+1)
  */
uint8_t BEEP_GetDuty(void)
{
	uint16_t duty;
	duty = 100*beep_compare/(beep_arr+1);
	return duty;
}

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/

