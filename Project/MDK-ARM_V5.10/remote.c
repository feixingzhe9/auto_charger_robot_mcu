#include "remote.h"
#include "delay.h"
#include "usart.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"


//红外遥控初始化
//设置IO以及定时器4的输入捕获
void Remote_Init(void)    			  
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;  
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); //使能PORTB时钟 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);	//TIM4 时钟使能 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				 //PB9 输入 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		//上拉输入 
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOB,GPIO_Pin_7);	//初始化GPIOB.9
	
						  
 	TIM_TimeBaseStructure.TIM_Period = 10000; //设定计数器自动重装值 最大10ms溢出  
	TIM_TimeBaseStructure.TIM_Prescaler =(72-1); 	//预分频器,1M的计数频率,1us加1.	   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;  // 选择输入端 IC4映射到TI4上
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
	TIM_ICInitStructure.TIM_ICFilter = 0x03;//IC4F=0011 配置输入滤波器 8个定时器时钟周期滤波
	TIM_ICInit(TIM4, &TIM_ICInitStructure);//初始化定时器输入捕获通道

	TIM_Cmd(TIM4,ENABLE ); 	//使能定时器4
 
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器	

	TIM_ITConfig( TIM4,TIM_IT_Update|TIM_IT_CC2,ENABLE);//允许更新中断 ,允许CC4IE捕获中断								 
}

//遥控器接收状态
//[7]:收到了引导码标志
//[6]:得到了一个按键的所有信息
//[5]:保留	
//[4]:标记上升沿是否已经被捕获								   
//[3:0]:溢出计时器
u8 	RmtSta=0;	  	  
u16 Dval;		//下降沿时计数器的值
u32 RmtRec=0;	//红外接收到的数据	   		    
u8  RmtCnt=0;	//按键按下的次数	  
//定时器2中断服务程序	 
void TIM4_IRQHandler(void)
{ 		    	 
    if(TIM_GetITStatus(TIM4,TIM_IT_Update)!=RESET)
	{
		if(RmtSta&0x80)//上次有数据被接收到了
		{	
			RmtSta&=~0X10;						//取消上升沿已经被捕获标记
			if((RmtSta&0X0F)==0X00)RmtSta|=1<<6;//标记已经完成一次按键的键值信息采集
			if((RmtSta&0X0F)<14)RmtSta++;
			else
			{
				RmtSta&=~(1<<7);//清空引导标识
				RmtSta&=0XF0;	//清空计数器	
			}						 	   	
		}							    
	}
 	if(TIM_GetITStatus(TIM4,TIM_IT_CC2)!=RESET)
	{	  
		if(RDATA)//上升沿捕获
		{

			TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获				
	    	TIM_SetCounter(TIM4,0);	   	//清空定时器值
			RmtSta|=0X10;					//标记上升沿已经被捕获
		}else //下降沿捕获
		{			
  			 Dval=TIM_GetCapture2(TIM4);//读取CCR1也可以清CC1IF标志位
			 TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC4P=0	设置为上升沿捕获
 			
			if(RmtSta&0X10)					//完成一次高电平捕获 
			{
 				if(RmtSta&0X80)//接收到了引导码
				{
					
					if(Dval>300&&Dval<800)			//560为标准值,560us
					{
						RmtRec|=0;	//接收到0	  
						RmtRec<<=1;	//左移一位. 
					}else if(Dval>1400&&Dval<1800)	//1680为标准值,1680us
					{
						RmtRec|=1;	//接收到1
						RmtRec<<=1;	//左移一位.
					}else if(Dval>2200&&Dval<2600)	//得到按键键值增加的信息 2500为标准值2.5ms
					{
						RmtCnt++; 		//按键次数增加1次
						RmtSta&=0XF0;	//清空计时器		
					}
 				}else if(Dval>4200&&Dval<4700)		//4500为标准值4.5ms
				{
					RmtSta|=1<<7;	//标记成功接收到了引导码
					RmtCnt=0;		//清除按键次数计数器
				}						 
			}
			RmtSta&=~(1<<4);
		}				 		     	    					   
	}
 TIM_ClearFlag(TIM4,TIM_IT_Update|TIM_IT_CC2);	    
}

