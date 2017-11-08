/*******************************************************************************
 * @name    : OLED模块底层驱动头文件
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.0
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 通过4线SPI驱动OLED显示
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
 * OLED_PWR  -> PC13 : OLED电源控制口，高电平打开电源
 * OLED_CS   -> PB3  : OLED SPI选通
 * OLED_RST  -> PB4  : OLED复位
 * OLED_DC   -> PB5  : OLED 数据/命令选择口 D/C#
 * OLED_SCLK -> PB6  : OLED SPI时钟
 * OLED_SDIN -> PB7  : OLED SPI数据
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
#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"
#include "stdlib.h"
#include "bitband.h"

//OLED模式设置: 4线串行模式
		    						  
//-----------------OLED端口定义----------------
#define OLED_PWR			GPIOout(GPIOC,13)	//OLED电源控制IO，高电平打开电源
#define OLED_CS				GPIOout(GPIOB,3)	//OLED SPI选通
#define OLED_RST			GPIOout(GPIOB,4)	//OLED复位
#define OLED_DC				GPIOout(GPIOB,5)	//OLED D/C#
#define OLED_SCLK			GPIOout(GPIOB,6)	//OLED SPI时钟
#define OLED_SDIN			GPIOout(GPIOB,7)	//OLED SPI数据
 		     
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

//OLED控制用函数
void OLED_WR_Byte(uint8_t dat,uint8_t DC);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);		   
							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t dot);
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size,uint8_t mode);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);
void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p);
void OLED_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);
#endif  

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/

