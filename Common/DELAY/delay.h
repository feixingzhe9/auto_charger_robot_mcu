/*******************************************************************************
 * @name    : 延时函数头文件
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.0
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 毫秒和微秒的延时，供后面的实验例程调用
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
 * KEY -> PA0
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

#ifndef __DELAY_H
#define __DELAY_H
#include "stm32f10x.h"

#define SYSTICK_PERIOD                1      //1ms

void Delay_Init(void);	//SysTick延时初始化
void delay_ms(uint16_t xms);//毫秒延时函数
//void delay_us(uint32_t xus);//微秒延时函数
extern void SysTickInit(void);
extern uint32_t os_get_time(void);

#define asm            __asm
#define delay_300ns()     do {asm("nop");asm("nop");asm("nop");asm("nop");\
                              asm("nop");asm("nop");asm("nop");asm("nop");\
                              asm("nop");asm("nop");asm("nop");asm("nop");\
                              asm("nop");asm("nop");asm("nop");asm("nop");\
                              asm("nop");asm("nop");asm("nop");asm("nop");\
                              asm("nop");asm("nop");} while(1==0)
                                
#define delay_600ns()     do { asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");asm("nop");asm("nop");\
                               asm("nop");asm("nop");} while(1==0)
                                
//#define delay_us(n)       do { for(uint32_t i=0;i<n;i++){delay_300ns();delay_600ns();}\
                                } while(0==1)
void delay_us(uint32_t xus);

#endif

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
