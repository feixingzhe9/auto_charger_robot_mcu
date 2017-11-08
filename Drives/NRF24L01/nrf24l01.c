/*******************************************************************************
 * @name    : 2.4G无线模块NRF24L01驱动
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
 
#include "nrf24l01.h"
#include "spi.h"
#include "delay.h"

//地址寄存器是从低字节开始写入，因此写低字节的时候，高字节保持原有数据不变
const uint8_t TX_ADDRESS[TX_ADR_WIDTH]= {0x12, 0x34, 0x56, 0x78, 0x9A}; //发送地址
const uint8_t RX_ADDRESS0[RX_ADR_WIDTH]={0x12, 0x34, 0x56, 0x78, 0x9A}; //接收通道0地址
const uint8_t RX_ADDRESS1[RX_ADR_WIDTH]={0x11, 0x22, 0x33, 0x44, 0x55}; //接收通道1地址
const uint8_t RX_ADDRESS2[1]={0x22};//接收通道2地址
const uint8_t RX_ADDRESS3[1]={0x33};//接收通道3地址
const uint8_t RX_ADDRESS4[1]={0x44};//接收通道4地址
const uint8_t RX_ADDRESS5[1]={0x55};//接收通道5地址

/**
  * @brief NRF24L01无线模块初始化
  * @param None
  * @retval 0 - 初始化成功; 1 - 初始化失败
  * @note
	* 	NRF24L01无线模块连接到SPI1上
	*		NRF_CE: 	PA2
	*		NRF_CSN:	PA3
	*		NRF_IRQ:	PA4
  */
uint8_t NRF24L01_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//配置NRF24L01片选管脚和SPI选通管脚,片选: PA2; SPI选通: PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//根据上面配置信息初始化
	NRF_CSN_DESELECT();//初始化SPI之前，先使NRF24L01处于非选通状态
	
	//配置NRF24L01中断引脚为下拉输入
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;   
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //下拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	SPI1_Init();	//SPI1初始化
	
	NRF_CE = 0;	//NRF24L01片选使能

	if(NRF24L01_Check())return 1;
	
	NRF_CE = 1;
	return 0;
}

/**
  * @brief 写寄存器函数
  * @param reg : 寄存器地址
	* @param val : 设置的寄存器值
  * @retval NRF24L01操作状态
  * @note
	* 	
  */
NRF_StatusTypedef NRF24L01_Write_Reg(uint8_t reg, uint8_t val)
{
	uint8_t status = 0;
	uint32_t timeout = 0;
	
	SPI_Set_BaudRate(SPI1, NRF_SPI_SPEED);	//设置SPI1波特率
	NRF_CSN_SELECT();	//获取SPI总线
	timeout = NRF_TIMEOUT_VAL;//设置访问超时时间
	status = SPI_RW(NRF_SPI,reg,&timeout);//写寄存器地址
	if(timeout == 0x00 && status == 0xff)//判断是否超时
	{
		NRF_CSN_DESELECT();	//释放SPI总选
		return NRF_TIMEOUT;		//返回错误
	}
	
	timeout = NRF_TIMEOUT_VAL;//设置访问超时时间
	status = SPI_RW(NRF_SPI,	val, &timeout);//写数据
	if(timeout == 0x00 && status == 0xff)//判断是否超时
	{
		NRF_CSN_DESELECT();//释放SPI总选
		return NRF_TIMEOUT;//返回错误
	}
	
	NRF_CSN_DESELECT();//释放SPI总选
	
	return NRF_SUCCESS;//返回操作成功
}

/**
  * @brief 读寄存器
  * @param reg : 寄存器地址
	* @param *val : 读取到的寄存器值存放地址
  * @retval NRF24L01操作状态
  * @note
	* 	
  */
NRF_StatusTypedef NRF24L01_Read_Reg(uint8_t reg, uint8_t* val)
{
	uint8_t data = 0;
	uint32_t timeout = 0;
	
	SPI_Set_BaudRate(SPI1, NRF_SPI_SPEED);	//设置SPI1波特率
	NRF_CSN_SELECT();	//获取SPI总线
	timeout = NRF_TIMEOUT_VAL;//设置访问超时时间
	data = SPI_RW(NRF_SPI,0x00,&timeout);//写寄存器地址
	if(timeout == 0x00 && data == 0xff)//判断是否超时
	{
		NRF_CSN_DESELECT();	//释放SPI总选
		return NRF_TIMEOUT;		//返回错误
	}
	
	*val = data;				//保存读取的寄存器值
	NRF_CSN_DESELECT();	//释放SPI总选
	
	return NRF_SUCCESS;	//返回操作成功
}

