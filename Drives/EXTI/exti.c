#include "exti.h"


/**
  * @brief  �ⲿ�жϳ�ʼ��
  * @param  None
  * @retval None
  * @note
  */
void exit0_init(void)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    //����һ:����GPIOAʱ�Ӻ͸�������IOʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);

    //�����������PA0Ϊ��������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //������������EXTI0�ⲿ�ж��ߵ��˿�PA0
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    //�����ģ�����EXTI0�ⲿ�ж���
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;//�ж���0
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж�ģʽ
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//�����ش���
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ���ⲿ�ж���
    EXTI_Init(&EXTI_InitStructure);//��������Ϣд��STM32�Ĵ���

    //�����壺�����ж����ȼ��飨ͬһ�������У����ȼ��������ͬ����������Ϊ��2)
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//���ȼ���2��2λ��ռ���ȼ���2Ϊ�����ȼ�

    //�������������ж�ͨ�����ȼ�
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//�ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;//��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;//�����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//�ж�ͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);

    //�����ߣ���stm32f10x_it.c�б�д�ⲿ�жϷ������void EXTI0_IRQHandler(void)
}

