/**
******************************************************************************
* @file    can.c 
* @author  Kaka
* @version V1.0.0
* @date    15-May-2017
* @brief   This file provides CAN protocol functions.
******************************************************************************

******************************************************************************
*/ 
#include "can_protocol.h"
#include "delay.h"
//#include "platform.h"
#include <string.h>
#include <stdio.h>


CanRxMsg RxMessage;
CanTxMsg TxMessage;
uint8_t CanTxdataBuff[CAN_LONG_FRAME_LENTH_MAX] = {0};


//////  function id define  //////
#define CAN_FUN_ID_RESERVE_0    0x00

#define CAN_FUN_ID_WRITE        0x01
#define CAN_FUN_ID_READ         0x02
#define CAN_FUN_ID_TRIGGER      0x03

#define CAN_FUN_ID_RESERVE_4    0x04
#define CAN_FUN_ID_RESERVE_5    0x05

#define CAN_FUN_ID_RESET        0x06

//////  source id define  //////
#define CAN_SOURCE_ID_READ_VERSION  0x01

#define CAN_SOURCE_ID_GET_BUTTON_VALUE      0x80
#define CAN_SOURCE_ID_GET_SLIDER_VALUE      0x81


#define CAN_READ_DATA               0x80
uint16_t CmdProcessing(CAN_ID_UNION *id, uint8_t *data_in, uint16_t data_len, uint8_t *data_out)
{
    id->CanID_Struct.ACK = 1;
    switch(id->CanID_Struct.FUNC_ID)
    {
        case CAN_FUN_ID_RESET:
          //platform_mcu_reset();
          break;
        case CAN_FUN_ID_WRITE:
        case CAN_FUN_ID_READ:
          switch(id->CanID_Struct.SourceID)
          {
            case CAN_SOURCE_ID_READ_VERSION:
              //SW_VERSION
              break;
            case CAN_READ_DATA:
            
                return 1;
            default :
              break;
          }       
        default: 
          break;
    }  
    return 0;
}


#define ONLYONCE       0x00
#define BEGIN         0x01
#define TRANSING       0x02
#define END            0x03


void Can1_TX(uint32_t CANx_ID,uint8_t* pdata,uint16_t len)
{
  //return ;
	uint16_t t_len;
	uint16_t roundCount;
	uint8_t modCount;
	CAN_DATA_UNION TxMsg = {0};
	
    
	t_len = len;
	roundCount = t_len/7;
	modCount = t_len%7;
	
	TxMessage.ExtId = CANx_ID;
	TxMessage.IDE   = CAN_ID_EXT;					 //????
	TxMessage.RTR   = CAN_RTR_DATA;				 //??????
	//if(roundCount <= 1)
    if(t_len <= 7)
    {
        TxMsg.CanData_Struct.SegPolo = ONLYONCE;
        TxMessage.DLC = t_len+1;		
        
        
        memcpy(&TxMessage.Data[1],pdata,t_len);
        TxMessage.Data[0] = TxMsg.CanData[0];
        
        if((CAN_USED->TSR&0x1C000000))
        {
            CAN_Transmit(CAN1, &TxMessage);//
        }
        else
        {
            printf("TX busy ! \r\n");
        }
        return ;
    }
    
	{
		int num;
        {
            for(num = 0; num < roundCount; num++)
            {		
        //SET SEGPOLO				
                if( num == 0)
                {
                    TxMsg.CanData_Struct.SegPolo = BEGIN;
                }
                else
                {
                    TxMsg.CanData_Struct.SegPolo = TRANSING;
                }
                
                if( modCount == 0 && num == roundCount-1)
                {
                    TxMsg.CanData_Struct.SegPolo = END;
                }
                            
                TxMsg.CanData_Struct.SegNum = num;
                memcpy(TxMsg.CanData_Struct.Data, &pdata[num*7], 7);
                memcpy(TxMessage.Data, TxMsg.CanData, 8);
                TxMessage.DLC = 8;
                if((CAN_USED->TSR&0x1C000000))
                {
                    CAN_Transmit(CAN1, &TxMessage);//
                }
                else
                {
                    printf("TX busy ! \r\n");
                }
                
                //TRANSMIT LAST MSG
                if( modCount !=0 && num == roundCount-1 )
                {
                    num++;
                    TxMsg.CanData_Struct.SegPolo = END;
                    TxMsg.CanData_Struct.SegNum = num;
                    memcpy(TxMsg.CanData_Struct.Data,&pdata[num*7],modCount);
                    memcpy(TxMessage.Data,TxMsg.CanData,modCount+1);
                    TxMessage.DLC = modCount+1;
                    if((CAN_USED->TSR&0x1C000000))
                    {
                        CAN_Transmit(CAN1, &TxMessage);//
                    }
                    else
                    {
                        printf("TX busy ! \r\n");
                    }
                }
            }
            
        }
        
	}
}



CAN_LONG_BUF_T can_long_frame_buf_ram;
CAN_LONG_BUF_T *can_long_frame_buf = &can_long_frame_buf_ram;

