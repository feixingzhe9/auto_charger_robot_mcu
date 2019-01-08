#include "tools.h"

void set_straight(void)
{
    return ;
//	power_ctl.v = 0xff00|(256-((navigation_mode == NAVIGATION_1)?V_1:(navigation_mode == NAVIGATION_2)?V_2:V_3));
	power_ctl.v = (navigation_mode == NAVIGATION_1)?V_1:(navigation_mode == NAVIGATION_2)?V_2:V_3;
	power_ctl.w = 0;
}

void set_left(void)
{
    return ;
	power_ctl.w = 0xff00|(256 - ((navigation_mode == NAVIGATION_1)?W_1:(navigation_mode == NAVIGATION_2)?W_2:W_3));
	power_ctl.v = 0;
}

void set_right(void)
{
    return ;
	power_ctl.w = (navigation_mode == NAVIGATION_1)?W_1:(navigation_mode == NAVIGATION_2)?W_2:W_3;	
	power_ctl.v = 0;
}

void set_forward(void)
{
    return ;
	power_ctl.v = V_3;
	power_ctl.w = 0;
}

void set_toward(void)
{
    return ;
	power_ctl.v = 0xff00|(256 - V_3);
	power_ctl.w = 0;
}

void set_stop(void)
{
    return ;
	power_ctl.v = 0;
	power_ctl.w = 0;
}

void set_high_w(uint8_t dir)
{
    return ;
	if(dir == RIGHT)
	{
		power_ctl.w = W_HIGH;	
		power_ctl.v = 0;		
	}
	if(dir == LEFT)
	{
		power_ctl.w = 0xff00|(256 - W_HIGH);
		power_ctl.v = 0;		
	}
}

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
//	printf("range_value = %d\r\n",range_value);
	if(vl6180x_status)
	{
		navigation_mode = NAVIGATION_3;
	}
	
	else
	{
		if(range_value > REDUCE_RANGE_1)
		{
			navigation_mode = NAVIGATION_1;//1
		}
		else if((range_value <= REDUCE_RANGE_1)&&(range_value > REDUCE_RANGE_2))
		{
			navigation_mode = NAVIGATION_2;	//2
		}
		else
		{
			navigation_mode = NAVIGATION_3;//3
		}
	}
	remote_calculate(id);
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
			set_forward();
		}
		else
		{
			if(power_ctl.control_flag != CONTORL_GOTO_INIT)
			{
				power_ctl.v = 0;
				power_ctl.w = 0;
			}
			power_ctl.power_state = POWER_ON;
	//		if((id == REMOTE_ID_POWER_ON)||(id == REMOTE_ID_POWER_OFF))
	//		{
	//			light_receive_flag = 1;
	//			if(id == REMOTE_ID_POWER_ON)
	//			{
	//				power_ctl.power_state = POWER_ON;	
	//			}
	//			else
	//			{
	//				power_ctl.power_state = POWER_OFF;	
	//			}
	//		}
			if(timer_1s_flag == TIME_UP)
			{
				timer_1s_flag = TIME_NOT_UP;
				
				TIM_ITConfig( TIM4,TIM_IT_Update | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4,DISABLE);
//				USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
//				DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,DISABLE);
				TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE );
				TIM4_CH1_PWM_Init(1895,0);	//72000/(1895+1) = 37.99K			//红外发射初始化		
	//			printf("power_ctl.vol = %d\r\n",power_ctl.vol);
#if 0
				if(power_ctl.vol <= 33)//按实际电量发送红外灯
				{
					SendData2(REMOTE_ID_POWER_33,REMOTE_ID_POWER_33);
				}
				else if(power_ctl.vol <= 66)//按实际电量发送红外灯
				{
					SendData2(REMOTE_ID_POWER_66,REMOTE_ID_POWER_66);
				}
				else if(power_ctl.vol <= 99)//按实际电量发送红外灯
				{
					SendData2(REMOTE_ID_POWER_99,REMOTE_ID_POWER_99);
				}
				else//按实际电量发送红外灯
				{
					SendData2(REMOTE_ID_POWER_100,REMOTE_ID_POWER_100);
				}
#else
                
//                power_ctl.vol = 50;////test code

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
                
                
#endif
				
				TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE);
//				USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//				DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);
				TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );
				Remote_Init();			//红外接收初始化	
				
				time_out = (time_out+1)%5;
	//			if(time_out ==4)
	//			{
	//				if(light_receive_flag == 0)//如果没收到充电桩的反馈
	//				{
	//					power_ctl.power_state = POWER_OFF;
	//					power_ctl.err_type |= ERR_NO_LIGHT_RECEIVE;
	//				}
	//				else
	//				{
	//					light_receive_flag = 0;
	//					power_ctl.err_type &= (0xff-ERR_NO_LIGHT_RECEIVE);
	//				}
	//			}
			}
		}
	}
	else
	{
		power_ctl.power_state = POWER_OFF;
	}
	
}

void com_receive(void)
{
    return ;
	if(COM1_RX_STATE == COM1_RX_NONE)
	{
		return;
	}
	else if(COM1_RX_STATE == COM1_RX_GET_RIGHT)
	{
		power_ctl.err_type &= (0xff-ERR_UART_RECEIVE);
		reply_usart1();
	}
	else
	{
		power_ctl.err_type |= ERR_UART_RECEIVE;
		reply_usart1();		
	}
	
	COM1_RX_STATE = COM1_RX_NONE;
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//关闭串口1接收中断

}
