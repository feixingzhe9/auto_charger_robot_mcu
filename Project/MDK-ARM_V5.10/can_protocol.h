/**
******************************************************************************
* @file    can.c 
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
#define SW_VERSION                      "NoahC001M09A102"
#define PROTOCOL_VERSION                "20170505R0101"



#define CAN_USED    CAN1
#define CAN_ID          (0x434D0000)      //CM

#define CAN_AUTO_CHARGER_ROBOT_MAC_ID       (0xD1)
#define CAN_LOCAL_ID    (0x06)            //local ID


#define CAN_CMD_READ_VERSION            0x01
#define CAN_CMD_LEDS_CONTROL            0x20
#define CAN_CMD_S_SYS_V_BAT             0x21
#define CAN_CMD_


#define SOURCE_ID_PREPARE_UPDATE        0x10
#define SOURCE_ID_TRANSMIT_UPDATE       0x11
#define SOURCE_ID_CHECK_TRANSMIT        0x12


#define CAN_FIFO_SIZE                   250
typedef union
{
	 struct{
		uint32_t start_address; // the address of the bin saved on flash.
		uint32_t length; // file real length
		uint8_t version[8];
		uint8_t type; // B:bootloader, P:boot_table, A:application, 
		uint8_t upgrade_type; //u:upgrade, 
		uint8_t reserved[6];
	}boot_table_t;
 char data[24];
 }BOOTLOADER_UNION;


typedef struct
{
	uint8_t FuncID;
	uint16_t len;
	uint8_t *pdata;
}CAN_TXDATA_STRUCT;

typedef struct {
	uint32_t ID;
	uint32_t TimeStamp;
	uint8_t TimeFlag;
	uint8_t SendType;
	uint8_t RemoteFlag;
	uint8_t ExternFlag;
	uint8_t DataLen;
	uint8_t Data[8];
	uint8_t Reserved[3];

} CM_CAN_T;

typedef struct {
	uint32_t AccCode;
	uint32_t AccMask;
	uint32_t Filter;
	uint32_t Bundrate;
	uint8_t Mode;
	
} CM_CAN_CONFIG_T;


typedef union
{
	struct
	{
		uint32_t SourceID  : 8;
		uint32_t FUNC_ID   : 4;
		uint32_t ACK       : 1;
		uint32_t DestMACID : 8;
		uint32_t SrcMACID  : 8;
		uint32_t res       : 3;
	}CanID_Struct;
	uint32_t  CANx_ID;
}CAN_ID_UNION;


typedef union
{
	struct
	{
        uint8_t SegNum  : 6;
        uint8_t SegPolo : 2;
		uint8_t Data[7];
	}CanData_Struct;
	uint8_t CanData[8];
}CAN_DATA_UNION;

#define CAN_ONE_FRAME_DATA_LENTH    7
#define CAN_SEG_NUM_MAX             64
#define CAN_LONG_FRAME_LENTH_MAX    (CAN_ONE_FRAME_DATA_LENTH*CAN_SEG_NUM_MAX)
typedef struct
{
    uint32_t can_id;
    uint32_t start_time; 
    uint16_t used_len;
    uint8_t rcv_buf[CAN_LONG_FRAME_LENTH_MAX];   
}CAN_RCV_BUFFER_T;

typedef uint8_t (*GetOneFreeBufFn)(void);
typedef uint8_t (*GetTheBufByIdFn)(uint32_t);
typedef void (*FreeBufFn)(uint8_t);

#define CAN_LONG_BUF_NUM    2   // value can not be 0 ! !
typedef struct
{
    CAN_RCV_BUFFER_T can_rcv_buf[CAN_LONG_BUF_NUM];
    GetOneFreeBufFn GetOneFreeBuf; 
    GetTheBufByIdFn GetTheBufById;
    FreeBufFn FreeBuf;
}CAN_LONG_BUF_T;


#if 0
extern uint8_t CanUpdataBuff[64];
extern uint8_t CanRxdataBuff[64];
#endif

void RxMsgHandle(uint32_t ID,uint8_t* pdata);

void CM_CAN_Init(void);
void RxMsgHandle(uint32_t ID,uint8_t* pdata);

void CAN_SetMsg(void);





void can_protocol_period( void );

void CanLongBufInit(void);

/*******************  Bit definition for ExtId bytes  ********************/
#define EXTID_FRAME_TYPE_BITS						((uint32_t)0x10000000)
#define EXTID_SRC_ID_BITS						((uint32_t)0x0FC00000)
#define EXTID_DST_ID_BITS						((uint32_t)0x003F0000)
/* normal */
#define EXTID_PROPERTY_BITS						((uint32_t)0x0000C000)
#define EXTID_FUNC_ID_BITS						((uint32_t)0x00003F80)
#define EXTID_SEGMENT_NUM_D_BITS					((uint32_t)0x00000078)
#define EXTID_ACK_D_BITS						((uint32_t)0x00000004)
#define EXTID_END_FLAG_D_BITS						((uint32_t)0x00000002)
#define EXTID_RESEND_D_BITS						((uint32_t)0x00000001)
/* update */
#define EXTID_ACK_U_BITS						((uint32_t)0x00008000)
#define EXTID_RESEND_U_BITS						((uint32_t)0x00004000)
#define EXTID_SEGMENT_NUM_U_BITS					((uint32_t)0x00003FFF)

#define EXTID_SRC_ID_BITS_NUM						(22U)
#define EXTID_DST_ID_BITS_NUM						(16U)
#define EXTID_PROPERTY_BITS_NUM						(14U)
#define EXTID_FUNC_ID_BITS_NUM						(4U)

#define EXTID_FRAME_TYPE_DATA						(0)
#define EXTID_FRAME_TYPE_UPDATE						(1)
#endif
