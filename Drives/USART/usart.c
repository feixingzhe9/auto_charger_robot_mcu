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
#include "usart.h"
#if COM_DMA_TRANSFER
#include "dma.h"
#endif

#if COM_RX_LEN
/* COM_RX_STA: 串口接收状态字
 * bit15，	接收完成标志
 * bit14，	接收到0x0d
 * bit13~0，	接收到的有效字节数目
*/
uint16_t COM_RX_STA = 0;

/* 串口接收缓冲 */
//uint8_t COM_RX_BUF[COM_RX_LEN];
#endif

/******************************* @printf  *************************************/

#ifdef PRINTF_SUPPORT //以下代码,支持printf函数,而不需要选择use MicroLIB
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
int _sys_exit(int x) 
{ 
	return x;
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
    while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET);
    USART_SendData(USART3, (uint8_t)ch);
    return ch;
}
#endif
/********************************* @end ***************************************/

/**
  * @brief 定义串口相关PORT，CLK等
  */
USART_TypeDef* COM_USART[COMn] = {UCORTEX_COM1, UCORTEX_COM2, UCORTEX_COM3}; 

GPIO_TypeDef* COM_TX_PORT[COMn] = {UCORTEX_COM1_TX_GPIO_PORT, UCORTEX_COM2_TX_GPIO_PORT, UCORTEX_COM3_TX_GPIO_PORT};
 
GPIO_TypeDef* COM_RX_PORT[COMn] = {UCORTEX_COM1_RX_GPIO_PORT, UCORTEX_COM2_RX_GPIO_PORT, UCORTEX_COM3_RX_GPIO_PORT};
 
const uint32_t COM_USART_CLK[COMn] = {UCORTEX_COM1_CLK, UCORTEX_COM2_CLK, UCORTEX_COM3_CLK};

const uint32_t COM_TX_PORT_CLK[COMn] = {UCORTEX_COM1_TX_GPIO_CLK, UCORTEX_COM2_TX_GPIO_CLK, UCORTEX_COM3_TX_GPIO_CLK};
 
const uint32_t COM_RX_PORT_CLK[COMn] = {UCORTEX_COM1_RX_GPIO_CLK, UCORTEX_COM2_RX_GPIO_CLK, UCORTEX_COM3_RX_GPIO_CLK};

const uint16_t COM_TX_PIN[COMn] = {UCORTEX_COM1_TX_PIN, UCORTEX_COM2_TX_PIN, UCORTEX_COM3_TX_PIN};

const uint16_t COM_RX_PIN[COMn] = {UCORTEX_COM1_RX_PIN, UCORTEX_COM2_RX_PIN, UCORTEX_COM3_RX_PIN};

const uint8_t COM_IRQn[COMn] = {UCORTEX_COM1_IRQn, UCORTEX_COM2_IRQn, UCORTEX_COM3_IRQn};

#if COM_DMA_TRANSFER
DMA_TypeDef* COM_DMA[COMn] = {UCORTEX_COM1_DMA, UCORTEX_COM2_DMA, UCORTEX_COM3_DMA}; 

DMA_Channel_TypeDef* COM_DMA_CH[COMn][2] = {{UCORTEX_COM1_RX_DMA_CH, UCORTEX_COM1_TX_DMA_CH},
																						{UCORTEX_COM2_RX_DMA_CH, UCORTEX_COM2_TX_DMA_CH},
																						{UCORTEX_COM3_RX_DMA_CH, UCORTEX_COM3_TX_DMA_CH}};
																						
const uint32_t COM_DR_BASE[COMn] = { UCORTEX_COM1_DR_Base, UCORTEX_COM2_DR_Base, UCORTEX_COM3_DR_Base};

#endif


/**
  * @brief  初始化串口GPIO
  * @param  COM: 指定要初始化的COM PORT   
  *     @arg COM1 串口1
  *     @arg COM2 串口2
  *     @arg COM3 串口3
  * @retval None
  * @note None
  */
