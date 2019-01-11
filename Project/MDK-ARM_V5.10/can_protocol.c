/**
******************************************************************************
* @file    can_protocol.c
* @author  Kaka
* @version V1.0.0
* @date    15-May-2017
* @brief   This file provides CAN protocol functions.
******************************************************************************

******************************************************************************
*/
#include "can_protocol.h"
#include "delay.h"
#include <string.h>
#include <stdio.h>
#include "can_fifo.h"
#include "global.h"


uint32_t can_comm_start_time;
uint8_t test_rx_complete = 0;

CanRxMsg RxMessage;
CanTxMsg TxMessage;
uint8_t CanTxdataBuff[CAN_LONG_FRAME_LENTH_MAX] = {0};

can_fifo_t can_fifo_ram;
can_fifo_t *can_fifo = &can_fifo_ram;
can_pkg_t can_pkg[CAN_FIFO_SIZE] = {0};

CAN_LONG_BUF_t can_long_frame_buf_ram;
CAN_LONG_BUF_t *can_long_frame_buf = &can_long_frame_buf_ram;

void can1_tx(uint32_t CANx_ID,uint8_t* pdata,uint16_t len)
{
    uint16_t num = 0;
    uint16_t t_len = 0;
    uint16_t roundCount = 0;
    uint8_t  modCount = 0;
    CAN_DATA_u tx_msg = {0};

    t_len = len;
    roundCount = t_len / 7;
    modCount = t_len % 7;

    TxMessage.ExtId = CANx_ID;
    TxMessage.IDE   = CAN_ID_EXT;
    TxMessage.RTR   = CAN_RTR_DATA;

    if(t_len <= 7)
    {
        tx_msg.CanData_Struct.SegPolo = ONLYONCE;
        TxMessage.DLC = t_len+1;

        memcpy(&TxMessage.Data[1], pdata, t_len);
        TxMessage.Data[0] = tx_msg.CanData[0];

        if((CAN_USED->TSR&0x1C000000))
        {
            CAN_Transmit(CAN1, &TxMessage);
        }
        else
        {
            printf("TX busy ! \r\n");
        }

        return;
    }

    for(num = 0; num < roundCount; num++)
    {
        /* Set SegPolo */
        if(num == 0)
        {
            tx_msg.CanData_Struct.SegPolo = BEGIN;
        }
        else
        {
            tx_msg.CanData_Struct.SegPolo = TRANSING;
        }

        if( modCount == 0 && num == roundCount-1)
        {
            tx_msg.CanData_Struct.SegPolo = END;
        }

        tx_msg.CanData_Struct.SegNum = num;
        memcpy(tx_msg.CanData_Struct.Data, &pdata[num*7], 7);
        memcpy(TxMessage.Data, tx_msg.CanData, 8);
        TxMessage.DLC = 8;
        if((CAN_USED->TSR & 0x1C000000))
        {
            CAN_Transmit(CAN1, &TxMessage);
        }
        else
        {
            printf("TX busy ! \r\n");
        }

        /* TRANSMIT LAST MSG */
        if( modCount !=0 && num == roundCount-1 )
        {
            num++;
            tx_msg.CanData_Struct.SegPolo = END;
            tx_msg.CanData_Struct.SegNum = num;
            memcpy(tx_msg.CanData_Struct.Data,&pdata[num*7], modCount);
            memcpy(TxMessage.Data,tx_msg.CanData,modCount+1);
            TxMessage.DLC = modCount + 1;
            if((CAN_USED->TSR & 0x1C000000))
            {
                CAN_Transmit(CAN1, &TxMessage);
            }
            else
            {
                printf("TX busy ! \r\n");
            }
        }
    }
}

void upload_multi_ir_info(void)
{
    CAN_ID_u id;
    uint8_t data_buf[6] = {0};
    id.CanID_Struct.ACK = 1;
    id.CanID_Struct.SourceID = CAN_SOURCE_ID_MULTI_IR_INFO;
    id.CanID_Struct.DestMACID = 0;
    id.CanID_Struct.SrcMACID = CAN_CHARGE_ROBOT_MAC_ID;
    id.CanID_Struct.FUNC_ID = 0;
    id.CanID_Struct.res = 0;
    data_buf[0] = ir_signal_intensity.intensity[0].left_intensity;
    data_buf[1] = ir_signal_intensity.intensity[0].right_intensity;
    data_buf[2] = ir_signal_intensity.intensity[1].left_intensity;
    data_buf[3] = ir_signal_intensity.intensity[1].right_intensity;
    data_buf[4] = ir_signal_intensity.intensity[2].left_intensity;
    data_buf[5] = ir_signal_intensity.intensity[2].right_intensity;
    can1_tx(id.CANx_ID, data_buf, 6);
}

uint16_t can_process(CAN_ID_u *id, uint8_t *data_in, uint16_t data_in_len, uint8_t *data_out)
{
    id->CanID_Struct.ACK = 1;
    id->CanID_Struct.DestMACID = id->CanID_Struct.SrcMACID;
    id->CanID_Struct.SrcMACID = CAN_CHARGE_ROBOT_MAC_ID;
    id->CanID_Struct.res = 0;

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
                    if(data_in_len == 1)
                    {
                        memcpy(&data_out[1], SW_VERSION, sizeof(SW_VERSION));
                        data_out[0] = strlen(SW_VERSION);
                        return (data_out[0] + 1);
                    }
                    break;

                case CAN_SOURCE_ID_GET_SYS_STATUS:
                    power_ctl.control_flag  = data_in[0];
                    power_ctl.vol = data_in[1];
//                    printf("can recv.data : %d\r\n", power_ctl.switch_status);
                    data_out[0] = power_ctl.switch_status;
                    data_out[1] = power_ctl.err_type;
                    data_out[2] = power_ctl.ir_left_num;
                    data_out[3] = power_ctl.ir_right_num;
                    data_out[4] = (uint8_t)(range_value/10);
                    upload_multi_ir_info();
                    return 5;

                default:
                    break;
            }
        default:
            break;
    }

    return 0;
}


