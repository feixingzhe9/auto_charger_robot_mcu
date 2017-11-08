/*******************************************************************************
 * @name    : 串口通信底层驱动
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.1
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : STM32串口的配置和使用
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
 * 注意使用本串口驱动文件时，需要在stm32f10x_it.c编写相应的串口接收中断函数
 * void USART1_IRQHandler(void)。
 *
 * COM1:
 * USART1_TX -> PA9
 * USART1_RX -> PA10
 * 
 * COM2:
 * USART2_TX -> PA2
 * USART2_RX -> PA3
 * 
 * COM3:
 * USART3_TX -> PB10
 * USART3_RX -> PB11
 *
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
 * 更改时间：2014-05-01    更改人：布谷鸟
 * 版本记录：V1.1
 * 更改内容：添加了DMA传输方式的支持
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/

#ifndef __USART_H
#define __USART_H
#include "stm32f10x.h"
#include "remote.h"

#define COMn							3							//定义STM32支持的串口数	

//定义print从哪个串口输出，如果不使用printf，则注释掉这一行
#define PRINTF_SUPPORT		UCORTEX_COM1	
#ifdef PRINTF_SUPPORT
#include <stdio.h>
#endif

//是否使用串口DMA传输
#define 	COM_DMA_TRANSFER		1 //如果不需要支持DMA，请定义为0

#define COM_RX_LEN			128							//串口接收缓冲长度
#if COM_RX_LEN
extern uint16_t COM_RX_STA;							//串口接收状态字
extern uint8_t COM_RX_BUF[COM_RX_LEN];	//串口接收缓冲
#endif

/**
 * @brief 定义COM PORT1，连接到USART1
 */ 
#define UCORTEX_COM1								USART1
#define UCORTEX_COM1_CLK						RCC_APB2Periph_USART1
#define UCORTEX_COM1_TX_PIN					GPIO_Pin_9
#define UCORTEX_COM1_TX_GPIO_PORT		GPIOA
#define UCORTEX_COM1_TX_GPIO_CLK		RCC_APB2Periph_GPIOA
#define UCORTEX_COM1_RX_PIN					GPIO_Pin_10
#define UCORTEX_COM1_RX_GPIO_PORT		GPIOA
#define UCORTEX_COM1_RX_GPIO_CLK		RCC_APB2Periph_GPIOA
#define UCORTEX_COM1_IRQn						USART1_IRQn
#define UCORTEX_COM1_DR_Base				((uint32_t)0x40013804)
#define UCORTEX_COM1_DMA						((DMA_TypeDef*)DMA1)
#define UCORTEX_COM1_TX_DMA_CH			((DMA_Channel_TypeDef*)DMA1_Channel4)
#define UCORTEX_COM1_RX_DMA_CH			((DMA_Channel_TypeDef*)DMA1_Channel5)


/**
 * @brief 定义COM PORT2，连接到USART2
 */ 
#define UCORTEX_COM2								USART2
#define UCORTEX_COM2_CLK						RCC_APB1Periph_USART2
#define UCORTEX_COM2_TX_PIN					GPIO_Pin_2
#define UCORTEX_COM2_TX_GPIO_PORT		GPIOA
#define UCORTEX_COM2_TX_GPIO_CLK		RCC_APB2Periph_GPIOA
#define UCORTEX_COM2_RX_PIN					GPIO_Pin_3
#define UCORTEX_COM2_RX_GPIO_PORT		GPIOA
#define UCORTEX_COM2_RX_GPIO_CLK		RCC_APB2Periph_GPIOA
#define UCORTEX_COM2_IRQn						USART2_IRQn
#define UCORTEX_COM2_DR_Base				((uint32_t)0x40004404)
#define UCORTEX_COM2_DMA						((DMA_TypeDef*)DMA1)
#define UCORTEX_COM2_TX_DMA_CH			((DMA_Channel_TypeDef*)DMA1_Channel7)
#define UCORTEX_COM2_RX_DMA_CH			((DMA_Channel_TypeDef*)DMA1_Channel6)

/**
 * @brief 定义COM PORT3，连接到USART3
 */ 
#define UCORTEX_COM3								USART3
#define UCORTEX_COM3_CLK						RCC_APB1Periph_USART3
#define UCORTEX_COM3_TX_PIN					GPIO_Pin_10
#define UCORTEX_COM3_TX_GPIO_PORT		GPIOB
#define UCORTEX_COM3_TX_GPIO_CLK		RCC_APB2Periph_GPIOB
#define UCORTEX_COM3_RX_PIN					GPIO_Pin_11
#define UCORTEX_COM3_RX_GPIO_PORT		GPIOB
#define UCORTEX_COM3_RX_GPIO_CLK		RCC_APB2Periph_GPIOB
#define UCORTEX_COM3_IRQn						USART3_IRQn
#define UCORTEX_COM3_DR_Base				((uint32_t)0x40004804)
#define UCORTEX_COM3_DMA						((DMA_TypeDef*)DMA1)
#define UCORTEX_COM3_TX_DMA_CH			((DMA_Channel_TypeDef*)DMA1_Channel2)
#define UCORTEX_COM3_RX_DMA_CH			((DMA_Channel_TypeDef*)DMA1_Channel3)

//定义串口端口
typedef enum 
{
  COM1 = 0,
  COM2 = 1,
	COM3 = 2
} COM_TypeDef;

void send_straight(void);
void send_left(void);
void send_right(void);

void COM_Init(COM_TypeDef COM, uint32_t BaudRate); //串口初始化

void COM_DMA_Polling_Init(COM_TypeDef COM, uint32_t MemoryBaseAddr, uint32_t BufferSize, uint32_t DMA_DIR);

void COM1_DMA_init(uint32_t BaudRate);

void reply_usart1(void);

#endif

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