void COM_GPIOInit(COM_TypeDef COM)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 开启GPIO时钟和复用功能时钟RCC_APB2Periph_AFIO */
  RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);

  /* 配置 USART Tx 复用推挽输出 */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

  /* 配置 USART Rx 浮空输入 */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);
}

/**
  * @brief  配置串口
  * @param  COM: 指定要初始化的COM PORT   
  *     @arg COM1 串口1
  *     @arg COM2 串口2
  *     @arg COM3 串口3
  * @param  Baudrate 串口波特率
  * @retval None
  * @note None
  */
void COM_Init(COM_TypeDef COM, uint32_t BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
  COM_GPIOInit(COM);//初始化串口GPIO

  /* 开启串口时钟，注意串口1挂载在APB1外设总线下，其他串口挂载APB2外设总线下*/
  if (COM == COM1)
    RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE); //开启串口时钟
  else
    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);	//开启串口时钟
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置优先级组2
	/* USARTx NVIC配置信息 */
	NVIC_InitStructure.NVIC_IRQChannel = COM_IRQn[COM];
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;				//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;						//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据上面的配置信息初始化NVIC寄存器
	
	
  /*    串口配置信息
	      -------------
        - 波特率根据参数指定
        - 数据长度8bit
        - 1个停止位
        - 无奇偶校验位
        - 无硬件数据流控制(RTS 和 CTS 信号)
        - 允许串口发送和接收
  */
  USART_InitStructure.USART_BaudRate = BaudRate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx;
  USART_Init(COM_USART[COM], &USART_InitStructure);			//根据串口配置信息初始化串口 

	USART_ITConfig(COM_USART[COM], USART_IT_RXNE, ENABLE);//开启串口接收中断
  USART_Cmd(COM_USART[COM], ENABLE);										//开启串口
	
}







void print_uart_init(u32 bound)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);  //使能USART3
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    USART_DeInit(USART3);   //复位串口3
    //USART3_TX    PB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB10

    //USART3_RX	  PB.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//PB11PA10


    //USART 初始化设置

    USART_InitStructure.USART_BaudRate = bound;//
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART3, &USART_InitStructure); //初始化串口
#if EN_USART1_RX		  //如果使能了接收
    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
#endif
    USART_Cmd(USART3, ENABLE);                    //使能串口

}





#if COM_DMA_TRANSFER
/**
  * @brief  轮询方式(Polling)串口DMA初始化
  * @param  COM: 要设置的串口 
  *     @arg COM1 串口1(USART1)
  *     @arg COM2 串口2(USART2)
  *     @arg COM3 串口3(USART3)
  * @param  MemoryBaseAddr : DMA缓冲地址（一般是我们定义的缓冲数组地址）
	* @param  BufferSize : 缓冲大小
	* @param  DMA_DIR : DMA传输方向
  *     @arg DMA_DIR_PeripheralDST : 串口发送DMA，即 Buffer->USART 的DMA传输
  *     @arg DMA_DIR_PeripheralSRC : 串口接收DMA，即 USART->Buffer 的DMA传输
  * @retval None
  * @note 在本函数中，打开了相应的串口DMA接口，但并未启动DMA传输，当需要启动DMA传输时：
	*				请调用DMA_Polling_Enable(DMA_Channel_TypeDef* DMAy_Channelx, uint16_t DataNumber)
  */
