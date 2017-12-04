
#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"
#include "i2c.h"
#include "VL6180x.h"
#include "remote.h"
#include "switch.h"
#include "global.h"
#include "tools.h"
#include "can_protocol.h"
#include "can.h"
#include "led.h"
int main(void)
{
	/* USER CODE BEGIN 1 */
	int vl6180x_status;
	uint8_t cnt=0;
	uint8_t light_ID = 0;
    uint32_t tick = 0;
	
	//Delay_Init();			//��ʱ��ʼ��
	SysTickInit();
	COM_Init(COM1, 115200);//����1��ʼ��
	COM1_DMA_init(115200);
	
	Timer1_Init(9999, 7199);				//�����жϷ�ʱ������Ƶ�� = 72000/(7199+1) = 10kHz��������(9999+1)Ϊ1s//ok
	
	I2C_Init();				//��ʼ��I2C�ӿ�
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ� 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�		
	
	TIM4_CH1_PWM_Init(1895,0);	//72000/(1895+1) = 37.99K			//���ⷢ���ʼ��		
	
	Remote_Init();			//������ճ�ʼ��	
	
	VL6180x_init();
	
	swtich_init();
    LED_Init();
    CanInit();
    
	
	while(1)
	{
		delay_ms(10);
		cnt = (cnt+1)%5;
        tick++;

		light_ID = Remote_Scan();													//��ȡ�����ֵ
		
		vl6180x_status = calculate_length(cnt);						//�������ݲ���
		
		calculate_speed(vl6180x_status,light_ID);					//��������˶��ٶ�
		
		update_status(light_ID);													//��������״̬

        can_protocol_period();  
        IndicatorLed();
	}
}

