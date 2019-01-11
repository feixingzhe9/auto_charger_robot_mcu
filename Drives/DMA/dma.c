#include "dma.h"


/**
  * @brief 查询模式DMA初始化
  * @param DMAy : DMA1 或 DMA2
  * @param DMAy_Channelx : 外设DMA通道，请参考《STM32中文参考手册》第148页，第P149页
  * @param PeripheralBaseAddr : 外设数据寄存器地址
  * @param MemoryBaseAddr : 存储器首地址（一般是我们定义的buffer数组的地址）
  * @param BufferSize : Buffer的大小（即我们定义的buffer数组大小）
  * @param DataType : 数据类型：0 - 8bit， 1 - 16bit， 2 - 32bit
  * @param DMA_DIR : DMA_DIR_PeripheralDST(buffer->外设) 或 DMA_DIR_PeripheralSRC(外设->buffer)
  * @param DMA_Mode: 普通传输模式(DMA_Mode_Normal或循环传输模式(DMA_Mode_Circular)
  * @retval None
  * @note
  */
void dma_poll_init(DMA_TypeDef* DMAy, DMA_Channel_TypeDef* DMAy_Channelx,
                   uint32_t PeripheralBaseAddr, uint32_t MemoryBaseAddr,
                   uint32_t BufferSize, uint32_t DataType,
                   uint32_t DMA_DIR, uint32_t DMA_Mode)
{
    DMA_InitTypeDef DMA_InitStructure;

    if(DMAy == DMA1)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//打开DMA时钟
    }
    else
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);//打开DMA时钟
    }

    /* USARTy TX DMA1 Channel (triggered by USARTy Tx event) Config */
    DMA_DeInit(DMAy_Channelx);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)PeripheralBaseAddr;//外设地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)MemoryBaseAddr;//存储器地址
    DMA_InitStructure.DMA_DIR = DMA_DIR;//DMA方向：DMA_DIR_PeripheralDST或DMA_DIR_PeripheralSRC
    DMA_InitStructure.DMA_BufferSize = BufferSize;//Buffer大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不增加
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器地址增加
    DMA_InitStructure.DMA_PeripheralDataSize = (uint32_t)(DataType << 8);//外设数据类型
    DMA_InitStructure.DMA_MemoryDataSize = (uint32_t)(DataType << 10);//Buffer数据类型
    DMA_InitStructure.DMA_Mode = DMA_Mode;//普通传输模式(DMA_Mode_Normal或循环传输模式(DMA_Mode_Circular)
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;//该DMA通道拥有高优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//非存储器到存储器的传输
    DMA_Init(DMAy_Channelx, &DMA_InitStructure);//将配置信息写入寄存器，初始化DMA
}


/**
  * @brief 开启一次DMA传输
  * @param DMAy_Channelx : 外设DMA通道，请参考《STM32中文参考手册》第148页，第P149页
  * @param BufferSize : 本次DMA传输多少个数据
  * @retval None
  * @note
  */
void dma_transfer_start(DMA_Channel_TypeDef* DMAy_Channelx, uint16_t DataNumber)
{
    DMA_Cmd(DMAy_Channelx, DISABLE);//关闭DMA通道
    DMA_SetCurrDataCounter(DMAy_Channelx, DataNumber);//设置本次DMA传输的数据量
    DMA_Cmd(DMAy_Channelx, ENABLE);//打开DMA通道
}

