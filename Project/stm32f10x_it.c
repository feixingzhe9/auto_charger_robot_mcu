/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"
#include "led.h"
#include "rtc.h"
#include "timer.h"
#include "global.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

extern uint32_t sys_tick_cnt;
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    sys_tick_cnt++;
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  实时时钟RTC秒中断服务程序
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    RTC_ClearITPendingBit(RTC_IT_SEC);//清除RTC秒中断标志位
    LED=!LED;							//LED翻转
    TimeDisplay = 1;			//时间更新标志置1
    RTC_WaitForLastTask();//等待RTC寄存器操作完成 
  }
}

/**
  * @brief  EXTI0中断服务程序
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
		LED=!LED;
    EXTI_ClearITPendingBit(EXTI_Line0);//清除EXTI line0的中断标志
  }
}


static uint8_t Uart1_Rx = 0;          
static uint8_t Uart1_head = 0;    
static uint8_t Uart1_Len = 0;

#define USART_LENGTH 6

/**
  * @brief  USART1串口中断
  * @param  None
  * @retval None
  */
void USART1_IRQHandler()
{
	uint8_t check = 0;
	uint8_t i = 0;
//	  static uint8_t Uart1_Buffer[64] = {0};
		
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET) 
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);       
		COM_RX_BUF[Uart1_Rx] = USART_ReceiveData(USART1);        
		Uart1_Rx++;
		Uart1_Rx &= 0xFF;
	}
	
	if(COM_RX_BUF[Uart1_head] != 0x5A) //判断头    
	{
		memset(COM_RX_BUF,0,Uart1_Rx);
		Uart1_Rx = 0;
		return;  
	}      
   
	Uart1_Len = Uart1_Rx - Uart1_head; //??  
	if(Uart1_Len > USART_LENGTH)
	{
		memset(COM_RX_BUF,0,Uart1_Rx);
		Uart1_Rx = 0;
		Uart1_Len = 0;
		return;
	}
		
	if(COM_RX_BUF[Uart1_Rx-1] == 0xA5)
	{ 
		Uart1_Len = Uart1_Rx - Uart1_head; //??  
		for(i = 0;i < USART_LENGTH-2;i++)
		{
			check += COM_RX_BUF[i];
		}
		
		if((Uart1_Len != USART_LENGTH)||(COM_RX_BUF[USART_LENGTH-2] != check))
		{
			memset(COM_RX_BUF,0,Uart1_Rx);
			Uart1_Rx = 0;
			Uart1_Len = 0;
			return;
		}
		Uart1_Rx = 0;
		Uart1_Len = 0;    
		if((COM_RX_BUF[1] > 2)||(COM_RX_BUF[2] > 100))
//		if(COM_RX_BUF[1] > 2)
		{
			COM1_RX_STATE = COM1_RX_GET_WRONG;		
			memset(COM_RX_BUF,0,Uart1_Rx);
			return;
		}
		else
		{
			COM1_RX_STATE = COM1_RX_GET_RIGHT;	
			power_ctl.control_flag = COM_RX_BUF[1];
			power_ctl.vol 				 = COM_RX_BUF[2];
			USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//关闭串口1接收中断
		}
	}      
	
	if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) == SET) //判断是否溢出    
	{       
		USART_ClearFlag(USART1,USART_FLAG_ORE); //?SR     
		USART_ReceiveData(USART1); //?DR    
	}    
		

}

/**
  * @brief  USART2串口中断
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
	uint8_t Res;
	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)	//接收中断
	{
		Res = USART_ReceiveData(USART2);	//读取接收到的数据
		USART_SendData(USART2, Res);			//将接收到的数据发送出去  		 
	}
}

/**
  * @brief  USART3串口中断
  * @param  None
  * @retval None
  */
void USART3_IRQHandler(void)
{
	uint8_t Res;
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)	//接收中断
	{
		Res = USART_ReceiveData(USART3);	//读取接收到的数据
		USART_SendData(USART3, Res);			//将接收到的数据发送出去  		 
	}
}

/**
  * @brief  This function handles TIM3 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) //发生了更新中断
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);//清除更新中断标志
		if(CaptureStatus&0X4000)//捕获到上升沿
		{
			if((CaptureStatus&0X0FFF)>=0X0FFF)//溢出，捕获的时间太长了
			{
				CaptureStatus|=0X2000;//设置溢出标志
			}
			else
			{
				CaptureStatus++;//更新事件次数累加
			}
		}
	}
	
  if(TIM_GetITStatus(TIM2, TIM_IT_CC1) == SET) 
  {
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);//清除捕获比较中断标志
		
		if(CaptureStatus&0X8000)//上次捕获值未被使用，丢弃掉上次捕获的值
		{
			CaptureStatus = 0;
			CaptureValue1 = 0;
			CaptureValue2 = 0;
		}
		
    if((CaptureStatus&0X4000) == 0)//捕捉到上升沿
    {
      CaptureValue1 = TIM_GetCapture1(TIM2);//记录捕获值
      CaptureStatus |= 0X4000;//标志捕捉到上升沿
			TIM_OC1PolarityConfig(TIM2,TIM_ICPolarity_Falling);//设置下降沿捕获
    }
    else if(CaptureStatus&0X4000)//捕获到下降沿
    {
      CaptureValue2 = TIM_GetCapture1(TIM2); //记录捕获值
      CaptureStatus &= ~0X4000;	//清除上升沿标志
			CaptureStatus |= 0X8000;	//设置捕获完成标志
			TIM_OC1PolarityConfig(TIM2,TIM_ICPolarity_Rising);//设置为上升沿捕获
    }
  }
}

/**
  * @brief  定时器3中断服务程序
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)   //TIM3中断
{
	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  ); //清除TIM3更新中断标志 
		
		timer_1s_flag = TIME_UP;
//		GPIO_ResetBits(GPIOA,GPIO_Pin_9);

	}
}

/**
  * @}
  */ 

void DMA1_Channel4_IRQHandler(void)
{
		if(DMA_GetITStatus(DMA1_FLAG_TC4) != RESET) 
		{
			DMA_ClearFlag(DMA1_FLAG_TC4);         // 清除标志
			DMA_Cmd(DMA1_Channel4, DISABLE);   // 关闭DMA通道			
		}
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
