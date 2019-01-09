#include "tools.h"

uint8_t calculate_length(uint8_t cnt)
{
	static uint8_t status = 0;				
	
	if(cnt == 0)
	{
		status = VL6180x_start();
//		printf("vl6180x_status = %d,",status);
	}	
	if(cnt == 4)
	{
		status = VL6180x_end();
//		printf("vl6180x_status = %d,",status);
	}
	
	if(status == 0)
	{
		power_ctl.err_type &= (0xff-ERR_VL6180X);
	}
	else
	{
		power_ctl.err_type |= ERR_VL6180X;
	}
	return status;
}

void calculate_speed(uint8_t vl6180x_status,uint8_t id)
{
	remote_calculate(id);
}

void cal_multi_ir_value(void)
{

}

void update_status(uint8_t id)
{
	static uint8_t switch_status = SWITCH_OFF;
	static uint8_t prev_status = SWITCH_OFF;
	static uint8_t time_out = 0;
	static uint8_t time_wait = 0;

	prev_status = switch_status;
	switch_status = switch_scan();
//	printf("switch_status = %d\r\n",switch_status);
	if((switch_status != SWITCH_ON)&&(switch_status != SWITCH_OFF))
	{
		power_ctl.err_type |= ERR_SWITCH;
	}
	else
	{
		power_ctl.err_type &= (0xff-ERR_SWITCH);
		power_ctl.err_type &= (0xff-ERR_NO_LIGHT_RECEIVE);
	}
	power_ctl.switch_status = switch_status;
	if(switch_status == SWITCH_ON)
	{
		if(prev_status != SWITCH_ON)
		{
			time_wait = 8;
		}
		
		if(time_wait !=0)
		{
			time_wait --;

		}
		else
		{
			if(power_ctl.control_flag != CONTORL_GOTO_INIT)
			{

			}
//			power_ctl.power_state = POWER_ON;

			if(timer_1s_flag == TIME_UP)
			{
				timer_1s_flag = TIME_NOT_UP;
				
				TIM_ITConfig( TIM4,TIM_IT_Update | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4,DISABLE);
//				USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
//				DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,DISABLE);
				TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE );
				TIM4_CH1_PWM_Init(1895,0);	//72000/(1895+1) = 37.99K			//红外发射初始化		

                if(power_ctl.vol < 25)//
				{
					SendData2(REMOTE_ID_POWER_1,REMOTE_ID_POWER_1);
				}
                else if(power_ctl.vol < 50)//
                {
                    SendData2(REMOTE_ID_POWER_2,REMOTE_ID_POWER_2);
                }
                else if(power_ctl.vol < 75)//
                {
                    SendData2(REMOTE_ID_POWER_3,REMOTE_ID_POWER_3);
                }
                else if(power_ctl.vol < 100)//
                {
                    SendData2(REMOTE_ID_POWER_4,REMOTE_ID_POWER_4);
                }
                else if(power_ctl.vol == 100)//
                {
                    SendData2(REMOTE_ID_POWER_MAX,REMOTE_ID_POWER_MAX);
                }

				TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE);
//				USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//				DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);
				TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );
				Remote_Init();			//红外接收初始化	
				
				time_out = (time_out+1)%5;
			}
		}
	}
	else
	{
//		power_ctl.power_state = POWER_OFF;
	}

}