void COM_DMA_Polling_Init(COM_TypeDef COM, uint32_t MemoryBaseAddr, uint32_t BufferSize, uint32_t DMA_DIR)
{
	//串口对应的DMA
	DMA_TypeDef* DMAy = COM_DMA[COM];
	//对应的DMA通道（注意串口接收和发送对应不同的DMA通道）
	DMA_Channel_TypeDef* DMAy_Channelx = COM_DMA_CH[COM][DMA_DIR>>4];
	//串口数据寄存器地址
	uint32_t PeripheralBaseAddr = COM_DR_BASE[COM];
	//数据位宽: 0 -> 8bit, 1 -> 16bit, 2-> 32bit
	uint32_t DataType = 0; //8-bit位宽
	
	//步骤一、配置对应的DMA通道
	DMA_Polling_Init(DMAy, DMAy_Channelx, PeripheralBaseAddr, MemoryBaseAddr, BufferSize, DataType, DMA_DIR, DMA_Mode_Normal);
	
	//步骤二、打开串口DMA发送或接收接口
	if(DMA_DIR==DMA_DIR_PeripheralDST)//打开串口DMA发送
		USART_DMACmd(COM_USART[COM], USART_DMAReq_Tx, ENABLE);
	
	if(DMA_DIR==DMA_DIR_PeripheralSRC)//打开串口DMA接收
		USART_DMACmd(COM_USART[COM], USART_DMAReq_Rx, ENABLE);
}

#endif

	
/**********************************************************
*	功能：发送数据
*	参数：buf 发送缓冲区首地址
*		  len 待发送的字节数
**********************************************************/
void RS232_Send_Data(u8 *buf,u8 len)
{
	u8 t;
	
  for(t=0;t<len;t++)		//循环发送数据
	{		   
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	//发送完成  
		USART_SendData(USART1,buf[t]);
	}	 
 
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
//	COM1_RX_CNT=0;	  
}


//***********************串口发DMA配置**************************//
void COM_DMA_send_init(uint8_t* send_buffer,uint32_t BaudRate)
{
	DMA_InitTypeDef 		DMA_InitStructure;
	USART_InitTypeDef 	USART_InitStructure;
	NVIC_InitTypeDef 		NVIC_InitStructure;
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE);     		//打开串口对应外设时钟   
    //??DMA??  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  
    //DMA??????  
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  
	
		/*  串口配置信息
			-------------
			- 波特率根据参数指定
			- 数据长度8bit
			- 1个停止位
			- 无奇偶校验位
			- 无硬件数据流控制(RTS 和 CTS 信号)
			- 允许串口发送和接收
	*/
	//USART_InitStructure.USART_BaudRate = DEFAULT_BAUD;    
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;    
	USART_InitStructure.USART_StopBits = USART_StopBits_1;    
	USART_InitStructure.USART_Parity = USART_Parity_No;    
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;    
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;      
	USART_InitStructure.USART_BaudRate = BaudRate;   
	
	USART_Init(USART1,&USART_InitStructure);    				//初始化串口
	
    DMA_DeInit(DMA1_Channel4);  //DMA1通道4配置
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);  	//外设地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)send_buffer;  		//内存地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  							//DMA传输方向单向
    DMA_InitStructure.DMA_BufferSize = 100;  													//设置DMA在传输时缓冲区的长度
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//设置DMA的外设递增模式，不递增  
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  					//设置DMA的内存递增模式，递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //外设数据单位——byte
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;  		 //内存数据单位——byte
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  										//DMA传输模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;  							//DMA优先级别
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  										//禁止DMA中的两个memory互相访问
    DMA_Init(DMA1_Channel4,&DMA_InitStructure);  
    DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);  
 	
		//采用DMA方式发送
    USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);      
    //使能DMA1通道4
//    DMA_Cmd(DMA1_Channel4, ENABLE);  
}

/***********DMA发送调用函数*************/
void COM1_DMA_send_data(uint8_t length)
{
	DMA_Cmd(DMA1_Channel4, DISABLE );	//关闭DMA通道  
 	DMA_SetCurrDataCounter(DMA1_Channel4, length);//设置本次DMA传输的数据量
 	DMA_Cmd(DMA1_Channel4, ENABLE);		//打开DMA通道
}

uint8_t Uart_Tx[100];

/***********初始化COM_DMA*************/
void COM1_DMA_init(uint32_t BaudRate)
{	
	COM_GPIOInit(COM1);
	COM_DMA_send_init(Uart_Tx, 115200);
//	COM_DMA_receive_init(Uart_Rx, 115200);	
}



