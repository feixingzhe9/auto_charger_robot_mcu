#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "stm32f10x.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"

//control_flag
#define CONTROL_STOP           0//�رճ��
#define CONTROL_START          1//�������
#define CONTORL_GOTO_INIT      2//�س�ʼ��

//power_state
#define POWER_ON               1//���ڹ���
#define POWER_OFF              0//ֹͣ����

//timer_1s_flag
#define TIME_UP                1//ʱ�䵽
#define TIME_NOT_UP            0//ʱ��δ��

//switch_flag
#define SWITCH_ON              0//��翪�ؾ�����
#define SWITCH1_ON             1//��翪��1����
#define SWITCH2_ON             2//��翪��2����
#define SWITCH_OFF             3//��翪�ؾ�δ����
#define SWITCH_NONE            4

//err_flag
#define ERR_NONE               0//�޴���
#define ERR_VL6180X            1//�������
#define ERR_UART_RECEIVE       2//���ڽ��ճ���
#define ERR_NO_LIGHT_RECEIVE   4//�Խ��Ϻ���ⷢ�����޷���
#define ERR_SWITCH             8//��翪�ض�ֵ����


#define REMOTE_RCV_INTERFACE_NUM    3

typedef struct power_control
{
    uint8_t  control_flag;//���Ƴ��״̬λ
    uint8_t  vol;//�����˵���
    uint8_t  power_state;//����״̬
    uint8_t  err_type;//����λ
    uint8_t ir_left_num;
    uint8_t ir_right_num;
    uint8_t switch_status;
}POWER_CONTROL;


typedef struct
{
    uint32_t ir_left_num;
    uint32_t ir_right_num;
}ir_rcv_channel_info_t;

typedef struct
{
    ir_rcv_channel_info_t ir_channel[REMOTE_RCV_INTERFACE_NUM];
}ir_rcv_info_t;

typedef struct
{
    uint8_t left_intensity;
    uint8_t right_intensity;
}ir_channel_signal_intensity_t;


typedef struct
{
    ir_channel_signal_intensity_t intensity[REMOTE_RCV_INTERFACE_NUM];
}ir_signal_intensity_t;

extern ir_signal_intensity_t ir_signal_intensity;
extern ir_rcv_info_t ir_info;

extern POWER_CONTROL power_ctl;
extern uint8_t timer_1s_flag;

#endif
