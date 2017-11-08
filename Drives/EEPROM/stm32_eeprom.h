/*******************************************************************************
 * @name    : STM32 Flash虚拟EEPROM驱动头文件
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.2
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 介绍通过STM32的FLASH虚拟成EEPROM，非常经典，值得研究！
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
 * 这个驱动文件不是简单的对STM32的FLASH进行页擦除，写，读等操作，而是考虑到FLASH
 * 操作的特性，定义了两个页(Page)对FLASH进行操作，并引入虚拟地址操作，通过这个方
 * 法，可以显著减少页擦除操作，从而成倍的延长FLASH操作寿命和缩短了FLASH操作时间。
 * 当然也可以定义更多的Page满足对FLASH寿命要求非常苛刻的场合。本驱动在ST官方提供
 * 的范例上进行了优化，并对其中的Bug进行了修正。虚拟地址表的定义也使得EEPROM操作
 * 变得非常的便捷。
 * 定义的EEPROM起始地址为FLASH第62KByte(0X08000000+62*1024)开始的2个page，虚拟
 * EEPROM大小为100个16bit的数值（最大可以管理255个16bit数值）。
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-06    更改人：布谷鸟
 * 版本记录：V1.0
 * 更改内容：新建
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/

#ifndef __STM32_EEPROM_H
#define __STM32_EEPROM_H
#include "stm32f10x.h"


#define FLASH_SIZE		(64*1024)//定义STM32 FLASH大小，根据实际情况定义

//定义STM32页大小，FLASH < 256KB
#if (FLASH_SIZE < (256*1024))
  #define PAGE_SIZE  (uint16_t)0x400  /* Page size = 1KByte */
#else
  #define PAGE_SIZE  (uint16_t)0x800  /* Page size = 2KByte */
#endif

//定义模拟EEPROM起始地址，根据自己的实际情况定义，注意需要页对齐！
#define EEPROM_START_ADDRESS    ((uint32_t)(0x08000000+62*PAGE_SIZE))

//定义模拟EEPROM PAGE0和PAGE1的首地址和尾地址，注意地址按照16bit数据类型定义
#define PAGE0_BASE_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + 0x000))
#define PAGE0_END_ADDRESS       ((uint32_t)(EEPROM_START_ADDRESS + (PAGE_SIZE - 1)))

#define PAGE1_BASE_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + PAGE_SIZE))
#define PAGE1_END_ADDRESS       ((uint32_t)(EEPROM_START_ADDRESS + (2 * PAGE_SIZE - 1)))

//定义模拟EEPROM页，这里定义了使用2页模拟EEPROM的情况
#define PAGE0                   ((uint16_t)0x0000)
#define PAGE1                   ((uint16_t)0x0001)

//没有有效页定义，用于查找有效页的返回
#define NO_VALID_PAGE           ((uint16_t)0x00AB)

//PAGE状态定义
#define ERASED                  ((uint16_t)0xFFFF)	// PAGE 空
#define RECEIVE_DATA            ((uint16_t)0xEEEE)	// PAGE 正在接收数据
#define VALID_PAGE              ((uint16_t)0x0000)	// PAGE 有效

//读有效页和写有效页定义
#define READ_FROM_VALID_PAGE    ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE     ((uint8_t)0x01)

//页满了，需要转移有效数据
#define PAGE_FULL               ((uint8_t)0x80)
#define	PAGE_OVER								((uint8_t)0x40)

//定义将存放与模拟EEPROM的数据个数，根据实际情况定义
//作为例子，我们定义128个16bit数据，相当于一个EEPROM: AT24C02
#define NumbOfVar               ((uint8_t)100)

//用户定义的虚拟地址表，虚拟地址必须是唯一的，不能是0XFFFF
extern uint16_t VirtAddVarTab[NumbOfVar];

uint16_t EE_Init(void);
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data);
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data);
uint16_t EE_ReadBuf(uint16_t VirtAddress, uint16_t* Data, uint8_t NbrOfData);
uint16_t EE_WriteBuf(uint16_t VirtAddress, uint16_t* Data, uint8_t NbrOfData);

#endif

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
