
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
	
	//Delay_Init();			//延时初始化
	SysTickInit();
	COM_Init(COM1, 115200);//串口1初始化
	COM1_DMA_init(115200);
	
	Timer1_Init(9999, 7199);				//用于中断分时，计数频率 = 72000/(7199+1) = 10kHz，计数到(9999+1)为1s//ok
	
	I2C_Init();				//初始化I2C接口
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级		
	
	TIM4_CH1_PWM_Init(1895,0);	//72000/(1895+1) = 37.99K			//红外发射初始化		
	
	Remote_Init();			//红外接收初始化	
	
	VL6180x_init();
	
	swtich_init();
    LED_Init();
    CanInit();
    
	
	while(1)
	{
		delay_ms(10);
		cnt = (cnt+1)%5;
        tick++;

		light_ID = Remote_Scan();													//获取红外读值
		
		vl6180x_status = calculate_length(cnt);						//激光数据测量
		
		calculate_speed(vl6180x_status,light_ID);					//计算底盘运动速度
		
		update_status(light_ID);													//更新所有状态

        can_protocol_period();  
        IndicatorLed();
	}
}

