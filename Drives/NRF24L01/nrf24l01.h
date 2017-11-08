/*******************************************************************************
 * @name    : 2.4G无线模块NRF24L01驱动头文件
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.1
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 2.4G无线模块驱动
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
 * 该驱动程序首先初始化了NRF24L01无线模块对应的SPI接口，同时提供了设置NRF24L01
 * 工作模式(RX模式或TX模式)的接口函数。设置接收模式接口函数NRF24L01_TX_Mode中，
 * 我们将NRF24L01的6个接收通到全部使能，并且设置了相应的通道地址。
 * 设置发射模式接口函数NRF24L01_TX_Mode中，引入CHx参数，用来说明将发射端配置为
 * 对应哪个接收通道。
 * 在本例程中，我们设置的接收通道地址为:
 *	RX_ADDR_P0: 0x12, 0x34, 0x56, 0x78, 0x9A
 *	RX_ADDR_P1:	0x11, 0x22, 0x33, 0x44, 0x55
 *	RX_ADDR_P2:	0x22, 0x22, 0x33, 0x44, 0x55
 *	RX_ADDR_P3:	0x33, 0x22, 0x33, 0x44, 0x55
 *	RX_ADDR_P4:	0x44, 0x22, 0x33, 0x44, 0x55
 *	RX_ADDR_P5:	0x55, 0x22, 0x33, 0x44, 0x55
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-06    更改人：布谷鸟
 * 版本记录：V1.0
 * 更改内容：新建
 * ----------------------------------------------------------------------------
 * 更改时间：2014-05-02    更改人：布谷鸟
 * 版本记录：V1.1
 * 更改内容：添加了6个接收通道开启的，从而实现通过6个通道分别接收来至6个发送端
 * 的数据，并通过数据包接收函数NRF24L01_RxPacket返回接收到的是来至哪个通道的数
 * 据。
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/

#ifndef __NRF24L01_H
#define __NRF24L01_H
#include "stm32f10x.h"
#include "bitband.h"

//NRF24L01寄存器操作命令
#define NRF_READ_REG		 0x00	//读配置寄存器，低5位为寄存器地址
#define NRF_WRITE_REG		 0x20	//写配置寄存器，低5位为寄存器地址
#define NRF_RX_PLOAD		 0x61	//读RX有效数据，1-32个字节，读操作始终从0字节开始，接收模式下使用
#define NRF_TX_PLOAD		 0xA0	//写TX有效数据，1-32个字节，写操作始终从0字节开始，发射模式下使用
#define NRF_FLUSH_TX		 0xE1	//清除TX FIFO寄存器，发射模式下使用
#define NRF_FLUSH_RX		 0xE2	//清除RX FIFO寄存器，接收模式下使用
#define NRF_REUSE_TX_PL	 0xE3	//重复使用上一包发送的有效数据，应用与发射端
#define NRF_NOP					 0xFF	//空操作，可用来读取状态寄存器

//NRF24L01寄存器地址定义
#define CONFIG					 0x00  //配置寄存器地址;bit0:1接收模式,0发射模式;bit1:电选择;bit2:CRC模式;bit3:CRC使能;
													 		 //bit4:中断MAX_RT(达到最大重发次数中断)使能;bit5:中断TX_DS使能;bit6:中断RX_DR使能
#define EN_AA            0x01  //使能自动应答功能  bit0~5,对应通道0~5
#define EN_RXADDR        0x02  //接收地址允许,bit0~5,对应通道0~5
#define SETUP_AW         0x03  //设置地址宽度(所有数据通道):bit1,0:00,3字节;01,4字节;02,5字节;
#define SETUP_RETR       0x04  //建立自动重发;bit3:0,自动重发计数器;bit7:4,自动重发延时 250*x+86us
#define RF_CH            0x05  //RF通道,bit6:0,工作通道频率;
#define RF_SETUP         0x06  //RF寄存器;bit3:传输速率(0:1Mbps,1:2Mbps);bit2:1,发射功率;bit0:低噪声放大器增益
#define STATUS           0x07  //状态寄存器;bit0:TX FIFO满标志;bit3:1,接收数据通道号(最大:6);bit4,达到最多次重发
                               //bit5:数据发送完成中断;bit6:接收数据中断;
#define MAX_TX					 0x10  //达到最大发送次数中断
#define TX_OK						 0x20  //TX发送完成中断
#define RX_OK						 0x40  //接收到数据中断

#define OBSERVE_TX       0x08  //发送检测寄存器,bit7:4,数据包丢失计数器;bit3:0,重发计数器
#define CD               0x09  //载波检测寄存器,bit0,载波检测;
#define RX_ADDR_P0       0x0A  //数据通道0接收地址,最大长度5个字节,低字节在前
#define RX_ADDR_P1       0x0B  //数据通道1接收地址,最大长度5个字节,低字节在前
#define RX_ADDR_P2       0x0C  //数据通道2接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P3       0x0D  //数据通道3接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P4       0x0E  //数据通道4接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P5       0x0F  //数据通道5接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define TX_ADDR          0x10  //发送地址(低字节在前),ShockBurstTM模式下,RX_ADDR_P0与此地址相等
#define RX_PW_P0         0x11  //接收数据通道0有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P1         0x12  //接收数据通道1有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P2         0x13  //接收数据通道2有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P3         0x14  //接收数据通道3有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P4         0x15  //接收数据通道4有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P5         0x16  //接收数据通道5有效数据宽度(1~32字节),设置为0则非法
#define NRF_FIFO_STATUS  0x17  //FIFO状态寄存器;bit0,RX FIFO寄存器空标志;bit1,RX FIFO满标志;bit2,3,保留
                               //bit4,TX FIFO空标志;bit5,TX FIFO满标志;bit6,1,循环发送上一数据包.0,不循环;

//24L01发送接收数据宽度定义
#define TX_ADR_WIDTH    5   	//5字节的地址宽度
#define RX_ADR_WIDTH    5   	//5字节的地址宽度
#define TX_PLOAD_WIDTH  12  	//12字节的用户数据宽度(1-32)
#define RX_PLOAD_WIDTH  12  	//12字节的用户数据宽度(1-32)

#define NRF_TIMEOUT_VAL					(200)	//定义SPI通信超时时间
#define NRF_SPI									SPI1
#define NRF_SPI_SPEED						SPI_BaudRatePrescaler_16

//定义NRF24L01控制GPIO
#define NRF_CE									GPIOout(GPIOA,2)
#define NRF_CSN									GPIOout(GPIOA,3)
#define NRF_IRQ									GPIOin(GPIOA, 4)

#define NRF_CSN_SELECT()				{NRF_CSN = 0;}
#define NRF_CSN_DESELECT()			{NRF_CSN = 1;}

typedef enum
{
	NRF_SUCCESS = 0,
	NRF_ERROR,
	NRF_BUSY,
	NRF_TIMEOUT,
}NRF_StatusTypedef;

uint8_t NRF24L01_Init(void);			//NRF24L01初始化
NRF_StatusTypedef NRF24L01_Write_Reg(uint8_t reg, uint8_t val);
NRF_StatusTypedef NRF24L01_Read_Reg(uint8_t reg, uint8_t* val);
NRF_StatusTypedef NRF24L01_Write_Buf(uint8_t reg, uint8_t* pbuf, uint8_t num);
NRF_StatusTypedef NRF24L01_Read_Buf(uint8_t reg, uint8_t* pbuf, uint8_t num);
NRF_StatusTypedef NRF24L01_Check(void);	//检查24L01是否存在
void NRF24L01_RX_Mode(void);
void NRF24L01_TX_Mode(uint8_t CHx);
uint8_t NRF24L01_TxPacket(uint8_t *buf);
uint8_t NRF24L01_RxPacket(uint8_t *buf);
 
#endif

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
