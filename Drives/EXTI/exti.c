#include "exti.h"


/**
  * @brief  外部中断初始化
  * @param  None
  * @retval None
  * @note
  */
void exit0_init(void)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    //步骤一:开启GPIOA时钟和辅助功能IO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);

    //步骤二：配置PA0为浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //步骤三：连接EXTI0外部中断线到端口PA0
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    //步骤四：配置EXTI0外部中断线
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;//中断线0
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//上升沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能外部中断线
    EXTI_Init(&EXTI_InitStructure);//将配置信息写入STM32寄存器

    //步骤五：配置中断优先级组（同一个工程中，优先级组必须相同，这里设置为组2)
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//优先级组2：2位抢占优先级，2为子优先级

    //步骤六：配置中断通道优先级
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;//抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;//子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//中断通道使能
    NVIC_Init(&NVIC_InitStructure);

    //步骤七：在stm32f10x_it.c中编写外部中断服务程序void EXTI0_IRQHandler(void)
}

