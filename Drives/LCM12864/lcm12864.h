/*******************************************************************************
 * @name    : 12864点阵模块显示底层驱动头文件
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.0
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 通过并口/串行口驱动
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
 * 并口模式:
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
 * LCM_RST -> PB12 : 复位
 *
 * 串行口模式:
 * LCM_CS  -> PB9  : 选择
 * LCM_SIF -> PB10 : 数据
 * LCM_CLK -> PB11 : 时钟
 * LCM_BK  -> PC13 : 背光控制，高电平亮
 * LCM_RST -> PB12 : 复位
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
#ifndef __LCM12864_H
#define __LCM12864_H

#include "stm32f10x.h"
#include "stdlib.h"
#include "bitband.h"

#define LCM_MODE	0 // 1 - 并行模式; 0 - 串行模式
		    						  
//-----------------LCM端口定义----------------
#define LCM_BK  PCout(13) // 背光控制，高电平亮
#define LCM_RST PBout(12) // 复位

#if LCM_MODE
#define LCM_RS  PBout(9)	// 数据/命令选择
#define LCM_RW  PBout(10)	// 读/写选择
#define LCM_E   PBout(11)	// 使能信号
//PB0..7作为数据线
#define SET_DATA_OUT()	{GPIOB->CRL=0X33333333;}//设置为推挽输出
#define SET_DATA_IN()		{GPIOB->CRL=0X88888888;GPIOB->BSRR=0XFF;}//设置为上拉输入
#define DATAOUT(X) 			{GPIOB->ODR&=0xff00;GPIOB->ODR|=X;}
#define DATAIN					(uint8_t)(GPIOB->IDR&0x00ff)
#else
#define LCM_CS	PBout(9) //选通脚
#define LCM_SIF	PBout(10)//数据
#define LCM_CLK	PBout(11)//时钟脚，上升沿数据发送数据
#endif

void LCM12864_Init(void);
void LCM_WriteData(uint8_t data);
void LCM_WriteCmd(uint8_t cmd);
#if LCM_MODE
uint8_t LCM_ReadData(void);
#endif
void LCM_SetCursor(uint8_t x, uint8_t y);
void LCM_ShowChar(uint8_t X, uint8_t Y, uint8_t data);
void LCM_ShowString(uint8_t x, uint8_t y, uint8_t *str);

void LCM_Gra(uint8_t x, uint8_t y);
void LCM_ShowBMP(uint8_t *bmp);
void LCM_ClrBMP(void);

void LCM_ClrScreen(void);
void LCM_Off(void);
void LCM_On(void);

#endif  

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/

