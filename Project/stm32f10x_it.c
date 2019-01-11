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
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}


/**
  * @brief  USART1串口中断
  * @param  None
  * @retval None
  */
void USART1_IRQHandler()
{

}


/**
  * @brief  USART2串口中断
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{

}


/**
  * @brief  USART3串口中断
  * @param  None
  * @retval None
  */
void USART3_IRQHandler(void)
{

}


/**
  * @brief  This function handles TIM3 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) //发生了更新中断
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);//清除更新中断标志
        if(capture_status & 0x4000)//捕获到上升沿
        {
            if((capture_status & 0x0FFF) >= 0x0FFF)//溢出，捕获的时间太长了
            {
                capture_status |= 0x2000;//设置溢出标志
            }
            else
            {
                capture_status++;//更新事件次数累加
            }
        }
    }

    if(TIM_GetITStatus(TIM2, TIM_IT_CC1) == SET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);//清除捕获比较中断标志

        if(capture_status & 0x8000)//上次捕获值未被使用，丢弃掉上次捕获的值
        {
            capture_status = 0;
            capture_rise_val = 0;
            capture_fall_val = 0;
        }

        if((capture_status & 0x4000) == 0)//捕捉到上升沿
        {
            capture_rise_val = TIM_GetCapture1(TIM2);//记录捕获值
            capture_status |= 0X4000;//标志捕捉到上升沿
            TIM_OC1PolarityConfig(TIM2, TIM_ICPolarity_Falling);//设置下降沿捕获
        }
        else if(capture_status & 0x4000)//捕获到下降沿
        {
            capture_fall_val = TIM_GetCapture1(TIM2); //记录捕获值
            capture_status &= ~0X4000;//清除上升沿标志
            capture_status |= 0X8000;//设置捕获完成标志
            TIM_OC1PolarityConfig(TIM2, TIM_ICPolarity_Rising);//设置为上升沿捕获
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
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)//检查TIM3更新中断发生与否
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );//清除TIM3更新中断标志
        timer_1s_flag = TIME_UP;
    }
}


void DMA1_Channel4_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_FLAG_TC4) != RESET)
    {
        DMA_ClearFlag(DMA1_FLAG_TC4);         // 清除标志
        DMA_Cmd(DMA1_Channel4, DISABLE);   // 关闭DMA通道
    }
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
