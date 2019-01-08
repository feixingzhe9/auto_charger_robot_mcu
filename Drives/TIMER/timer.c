/*******************************************************************************
 * @name    : 定时器配置
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.2
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 介绍如何配置定时器和编写定时中断服务程序
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
 * 更改时间：2014-05-02    更改人：布谷鸟
 * 版本记录：V1.1
 * 更改内容：添加TIM4_CH3 PWM输出配置
 * ----------------------------------------------------------------------------
 * 更改时间：2014-05-02    更改人：布谷鸟
 * 版本记录：V1.2
 * 更改内容：添加TIM2_CH1输入捕获配置
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
 
#include "timer.h" 

/**
  * @brief 定时器3(TIM3)中断初始化
  * @param arr：自动重装值。
  * @param psc：时钟预分频数
  * @retval none
  * @note
	*    配置定时器3(TIM3)为向下计数模式
  *    定时器计数频率 = PCLK / ( psc + 1 )
  *    定时器中断周期 = ( arr + 1 )*( pac + 1) / PCLK
  */
void Timer1_Init(uint16_t arr, uint16_t psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //使能外设时钟
	
	TIM_TimeBaseStructure.TIM_Period = arr;		//设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = psc;	//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;			//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);			//根据指定的参数初始化TIMx的时间基数单位
	
	TIM_ARRPreloadConfig(TIM3,ENABLE);//使能ARR预装载，防止向上计数时更新事件异常延迟
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );	//允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;						//TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;	//先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;				//从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;						//IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);	//初始化NVIC寄存器

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级

	TIM_Cmd(TIM3, ENABLE);  //使能TIM3
}

//CaptureStatus : Bit15 - 完成捕获标志
//                Bit14 - 捕捉到上升沿标志
//                Bit13 - 溢出错误标志
//                Bit12 - 保留位
//                Bit11..0 - 更新事件发生次数
__IO uint16_t CaptureStatus = 0;//捕获状态
__IO uint16_t CaptureValue1 = 0;//上升沿时的捕获值
__IO uint16_t CaptureValue2 = 0;//下降沿时的捕获值

void TIM2_Capture_Init(uint16_t arr,uint16_t psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//打开GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//打开TIM2时钟

  //设置PA0为浮空输入，注意开发板上PA0外部有10K的下拉电阻
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  //配置TIM2中断通道，设置系统中断组为2，抢占优先级为0，子优先级为2
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;//中断号
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;//子优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);//根据配置参数初始化对应的中断通道
	
	//注意在同一个项目中，中断组必须是一样的！
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置NVIC中断分组2:2位抢占优先级，2位响应优先级


	  /* TIM2 时基模块初始化 ---------------------
     定时器计数频率Fck_cnt = PCLK/(psc+1)
		 定时器更新频率Freq = PCLK/((psc+1)*(arr+1))
  ------------------------------------------------------------ */
	TIM_TimeBaseStructure.TIM_Period = arr;		//设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = psc;	//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM向下计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);	//根据指定的参数初始化TIMx的时间基数单位
	
	TIM_ARRPreloadConfig(TIM2,ENABLE);//使能ARR预装载，防止向上计数时更新事件异常延迟

	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE );	//允许更新中断
	
	  /* TIM2 configuration: Input Capture mode ---------------------
     The external signal is connected to TIM2 CH1 pin (PA0)  
     The Rising edge is used as active edge,
     The TIM2 CCR1 is used to compute the frequency value 
  ------------------------------------------------------------ */

  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;
  TIM_ICInit(TIM2, &TIM_ICInitStructure);
  
  /* TIM enable counter */
  TIM_Cmd(TIM2, ENABLE);

  /* Enable the CC1 Interrupt Request */
  TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
	
}
/********************************************************************
* Function Name  : TIM2_PWM_Init(u16 arr,u16 psc)
* Function       : TIM2???CH1?PWM?????
* parameter      : arr - ?????
				   psc - ??????	
* Description    : ??f = 72M/[(psc+1)*(arr+1)]               
* Return         : void
//pb8 tim4ch3
*********************************************************************/
void TIM4_CH1_PWM_Init(u16 arr,u16 psc)
{  
	/*********使能初始化定时器1时钟*************/
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//使能定时器1时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIOB和AFIO复用功能时钟   	
	
	/*IO设置*/
//	GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);
	
 	/***************初始化GPIO*****************/
	//配置PB10为复用推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //PE10，对应TIM1_CH2N
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO

 	/*************定时器基本配置***************/	
   //初始化TIM1
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	TIM_ARRPreloadConfig(TIM4,ENABLE);//使能ARR预装载，防止向上计数时更新事件异常延迟

 	/*************设置占空比，将TIM1_CH2N配置成pwm输出***************/
	//初始化TIM1_CH2N PWM输出
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性低
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low; //输出互补极性:TIM输出比较互补极性高
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;//使能互补端输出
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //将配置参数初始化外设TIM1 OC1

	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);  //使能TIM1在CCR1上的预装载寄存器
 
	TIM_Cmd(TIM4, ENABLE);  //使能TIM1
  TIM_CtrlPWMOutputs(TIM4,ENABLE);	
}


