#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "stm32f10x.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"

//control_flag
#define CONTROL_STOP           0//关闭充电
#define CONTROL_START          1//开启充电
#define CONTORL_GOTO_INIT      2//回初始点

//power_state
#define POWER_ON               1//正在供电
#define POWER_OFF              0//停止供电

//timer_1s_flag
#define TIME_UP                1//时间到
#define TIME_NOT_UP            0//时间未到

//switch_flag
#define SWITCH_ON              0//光电开关均触发
#define SWITCH1_ON             1//光电开关1触发
#define SWITCH2_ON             2//光电开关2触发
#define SWITCH_OFF             3//光电开关均未触发
#define SWITCH_NONE            4

//err_flag
#define ERR_NONE               0//无错误
#define ERR_VL6180X            1//激光出错
#define ERR_UART_RECEIVE       2//串口接收出错
#define ERR_NO_LIGHT_RECEIVE   4//对接上后红外发出后无返回
#define ERR_SWITCH             8//光电开关读值出错


#define REMOTE_RCV_INTERFACE_NUM    3

typedef struct power_control
{
    uint8_t  control_flag;//控制充电状态位
    uint8_t  vol;//机器人电量
    uint8_t  power_state;//供电状态
    uint8_t  err_type;//故障位
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
