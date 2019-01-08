#ifndef __REMOTE_H
#define __REMOTE_H
#include "stm32f10x.h"
#include "bitband.h"
#include "usart.h"
#include "tools.h"

#define RDATA_0 GPIOin(GPIOB,7)	 //红外数据输入脚
#define RDATA_1 GPIOin(GPIOB,9)	 //红外数据输入脚
#define RDATA_2 GPIOin(GPIOB,8)	 //红外数据输入脚
#define	RANGE 		3
#define VALUE_NUM   20

#define START 					0
#define STRAIGHT 				1
#define TURN_LEFT				2
#define TURN_RIGHT			3
//#define TURN_WHIRL			4

#define REMOTE_ID1 						0x68    		//left
#define REMOTE_ID2 						0x9A 				//right

#define REMOTE_ID_POWER_33 		0x33    		//电量低于33%
#define REMOTE_ID_POWER_66 		0x66 		   	//电量低于66%
#define REMOTE_ID_POWER_99 		0x99    		//电量低于99%
#define REMOTE_ID_POWER_100 	0xFF 		   	//电量满


#define REMOTE_ID_POWER_1 		0x33    		//电量1
#define REMOTE_ID_POWER_2 		0x66 		   	//电量2
#define REMOTE_ID_POWER_3 		0x99    		//电量3
#define REMOTE_ID_POWER_4 		0xbb    		//电量4
#define REMOTE_ID_POWER_MAX 	    0xFF 		   	//电量5

#define REMOTE_ID_POWER_ON 		0x5A    		//上电
#define REMOTE_ID_POWER_OFF 	0xA5 		   	//下电

#define REDUCE_RANGE_1		400
#define REDUCE_RANGE_2		200
#define REDUCE_RANGE_3		40
#define INIT_CTL_RANGE		400

#define NAVIGATION_1   	1
#define NAVIGATION_2	 	2
#define NAVIGATION_3	 	3

#define LEFT			1
#define RIGHT			0

#define W_HIGH		250

#define V_1				140//140
#define W_1				70//70

#define V_2				70//90
#define W_2				30//50

#define V_3				15//50
#define W_3				10//20



#define REMOTE_RCV_INTERFACE_NUM    3


extern uint8_t navigation_mode;
extern uint8_t RmtCnt[REMOTE_RCV_INTERFACE_NUM];	//按键按下的次数

void Remote_Init(void);    //红外传感器接收头引脚初始化
u8 Remote_Scan(void);
void remote_calculate(uint8_t scan_value);
#endif















