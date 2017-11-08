/*******************************************************************************
 * @name    : ADC底层驱动
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.2
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 设置PA0为模拟输入端口，使能内部STM32内部温度传感器
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
 * PA0 -> ADC1_CH0
 * TEMP-> ADC1_CH16
 *
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-06    更改人：布谷鸟
 * 版本记录：V1.0
 * 更改内容：新建
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-06    更改人：布谷鸟
 * 版本记录：V1.1
 * 更改内容：在ADC_Initialize中添加了开启STM32内部温度传感器：
 * ADC_TempSensorVrefintCmd(ENABLE); //开启内部温度传感器，不需要开启时请注释掉
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-07    更改人：布谷鸟
 * 版本记录：V1.2
 * 更改内容：添加了ADC DMA工作方式：void ADC1_DMAInitialize(void)
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
#include "adc.h"
#include "delay.h"
#if ADC_DMA_TRANSFER
#include "dma.h"
__IO uint16_t ADC_Buffer[2];
#endif

/**
  * @brief ADC初始化
	* @param none
  * @retval none
  * @note  初始化PA.00为ADC1_CH1，单次转换，软件触发ADC转换
  */ 
void ADC_Initialize(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 使能GPIOA，ADC1,AFIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
	
	/* 设置ADCCLK分频因子 ADCCLK = PCLK2/6，即 72MHz/6 = 12MHz */
  RCC_ADCCLKConfig(RCC_PCLK2_Div6);

  /* 配置 PA.00 (ADC1_IN0) 作为模拟输入引脚 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	ADC_DeInit(ADC1);  //将ADC1设为缺省值
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;				//单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//单次转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//软件触发ADC转换
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//规则转换通道数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct初始化ADC
	
	ADC_TempSensorVrefintCmd(ENABLE); //开启内部温度传感器，不需要开启时请注释掉这一句
	
	ADC_Cmd(ADC1, ENABLE);	//使能ADC1
	
	ADC_ResetCalibration(ADC1);	//复位ADC校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	ADC_StartCalibration(ADC1);	 //开启AD校准
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束 
}

/**
  * @brief 获得ADC值
  * @param ADCx: ADC1 / ADC2 / ADC3
  * @param ch:通道值 ADC_Channel_0 ~ ADC_Channel_17
  * @retval ADC转换值
  * @note 
  */
uint16_t Get_Adc(ADC_TypeDef* ADCx, uint8_t ch)   
{
	/* ADC的规则通道组设置：一个序列，采样时间239.5周期	*/
	ADC_RegularChannelConfig(ADCx, ch, 1, ADC_SampleTime_239Cycles5 );
	
	ADC_SoftwareStartConvCmd(ADCx, ENABLE);					//开始转换
	while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC ));	//等待转换结束
	return ADC_GetConversionValue(ADCx);						//返回ADC1转换结果
}

/**
  * @brief 获取连续10次ADC采样的平均值
  * @param ADCx: ADC1 / ADC2 / ADC3
  * @param ch:通道值 ADC_Channel_0 ~ ADC_Channel_17
  * @retval ADC转换的平均值
  * @note 
  */
uint16_t Get_Adc_Average(ADC_TypeDef* ADCx, uint8_t ch)
{
	uint32_t adc_val = 0;
	uint8_t i;
	
	for(i = 0; i < 10; i++)
	{
		adc_val += Get_Adc(ADCx, ch);
		delay_ms(5);
	}
	return adc_val/10;
}

#if ADC_DMA_TRANSFER
/**
  * @brief ADC1 DMA初始化
	* @param none
  * @retval none
  * @note  初始化PA.00(ADC1_CH1),	STM32内部温度传感器(ADC1_CH16)，连续转换，DMA传输
  */ 
void ADC1_DMAInitialize(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 使能GPIOA，ADC1，DMA1时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

  /* ADC1_IN0 GPIO (PA.00) 配置 --------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//配置 PA.00 (ADC1_IN0) 作为模拟输入引脚
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  /* ADC1 配置 ------------------------------------------------------*/
  RCC_ADCCLKConfig(RCC_PCLK2_Div6);//设置ADCCLK分频因子：72MHz/6 = 12MHz 
	
	ADC_DeInit(ADC1);  //将ADC1设为缺省值
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//独立模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;				//多通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	//连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//软件触发ADC转换
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 2;	//规则转换通道数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct初始化ADC

  ADC_TempSensorVrefintCmd(ENABLE);//开启ADC内置温度传感器CH16
	
	//常规转换序列1：ADC1_CH1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_13Cycles5);
	//常规转换序列2：ADC1_CH16（内部温度传感器）
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_239Cycles5);

	ADC_Cmd(ADC1, ENABLE);	//使能ADC1

	ADC_ResetCalibration(ADC1);	//复位ADC校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1)){}	//等待复位校准结束
	ADC_StartCalibration(ADC1);	 //开启AD校准
	while(ADC_GetCalibrationStatus(ADC1)){}//等待校准结束 
		
  /*    DMA配置信息
	      -------------
				- DMA通道: DMA1_Channel1
        - 外设数据寄存器地址: ADC1_DR_Address
        - 缓冲器地址: ADC_Buffer
				- 缓冲大小: 2
        - 数据类型: 1 -> 16bit
        - DMA方向: ADC数据寄存器->ADC_Buffer
        - DMA工作模式: 循环传输
  */
		
	DMA_Polling_Init(DMA1, DMA1_Channel1,
									(uint32_t)ADC1_DR_Address,(uint32_t)ADC_Buffer,
									2,1,
									DMA_DIR_PeripheralSRC,DMA_Mode_Circular);
	
	ADC_DMACmd(ADC1, ENABLE);//开启ADC的DMA支持
	DMA_Cmd(DMA1_Channel1,ENABLE);//开启DMA Channel1
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);//ADC自动连续转换，结果自动保存在ADC_Value
}

#endif

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