/**
  * @brief 连续写数据到缓冲区(NRF24L01 FIFO)
  * @param reg : 寄存器地址
	* @param *pbuf : 数据包首地址
	* @param num : 发送的数据个数
  * @retval NRF24L01操作状态
  * @note  该函数主要用于将一组数据放到NRF24L01的发射FIFO缓冲区
	* 	
  */
NRF_StatusTypedef NRF24L01_Write_Buf(uint8_t reg, uint8_t* pbuf, uint8_t num)
{
	uint8_t status = 0;
	uint32_t timeout = 0;
	uint8_t i = 0;
	
	SPI_Set_BaudRate(SPI1, NRF_SPI_SPEED);	//设置SPI1波特率
	NRF_CSN_SELECT();//获取SPI总线
	timeout = NRF_TIMEOUT_VAL;//设置访问超时时间
	status = SPI_RW(NRF_SPI,reg,&timeout);	//选择要写入的寄存器
	if(timeout == 0x00 && status == 0xff)//判断是否超时
	{
		NRF_CSN_DESELECT();	//释放SPI总选
		return NRF_TIMEOUT;	//返回错误
	}
	
	for(i = 0; i < num; i++)
	{
		timeout = NRF_TIMEOUT_VAL;//设置访问超时时间
		status = SPI_RW(NRF_SPI,*pbuf,&timeout);
		if(timeout == 0x00 && status == 0xff)//判断是否超时
		{
			NRF_CSN_DESELECT();	//释放SPI总选
			return NRF_TIMEOUT;	//返回错误
		}
		pbuf++;
	}
	NRF_CSN_DESELECT();//释放SPI总线
	return NRF_SUCCESS;//返回操作成功
}

/**
  * @brief 连续写数据到缓冲区(NRF24L01 FIFO)
  * @param reg : 寄存器地址
	* @param *pbuf : 数据包首地址
	* @param num : 发送的数据个数
  * @retval NRF24L01操作状态
  * @note  该函数主要用于将一组数据放到NRF24L01的发射FIFO缓冲区
	* 	
  */
NRF_StatusTypedef NRF24L01_Read_Buf(uint8_t reg, uint8_t* pbuf, uint8_t num)
{
	uint8_t status = 0;
	uint32_t timeout = 0;
	uint8_t i = 0;
	
	SPI_Set_BaudRate(SPI1, NRF_SPI_SPEED);	//设置SPI1波特率
	NRF_CSN_SELECT();//获取SPI总线
	timeout = NRF_TIMEOUT_VAL;//设置访问超时时间
	status = SPI_RW(NRF_SPI,reg,&timeout);	//选择要写入的寄存器
	if(timeout == 0x00 && status == 0xff)//判断是否超时
	{
		NRF_CSN_DESELECT();	//释放SPI总选
		return NRF_TIMEOUT;	//返回错误
	}
	
	for(i = 0; i < num; i++)
	{
		timeout = NRF_TIMEOUT_VAL;//设置访问超时时间
		status = SPI_RW(NRF_SPI,0x00,&timeout);
		if(timeout == 0x00 && status == 0xff)//判断是否超时
		{
			NRF_CSN_DESELECT();	//释放SPI总选
			return NRF_TIMEOUT;	//返回错误
		}
		*pbuf = status;//存放数据
		pbuf++;
	}
	
	NRF_CSN_DESELECT();//释放SPI总线
	return NRF_SUCCESS;//返回操作成功
}

/**
  * @brief 连续写数据到缓冲区(NRF24L01 FIFO)
  * @param reg : 寄存器地址
	* @param *pbuf : 数据包首地址
	* @param num : 发送的数据个数
  * @retval NRF24L01操作状态
  * @note  该函数主要用于将一组数据放到NRF24L01的发射FIFO缓冲区
	* 	
  */
