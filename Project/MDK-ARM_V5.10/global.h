#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "stm32f10x.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"

//control_flag
#define CONTROL_STOP				0		//�رճ��
#define CONTROL_START				1		//�������
#define CONTORL_GOTO_INIT		2		//�س�ʼ��

//power_state
#define POWER_ON						1		//���ڹ���
#define POWER_OFF						0		//ֹͣ����

//COM1_RX_CNT
#define COM1_RX_NONE				0		//δ�յ�����ͨѶ����
#define COM1_RX_GET_RIGHT		1		//���յ���ȷ����ͨѶ����
#define COM1_RX_GET_WRONG		2		//���յ����󴮿�ͨѶ����

//timer_1s_flag
#define TIME_UP							1		//ʱ�䵽
#define TIME_NOT_UP					0		//ʱ��δ��

//switch_flag
#define SWITCH_ON						0		//��翪�ؾ�����
#define SWITCH1_ON					1		//��翪��1����
#define SWITCH2_ON					2		//��翪��2����
#define SWITCH_OFF					3		//��翪�ؾ�δ����

//err_flag
#define	ERR_NONE								0			//�޴���
#define	ERR_VL6180X							1			//�������
#define	ERR_UART_RECEIVE				2			//���ڽ��ճ���
#define	ERR_NO_LIGHT_RECEIVE		4			//�Խ��Ϻ���ⷢ�����޷���
#define	ERR_SWITCH							8			//��翪�ض�ֵ����

typedef struct power_control
{
	uint8_t  control_flag;				//���Ƴ��״̬λ
	uint8_t  vol;									//�����˵���
	uint8_t  power_state;					//����״̬
	uint8_t  err_type;						//����λ
	uint16_t w;										//���ٶ�
	uint16_t v;										//���ٶ�
    uint8_t ir_left_num;
    uint8_t ir_right_num;
}POWER_CONTROL;


extern POWER_CONTROL power_ctl;
extern uint8_t COM1_RX_STATE;
extern uint8_t timer_1s_flag;

#endif