//处理红外键盘
//返回值:
//	 0,没有任何按键按下
//其他,按下的按键键值.
u8 Remote_Scan(void)
{        
	u8 value=0;
	u8 sta=0;       
  u8 t1=0;
	u8 t2=0;
  u8 c1=0;
	u8 c2=0;  
	if(RmtSta&(1<<6))//得到一个按键的所有信息了
	{ 
	    t1=RmtRec>>24;			//得到地址码
	    t2=(RmtRec>>16)&0xff;	//得到地址反码 
		  c1=(RmtRec>>8)&0xff;	//得到命令码
		  c2=(RmtRec>>0)&0xff;	//得到命令反码
 	    if((t1==(u8)~t2)&&((c1==(u8)~c2)||(c1==(u8)~(c2+1)))&&((t1==REMOTE_ID1)||(t1==REMOTE_ID2)||(t1==REMOTE_ID_POWER_ON)||(t1==REMOTE_ID_POWER_OFF)))//检验遥控识别码(ID)及地址 
	    { 
//				printf("t1=%x,t2=%x,c1=%x,c2=%x\r\n",t1,t2,c1,c2);
//	        t1=RmtRec>>8;
//	        t2=RmtRec; 	
//	        if(t1==(u8)~t2)sta=t1;//键值正确	 
				value = t1;
			}
			else
			{
				value = 0;
			}
		if((sta==0)||((RmtSta&0X80)==0))//按键数据错误/遥控已经没有按下了
		{
		 	RmtSta&=~(1<<6);//清除接收到有效按键标识
			RmtCnt=0;		//清除按键次数计数器
		}
	}  
    return value;
}

u8 left_value[VALUE_NUM] = {0};
u8 right_value[VALUE_NUM] = {0};

static u8 prev_dir = START;
uint8_t navigation_mode = NAVIGATION_1;

void remote_calculate(uint8_t scan_value)
{
	int8_t i;
	static u8 time_out = 0;
	u8 left = 0;
	u8 right = 0;
	
	if(power_ctl.control_flag == CONTROL_STOP)
	{
		set_stop();
		return;
	}
	else if(power_ctl.control_flag == CONTORL_GOTO_INIT)
	{
		if(range_value > INIT_CTL_RANGE)
		{
			set_stop();
			return;
		}
		else
		{
			set_toward();
			return;
		}
	}
	else
	{
		time_out++;				//NEC协议一次通讯110+9+4.5 = 123.5ms
	//	printf("%x\r\n",scan_value);
		if((scan_value != REMOTE_ID1)&&(scan_value != REMOTE_ID2)&&(time_out <= 10))
		{
			return ;
		}
	//	printf("scan_value=%x\r\n",scan_value);
		for(i=VALUE_NUM-1;i>0;i--)
		{
			left_value[i] = left_value[i-1];
			right_value[i] = right_value[i-1];
		}
		
		if(scan_value == REMOTE_ID1)
		{
			time_out = 0;
			left_value[0] = 1;
			right_value[0] = 0;
		}
		else if(scan_value == REMOTE_ID2)
		{
			time_out = 0;
			left_value[0] = 0;
			right_value[0] = 1;
		}
		else
		{
			if(time_out > 10)
			{
				time_out = 0;
				left_value[0] = 0;
				right_value[0] = 0;			
			}
		}
		
		for(i=0;i<VALUE_NUM;i++)
		{
			left += left_value[i];
			right += right_value[i];
	//		printf("%d,%d,",left_value[i],right_value[i]);
		}
		if((left>=RANGE)&&(right>=RANGE))
		{
	//		printf("直走\r\n");
			set_straight();//发送直行
	//		send_straight();//发送直行
			prev_dir = STRAIGHT;
		}
		else if(range_value <= REDUCE_RANGE_3)
		{
			set_toward();//发送向前命令，避免擦伤壳体
		}
		else if((left>=RANGE)&&(right<RANGE))
		{
	//		printf("右转\r\n");	
			if(prev_dir == TURN_LEFT)
			{
				set_straight();//发送直行
	//			send_straight();//发送直行
			}
			else
			{
				set_right();//发送右转
	//			send_right();//发送右转
			}
			prev_dir = TURN_RIGHT;
		}	
		else if((left<RANGE)&&(right>=RANGE))
		{
	//		printf("左转\r\n");
			if(prev_dir == TURN_RIGHT)
			{
				set_straight();//发送直行
	//			send_straight();//发送直行
			}
			else
			{
				set_left();//发送左转
	//			send_left();//发送左转
			}
			prev_dir = TURN_LEFT;
		}
		else
		{
	//		printf("原地顺时针转\r\n");
			if(prev_dir == TURN_LEFT)
			{
				if(range_value <= 100)
				{
					set_left();//发送左转
				}
				else
				{
					set_high_w(LEFT);//发送左转
				}
	//			send_left();//发送左转	
			}
			else
			{
				if(range_value <= 100)
				{
					set_right();//发送右转
				}
				else
				{
					set_high_w(RIGHT);//发送右转	
				}	
	//			send_right();//发送右转		
			}
		}
//		printf("left= %d,right= %d,light = %d \r\n",left,right,range_value);
	}
}