NRF_StatusTypedef NRF24L01_Check(void)
{
	NRF_StatusTypedef status = NRF_ERROR;
	uint8_t test[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};
	uint8_t result[5] = {0x00,0x00,0x00,0x00,0x00};
	uint8_t i = 0;
	
	//将测试数据写入发送缓冲
	status = NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR, test, 5);
	if(status != NRF_SUCCESS)return status;//NRF24L01通信错误
	
	//从发送缓冲读取数据
	status = NRF24L01_Read_Buf(NRF_READ_REG+TX_ADDR, result, 5);
	if(status != NRF_SUCCESS)return status;//NRF24L01通信错误
	
	//判断读取到的数据是否和发送的数据相同
	for(i = 0; i < 5; i++)
	{
		if(result[i] != test[i])
			return NRF_ERROR;//数据错误
	}
	
	return NRF_SUCCESS;	//NRF24L01通信正常
}

/**
  * @brief 初始化NRF24L01为RX模式
  * @param none
  * @retval none
  * @note 设置RX工作模式，将开启NRF24L01的6个接收通道(channel0~channel5)，并设置相应的通道地址。
	*				在本例程中，我们设置的接收通道地址为:
	*				RX_ADDR_P0: 0x12, 0x34, 0x56, 0x78, 0x9A
	*				RX_ADDR_P1:	0x11, 0x22, 0x33, 0x44, 0x55
	*				RX_ADDR_P2:	0x22, 0x22, 0x33, 0x44, 0x55
	*				RX_ADDR_P3:	0x33, 0x22, 0x33, 0x44, 0x55
	*				RX_ADDR_P4:	0x44, 0x22, 0x33, 0x44, 0x55
	*				RX_ADDR_P5:	0x55, 0x22, 0x33, 0x44, 0x55
	*				如果数据要发送到接收端的某一个通道，则只需要设置发射端的发射地址(TX_ADDR)、接收通道0
	*       地址(RX_ADDR_P0)和对应通道的地址相等即可。
	*				通过这种方式，我们一个接收模块最多可以同时接收6个发射模块的数据，而不会有任何冲突。
	*/
void NRF24L01_RX_Mode(void)
{
	NRF_CE = 0;
	
	//设置接收通道地址
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0, (uint8_t*)RX_ADDRESS0, RX_ADR_WIDTH);//写接收通道0地址
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P1, (uint8_t*)RX_ADDRESS1, RX_ADR_WIDTH);//写接收通道1地址
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P2, (uint8_t*)RX_ADDRESS2, 1);//写接收通道2地址
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P3, (uint8_t*)RX_ADDRESS3, 1);//写接收通道3地址
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P4, (uint8_t*)RX_ADDRESS4, 1);//写接收通道4地址
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P5, (uint8_t*)RX_ADDRESS5, 1);//写接收通道5地址

	//设置接收通道有效数据宽度
	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//接收通道0有效数据宽度
	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P1,RX_PLOAD_WIDTH);//接收通道1有效数据宽度
	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P2,RX_PLOAD_WIDTH);//接收通道2有效数据宽度
	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P3,RX_PLOAD_WIDTH);//接收通道3有效数据宽度
	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P4,RX_PLOAD_WIDTH);//接收通道4有效数据宽度
	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P5,RX_PLOAD_WIDTH);//接收通道5有效数据宽度
	
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA, 0x3F);		//使能所有通道自动ACK应答
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR, 0x3F);//使能所有接收通道
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,40);			//设置工作频道，必须接收端和发射端一致
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP, 0x0F);	//传输速率2Mbps，发射功率0dBm，开启低噪放大器LNA
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0F);		//配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式
	
	NRF_CE = 1; //CE为高,进入接收模式 
}						 

/**
  * @brief 初始化NRF24L01为RX模式
  * @param CHx : 设置发射端对应的接收通道
  * @retval none
  * @note
	*		- 首先将发射地址和RX_ADDR_P0地址设置为对应接收通道的地址。这里配置发射模式的时候，还要
	*			设置接收地址，是因为需要通过接收通道0来接收ACK应答信号。
	*		- 当CE变高后,即进入TX模式,并可以接收数据了
	*		- CE为高大于10us,则启动发送
  */