#define CAN_LONG_BUF_FULL   0xff
static uint8_t GetOneFreeBuf(void)
{
    for(uint8_t i = 0; i < CAN_LONG_BUF_NUM; i++)
    {
        if(can_long_frame_buf->can_rcv_buf[i].used_len == 0)
        {
            return i;
        }
    }
    return CAN_LONG_BUF_FULL;
}
static void FreeBuf(uint8_t index)
{
    can_long_frame_buf->can_rcv_buf[index].can_id = 0;
    can_long_frame_buf->can_rcv_buf[index].used_len = 0;
}
#define CAN_BUF_NO_THIS_ID      0xfe
static uint8_t GetTheBufById(uint32_t id)
{
    for(uint8_t i = 0; i < CAN_LONG_BUF_NUM; i++)
    {
        if(id == can_long_frame_buf->can_rcv_buf[i].can_id)
        {
            return i;
        }
    }
    return CAN_BUF_NO_THIS_ID;
}
void CanLongBufInit(void)
{ 
    can_long_frame_buf->GetOneFreeBuf = GetOneFreeBuf;
    can_long_frame_buf->GetTheBufById = GetTheBufById;
    can_long_frame_buf->FreeBuf = FreeBuf;
    
    //my_id = GetCanMacId();//test 
}

#define CAN_LONG_FRAME_TIME_OUT     20000/SYSTICK_PERIOD

#define CAN_COMM_TIME_OUT           5000
uint32_t can_comm_start_time;

uint8_t test_rx_complete = 0;
void can_protocol_period( void )
{  
    if( test_rx_complete == 0 )
    {
      return;
    }

    CAN_ID_UNION id;
    uint16_t tx_len;
//    uint8_t test_data[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    CAN_DATA_UNION rx_buf;
//    uint8_t buf_index;
    uint8_t seg_polo;
    uint8_t seg_num;
    
    
    memcpy(&rx_buf, RxMessage.Data, RxMessage.DLC);
    
    seg_polo = rx_buf.CanData_Struct.SegPolo;
    seg_num = rx_buf.CanData_Struct.SegNum;
    
    if(seg_polo == ONLYONCE)
    {
        id.CANx_ID = RxMessage.ExtId;
        //if( (id.CanID_Struct.SourceID < SOURCE_ID_PREPARE_UPDATE) && (id.CanID_Struct.SourceID > SOURCE_ID_CHECK_TRANSMIT) )
        {
            //process the data here//
            tx_len = CmdProcessing(&id, rx_buf.CanData_Struct.Data, RxMessage.DLC - 1, CanTxdataBuff);
            //process the data here//
            
            Can1_TX( id.CANx_ID, CanTxdataBuff, tx_len );
            //Can1_TX( id.CANx_ID, test_data, sizeof(test_data) );
        }
    }
#if 0
    else //long frame
    {
        
        for(uint8_t i = 0; i < CAN_LONG_BUF_NUM; i++)
        {
            if(can_long_frame_buf->can_rcv_buf[i].used_len > 0)
            {
                if(os_get_time() - can_long_frame_buf->can_rcv_buf[i].start_time > CAN_LONG_FRAME_TIME_OUT)
                {
                    can_long_frame_buf->FreeBuf(i);
                }
            }     
        }
        
        if(seg_polo == BEGIN)
        {
            buf_index = can_long_frame_buf->GetTheBufById(RxMessage.ExtId);
            if(buf_index == CAN_BUF_NO_THIS_ID)
            {
                buf_index = can_long_frame_buf->GetOneFreeBuf();
            }
            else
            {
                //
            }
            
            if((buf_index == CAN_LONG_BUF_FULL) || (buf_index >= CAN_LONG_BUF_NUM))
            {
                //CanProtocolLog("LONG FRAME RCV BUF IS FULL! ! ! !\r\n");
                
                goto exit;
            }
            memcpy(&can_long_frame_buf->can_rcv_buf[buf_index].rcv_buf[0], rx_buf.CanData_Struct.Data, CAN_ONE_FRAME_DATA_LENTH);
            can_long_frame_buf->can_rcv_buf[buf_index].used_len = CAN_ONE_FRAME_DATA_LENTH;
            can_long_frame_buf->can_rcv_buf[buf_index].can_id = RxMessage.ExtId;
            can_long_frame_buf->can_rcv_buf[buf_index].start_time = os_get_time();
        }
        else if((seg_polo == TRANSING) || (seg_polo == END))
        {
            buf_index = can_long_frame_buf->GetTheBufById(RxMessage.ExtId);
            if((buf_index == CAN_BUF_NO_THIS_ID) || (buf_index >= CAN_LONG_BUF_NUM))
            {
//                CanProtocolLog("ERROR ! !\r\n");
                goto exit;
            }
            can_long_frame_buf->can_rcv_buf[buf_index].start_time = os_get_time();
            if(seg_polo == TRANSING)
            {
                memcpy(&can_long_frame_buf->can_rcv_buf[buf_index].rcv_buf[seg_num*CAN_ONE_FRAME_DATA_LENTH], rx_buf.CanData_Struct.Data, CAN_ONE_FRAME_DATA_LENTH);
                can_long_frame_buf->can_rcv_buf[buf_index].used_len += CAN_ONE_FRAME_DATA_LENTH;
            }
            if(seg_polo == END)
            {
                memcpy(&can_long_frame_buf->can_rcv_buf[buf_index].rcv_buf[seg_num*CAN_ONE_FRAME_DATA_LENTH], rx_buf.CanData_Struct.Data, RxMessage.DLC - 1);
                can_long_frame_buf->can_rcv_buf[buf_index].used_len += RxMessage.DLC - 1; 
                
                //process the data here//
                /**********************/
                //process the data here//
                
                Can1_TX(id.CANx_ID, can_long_frame_buf->can_rcv_buf[buf_index].rcv_buf, can_long_frame_buf->can_rcv_buf[buf_index].used_len);  // test :send the data back;             
                can_long_frame_buf->FreeBuf(buf_index);
            }       
        }
    }
#endif
    
//exit:    
    test_rx_complete = 0;
    return;
}