/**
  * @brief 设置PWM占空比
  * @param duty : 占空比，如50%，则参数为50
  * @retval none
  * @note
	* 		duty = 100*CCR3/(ARR+1)
  */
void PWM_SetDuty2(uint8_t duty)
{
	uint16_t compare;
	compare = duty*(1985+1)/100;//计算比较值
	TIM4->CCR1 = compare;//更新比较值
}

void SendData2(u8 SysNum, u8 KeyNum)
{ 
	u8 i; 
	u32 data = 0;
//	u32 data = 0xED12BF40; 
	data = ((~KeyNum & 0xFF)<<24) + ((KeyNum & 0xFF)<<16) + ((~SysNum & 0xff)<< 8) + SysNum;

//	LED = 1;
//	TIM_Cmd(TIM4, ENABLE); 
	PWM_SetDuty2(70);
	delay_us(9000); 
	PWM_SetDuty2(100);
//	LED = 0;
//	TIM_Cmd(TIM4, DISABLE); 
	delay_us(4500);
	PWM_SetDuty2(70);
//	LED = 1;
//	TIM_Cmd(TIM4, ENABLE); 	
	
	for(i=0; i<32; i++) 
	{ 
		delay_us(560);//480
	PWM_SetDuty2(100);
//		LED = 0;
//		TIM_Cmd(TIM4, DISABLE); 
		
		if(((data >> i) & 0x00000001) == 0) 
		{   
			delay_us(560);   //0.56ms 620
		} 
		else 
		{  
			delay_us(1690); //1.69ms 
		} 
	PWM_SetDuty2(70);
//		LED = 1;
//		TIM_Cmd(TIM4, ENABLE); 
	} 
	PWM_SetDuty2(100);
	delay_us(560); 
	PWM_SetDuty2(70);
	delay_us(560); 
	PWM_SetDuty2(100);
//	TIM_Cmd(TIM4, DISABLE); 
//	  LED = 0;
} 

/*
void SendData2(u8 SysNum, u8 KeyNum) 
{ 
	u8 i; 
	u32 data = 0;
//	u32 data = 0xED12BF40; 
	data = ((~KeyNum & 0xFF)<<24) + ((KeyNum & 0xFF)<<16) + ((~SysNum & 0xff)<< 8) + SysNum;

//	LED = 1;
//	TIM_Cmd(TIM4, ENABLE); 
	PWM_SetDuty2(70);
	delay_us(9000); 
	PWM_SetDuty2(0);
//	LED = 0;
//	TIM_Cmd(TIM4, DISABLE); 
	delay_us(4500);
	PWM_SetDuty2(70);
//	LED = 1;
//	TIM_Cmd(TIM4, ENABLE); 	
	
	for(i=0; i<32; i++) 
	{ 
		delay_us(560);//480
	PWM_SetDuty2(0);
//		LED = 0;
//		TIM_Cmd(TIM4, DISABLE); 
		
		if(((data >> i) & 0x00000001) == 0) 
		{   
			delay_us(560);   //0.56ms 620
		} 
		else 
		{  
			delay_us(1690); //1.69ms 
		}
		if(i != 31)
		{			
			PWM_SetDuty2(70);
		}
//		LED = 1;
//		TIM_Cmd(TIM4, ENABLE); 
	} 
//	PWM_SetDuty2(100);
//	delay_us(560); 
//	PWM_SetDuty2(70);
//	delay_us(560); 
	PWM_SetDuty2(0);
//	TIM_Cmd(TIM4, DISABLE); 
//	  LED = 0;
} 
*/
/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