void NRF24L01_TX_Mode(uint8_t CHx)
{
	uint8_t channel = 0xFF;
	
	channel = CHx;
	NRF_CE = 0;
	
	//默认发射端对应接收端通道1
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)RX_ADDRESS1, RX_ADR_WIDTH);		//写TX地址，需要和对应的接受通道地址相同
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS1, RX_ADR_WIDTH);	//设置接收通道0的地址，用于接收ACK应答信号

	switch(channel)//配置TX对应接收端的接收通道(接收端共有6个通道)
	{
		case 0x00: //接收通道0
		NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)RX_ADDRESS0, RX_ADR_WIDTH);		//写TX地址，需要和对应的接受通道地址相同
		NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS0, RX_ADR_WIDTH);	//设置接收通道0的地址，用于接收ACK应答信号
		break;
		
		case 0x01: //接收通道1
		NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)RX_ADDRESS1, RX_ADR_WIDTH);		//写TX地址，需要和对应的接受通道地址相同
		NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS1, RX_ADR_WIDTH);	//设置接收通道0的地址，用于接收ACK应答信号
		break;
		
		case 0x02: //接收通道2，注意通道2~5的地址都只能设置最低字节，另外4个高字节和通道1相同
		NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)RX_ADDRESS2, 1);
		NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS2, 1);
		break;
		
		case 0x03: //接收通道2，注意通道2~5的地址都只能设置最低字节，另外4个高字节和通道1相同
		NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)RX_ADDRESS3, 1);
		NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS3, 1);
		break;
		
		case 0x04: //接收通道2，注意通道2~5的地址都只能设置最低字节，另外4个高字节和通道1相同
		NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)RX_ADDRESS4, 1);
		NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS4, 1);
		break;
		
		case 0x05: //接收通道2，注意通道2~5的地址都只能设置最低字节，另外4个高字节和通道1相同
		NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)RX_ADDRESS5, 1);
		NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS5, 1);
		break;
		
		default:
			break;
	}

	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA, 0x01);     //使能通道0的自动应答    
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR, 0x01); //使能通道0的接收地址
	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0, RX_PLOAD_WIDTH);//接收通道0有效数据宽度
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR, 0x1A);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH, 40);       //设置RF通道为40
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP, 0x0F);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0E);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	
	NRF_CE = 1;//CE为高,10us后启动发送
}	

/**
  * @brief 发送一个数据包
  * @param buf : 待发送数据首地址
  * @retval 发送完成状态
  * @note 
  */
uint8_t NRF24L01_TxPacket(uint8_t *buf)
{
	uint8_t status = 0;
	
	NRF_CE = 0;
	NRF24L01_Write_Buf(NRF_TX_PLOAD, buf, TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
 	NRF_CE = 1;					//启动发送	   
	while(NRF_IRQ != 0);//等待发送完成
	NRF24L01_Read_Reg(NRF_READ_REG+STATUS, &status);  //读取状态寄存器的值	   
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS, status); //清除TX_DS或MAX_RT中断标志
	if(status & MAX_TX)//达到最大重发次数
	{
		NRF24L01_Write_Reg(NRF_FLUSH_TX, 0xff);//清除TX FIFO寄存器 
		return MAX_TX; 
	}
	if(status & TX_OK)//发送完成
	{
		return TX_OK;
	}
	return 0xff;//其他原因发送失败
}

/**
  * @brief 接收一个数据包
  * @param buf : 接收到的数据包存放地址
  * @retval 6~255 - 未接收到, 0~5 - 接收到通道几的数据包
  * @note 
  */
uint8_t NRF24L01_RxPacket(uint8_t *buf)
{
	uint8_t status = 0;
	uint8_t rx_channel = 0xFF;

	NRF24L01_Read_Reg(NRF_READ_REG+STATUS, &status);  //读取状态寄存器的值    	 
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS, status); //清除RX_DR中断标志
	if(status & RX_OK)//接收到数据
	{
		NRF24L01_Read_Buf(NRF_RX_PLOAD, buf, RX_PLOAD_WIDTH);//读取数据
		NRF24L01_Write_Reg(NRF_FLUSH_RX, 0xff);//清除RX FIFO寄存器 
		rx_channel = (status>>1)&0x07;
		return rx_channel;
	}
	return 255;//没收到数据
}


/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
