#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "stm32f10x.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"

//control_flag
#define CONTROL_STOP				0		//关闭充电
#define CONTROL_START				1		//开启充电
#define CONTORL_GOTO_INIT		2		//回初始点

//power_state
#define POWER_ON						1		//正在供电
#define POWER_OFF						0		//停止供电

//COM1_RX_CNT
#define COM1_RX_NONE				0		//未收到串口通讯数据
#define COM1_RX_GET_RIGHT		1		//已收到正确串口通讯数据
#define COM1_RX_GET_WRONG		2		//已收到错误串口通讯数据

//timer_1s_flag
#define TIME_UP							1		//时间到
#define TIME_NOT_UP					0		//时间未到

//switch_flag
#define SWITCH_ON						0		//光电开关均触发
#define SWITCH1_ON					1		//光电开关1触发
#define SWITCH2_ON					2		//光电开关2触发
#define SWITCH_OFF					3		//光电开关均未触发

//err_flag
#define	ERR_NONE								0			//无错误
#define	ERR_VL6180X							1			//激光出错
#define	ERR_UART_RECEIVE				2			//串口接收出错
#define	ERR_NO_LIGHT_RECEIVE		4			//对接上后红外发出后无返回
#define	ERR_SWITCH							8			//光电开关读值出错

typedef struct power_control
{
	uint8_t  control_flag;				//控制充电状态位
	uint8_t  vol;									//机器人电量
	uint8_t  power_state;					//供电状态
	uint8_t  err_type;						//故障位
	uint16_t w;										//角速度
	uint16_t v;										//线速度
    uint8_t ir_left_num;
    uint8_t ir_right_num;
}POWER_CONTROL;


extern POWER_CONTROL power_ctl;
extern uint8_t COM1_RX_STATE;
extern uint8_t timer_1s_flag;

#endif