static uint8_t get_one_free_buf(void)
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


static void free_buf(uint8_t index)
{
    can_long_frame_buf->can_rcv_buf[index].can_id = 0;
    can_long_frame_buf->can_rcv_buf[index].used_len = 0;
}


static uint8_t get_buff_by_id(uint32_t id)
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


void can_long_buf_init(void)
{
    can_long_frame_buf->GetOneFreeBuf = get_one_free_buf;
    can_long_frame_buf->GetTheBufById = get_buff_by_id;
    can_long_frame_buf->FreeBuf = free_buf;

    can_fifo_init(can_fifo, can_pkg, CAN_FIFO_SIZE);
}


void can_protocol_period( void )
{
    static CAN_ID_u id;
    uint16_t tx_len;
    uint8_t rx_len;
    static CAN_DATA_u rx_buf;
    can_pkg_t can_pkg_tmp;
    uint8_t buf_index;
    uint8_t seg_polo;
    uint8_t seg_num;

    while(is_can_fifo_empty(can_fifo) == FALSE)
    {
        can_fifo_get_pkg(can_fifo, &can_pkg_tmp);

        memcpy(rx_buf.CanData,  can_pkg_tmp.data.CanData, can_pkg_tmp.len);
        id.CANx_ID = can_pkg_tmp.id.CANx_ID;
        rx_len = can_pkg_tmp.len;
        seg_polo = rx_buf.CanData_Struct.SegPolo;
        seg_num = rx_buf.CanData_Struct.SegNum;

        if(seg_polo == ONLYONCE)
        {
            tx_len = can_process(&id, rx_buf.CanData_Struct.Data, rx_len - 1, CanTxdataBuff );

            if(tx_len > 0)
            {
                can1_tx( id.CANx_ID, CanTxdataBuff, tx_len );
            }
        }
        else/* long frame */
        {
            for(uint8_t i = 0; i < CAN_LONG_BUF_NUM; i++)
            {
                if(can_long_frame_buf->can_rcv_buf[i].used_len > 0)
                {
                    if(os_get_time() - can_long_frame_buf->can_rcv_buf[i].start_time > CAN_LONG_FRAME_TIME_OUT)
                    {
                        can_long_frame_buf->FreeBuf(i);
                        printf("LONG FRAME RCV TIMEOUT! ! ! !\r\n");
                    }
                }
            }

            if(seg_polo == BEGIN)
            {
                buf_index = can_long_frame_buf->GetTheBufById(id.CANx_ID);
                if(buf_index == CAN_BUF_NO_THIS_ID)
                {
                    buf_index = can_long_frame_buf->GetOneFreeBuf();
                }
                else
                {

                }

                if( (buf_index == CAN_LONG_BUF_FULL) || (buf_index >= CAN_LONG_BUF_NUM) )
                {
                    printf("LONG FRAME RCV BUF IS FULL! ! ! !\r\n");
                    continue;
                }

                memcpy(&can_long_frame_buf->can_rcv_buf[buf_index].rcv_buf[0], rx_buf.CanData_Struct.Data, CAN_ONE_FRAME_DATA_LENTH);
                can_long_frame_buf->can_rcv_buf[buf_index].used_len = CAN_ONE_FRAME_DATA_LENTH;
                can_long_frame_buf->can_rcv_buf[buf_index].can_id = id.CANx_ID;
                can_long_frame_buf->can_rcv_buf[buf_index].start_time = os_get_time();
            }
            else if((seg_polo == TRANSING) || (seg_polo == END))
            {
                buf_index = can_long_frame_buf->GetTheBufById(id.CANx_ID);
                if((buf_index == CAN_BUF_NO_THIS_ID) || (buf_index >= CAN_LONG_BUF_NUM))
                {
                    printf("ERROR ! !\r\n");
                    continue;
                }

                can_long_frame_buf->can_rcv_buf[buf_index].start_time = os_get_time();
                if(seg_polo == TRANSING)
                {
                    memcpy(&can_long_frame_buf->can_rcv_buf[buf_index].rcv_buf[seg_num*CAN_ONE_FRAME_DATA_LENTH], rx_buf.CanData_Struct.Data, CAN_ONE_FRAME_DATA_LENTH);
                    can_long_frame_buf->can_rcv_buf[buf_index].used_len += CAN_ONE_FRAME_DATA_LENTH;
                }
                if(seg_polo == END)
                {
                    memcpy(&can_long_frame_buf->can_rcv_buf[buf_index].rcv_buf[seg_num*CAN_ONE_FRAME_DATA_LENTH], rx_buf.CanData_Struct.Data, rx_len - 1);
                    can_long_frame_buf->can_rcv_buf[buf_index].used_len += rx_len - 1;

                    printf("long frame receive complete\r\n");
                    for(uint8_t j = 0; j < can_long_frame_buf->can_rcv_buf[buf_index].used_len; j++)
                    {
                      printf("data[%d]: %d\r\n",j,can_long_frame_buf->can_rcv_buf[buf_index].rcv_buf[j]);
                    }

                    can1_tx(id.CANx_ID, can_long_frame_buf->can_rcv_buf[buf_index].rcv_buf, can_long_frame_buf->can_rcv_buf[buf_index].used_len);// test :send the data back;             
                    can_long_frame_buf->FreeBuf(buf_index);
                }
            }
        }
    }
}

