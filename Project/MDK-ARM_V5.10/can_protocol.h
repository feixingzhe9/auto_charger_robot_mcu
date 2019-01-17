/**
******************************************************************************
* @file    can_protocol.h
* @author  Kaka
* @version V1.0.0
* @date    15-May-2017
* @brief   This file provides CAN driver.
******************************************************************************
*/
#ifndef __CAN_PROTOCOL_H
#define __CAN_PROTOCOL_H

#include "stm32f10x.h"

#define HW_VERSION                      "11"
#define SW_VERSION                      "104"
#define PROTOCOL_VERSION                "20170505R0101"

#define CAN_USED                        CAN1

#define CAN_CHARGE_ROBOT_MAC_ID         (0xD1)
#define CAN_LOCAL_ID                    (0x06)

#define CAN_CMD_READ_VERSION            0x01
#define CAN_CMD_S_SYS_V_BAT             0x21

#define SOURCE_ID_PREPARE_UPDATE        0x10
#define SOURCE_ID_TRANSMIT_UPDATE       0x11
#define SOURCE_ID_CHECK_TRANSMIT        0x12

#define CAN_FIFO_SIZE                   250

#define CAN_ONE_FRAME_DATA_LENTH        7
#define CAN_SEG_NUM_MAX                 64
#define CAN_LONG_FRAME_LENTH_MAX       (CAN_ONE_FRAME_DATA_LENTH*CAN_SEG_NUM_MAX)

#define CAN_LONG_BUF_NUM                2

/** function id define  **/
#define CAN_FUN_ID_RESERVE_0          0x00
#define CAN_FUN_ID_WRITE              0x01
#define CAN_FUN_ID_READ               0x02
#define CAN_FUN_ID_TRIGGER            0x03
#define CAN_FUN_ID_RESERVE_4          0x04
#define CAN_FUN_ID_RESERVE_5          0x05
#define CAN_FUN_ID_RESET              0x06

/** source id define  **/
#define CAN_SOURCE_ID_READ_VERSION    0x01
#define CAN_SOURCE_ID_GET_SYS_STATUS  0x80


#define CAN_SOURCE_ID_MULTI_IR_INFO     0x90
#define CAN_SOURCE_ID_SYS_INFO          0x91

#define ONLYONCE                      0x00
#define BEGIN                         0x01
#define TRANSING                      0x02
#define END                           0x03

#define CAN_LONG_BUF_FULL             0xff
#define CAN_BUF_NO_THIS_ID            0xfe

#define CAN_LONG_FRAME_TIME_OUT       20000/SYSTICK_PERIOD
#define CAN_COMM_TIME_OUT             5000


typedef uint8_t (*GetOneFreeBufFn)(void);
typedef uint8_t (*GetTheBufByIdFn)(uint32_t);
typedef void (*FreeBufFn)(uint8_t);

typedef struct{
    uint32_t AccCode;
    uint32_t AccMask;
    uint32_t Filter;
    uint32_t Bundrate;
    uint8_t  Mode;
} CM_CAN_CONFIG_t;

typedef union
{
    struct{
        uint32_t SourceID  : 8;
        uint32_t FUNC_ID   : 4;
        uint32_t ACK       : 1;
        uint32_t DestMACID : 8;
        uint32_t SrcMACID  : 8;
        uint32_t res       : 3;
    }CanID_Struct;
uint32_t  CANx_ID;
}CAN_ID_u;

typedef union
{
    struct{
        uint8_t SegNum  : 6;
        uint8_t SegPolo : 2;
        uint8_t Data[7];
    }CanData_Struct;
    uint8_t CanData[8];
}CAN_DATA_u;

typedef struct
{
    uint32_t can_id;
    uint32_t start_time;
    uint16_t used_len;
    uint8_t rcv_buf[CAN_LONG_FRAME_LENTH_MAX];
}CAN_RECV_BUFF_t;

typedef struct
{
    CAN_RECV_BUFF_t can_rcv_buf[CAN_LONG_BUF_NUM];
    GetOneFreeBufFn GetOneFreeBuf;
    GetTheBufByIdFn GetTheBufById;
    FreeBufFn FreeBuf;
}CAN_LONG_BUF_t;


void can_long_buf_init(void);
void can_protocol_period( void );

#endif

