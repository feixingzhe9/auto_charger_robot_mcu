/*******************************************************************************
 * @name    : SPI驱动
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.0
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : SPI驱动文件
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
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-06    更改人：布谷鸟
 * 版本记录：V1.0
 * 更改内容：新建
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
 
#include "spi.h" 


/**
  * @brief  初始化SPI1 GPIO
  * @param  None
  * @retval None
	* @nete		
	*		SPI1_CLK  : PA5
	*		SPI1_MISO : PA6
	*		SPI1_MOSI : PA7
  */
void SPI1_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	
	//使能SPI1 GPIO时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  
	//配置 SPI1_CLK(PA5), SPI1_MOSI(PA7)为辅助功能推挽输出
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//辅助功能推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//根据上面配置信息初始化

  //配置 SPI1_MISO(PA6)为浮空输入
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//根据上面配置信息初始化
}

/**
  * @brief SPI1初始化
  * @param None
  * @retval None
  * @note 
	*		SPI1挂接在APB2外设总线，因此外设总线频率为72MHz，初始化函数中，我们配置SPI
	*		波特率预分频是8，因此波特率 = 72MHz/16 = 4MHz；特别注意SPI2是挂接在APB1外设
	*		总线的，APB1的外设总线是36MHz，因此同样的预分频得到的波特率变为2MHz。
  */
void SPI1_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //使能SPI1外设始终
	
  SPI1_GPIO_Init();// SPI1 GPIO初始化

  //SPI1初始化
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//双线双向全双工模式
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;			//设置为SPI主设备
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	//数据为8bit位宽数据帧
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;				//SPI时钟信号空闲状态为高电平
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;			//SPI时钟第二个跳变(上升沿/下降沿)采样数据
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;					//软件管理选通管脚(硬件管理则由SPI对应的NSS管脚)
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;//波特率预分频,PCLK2/8=9MHz
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//数据从MSB开始传输
  SPI_InitStructure.SPI_CRCPolynomial = 7;					//定义CRC值计算用的多项式
  SPI_Init(SPI1, &SPI_InitStructure);								//根据以上参数初始化SPI1

  SPI_Cmd(SPI1, ENABLE);	//开启SPI1
}

/**
  * @brief  SPI读写，读和写共用
  * @param  SPIx: 选择SPI端口，SPI1或SPI2...
	* @param	data: 待发送的数据，如果接收数据是，则发送一个无效数据(dummy data)
	* @param	timeout: 等待超时时间
  * @retval 接收到的数据
  */
uint8_t SPI_RW(SPI_TypeDef* SPIx, uint8_t data, uint32_t* timeout)
{
  //等待数据寄存器为空
  while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
	{
		//只要发现timeout=0，且返回值为0XFF，则说明传输超时
		*timeout -= 1;
		if(*timeout == 0) return 0XFF;
	}
  SPI_I2S_SendData(SPIx, data);//向SPI写一个数据
	

  //等待从SPI读一个数据
  while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)
	{
		//只要发现timeout=0，且返回值为0XFF，则说明传输超时
		*timeout -= 1;
		if(*timeout == 0) return 0XFF;
	}
  return SPI_I2S_ReceiveData(SPIx);//返回读到的数据
}


/**
  * @brief  设置SPI波特率
  * @param  SPIx: 选择SPI端口，SPI1或SPI2...
	* @param	SPI_BaudRatePrescaler: 波特率预分频
  *		@arg - SPI_BaudRatePrescaler_2   2分频   
  *		@arg - SPI_BaudRatePrescaler_8   8分频   
  *		@arg - SPI_BaudRatePrescaler_16  16分频  
  *		@arg - SPI_BaudRatePrescaler_256 256分频 
  * @retval none
	* @note: 波特率 =  PCLK / SPI_BaudRatePrescaler
  */
void SPI_Set_BaudRate(SPI_TypeDef* SPIx, uint16_t SPI_BaudRatePrescaler)
{
	uint16_t tmpreg;
	
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	
	tmpreg = SPIx->CR1;
	tmpreg &= 0XFFC7;	//清除BR位
	tmpreg |= (uint16_t)SPI_BaudRatePrescaler; //设置波特率
	SPIx->CR1 = tmpreg;
	
//	SPI_Cmd(SPIx,ENABLE); 

}

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
