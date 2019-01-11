#ifndef __DMA_H
#define __DMA_H

#include "stm32f10x.h"


void dma_poll_init(DMA_TypeDef* DMAy, DMA_Channel_TypeDef* DMAy_Channelx,
                   uint32_t PeripheralBaseAddr, uint32_t MemoryBaseAddr,
                   uint32_t BufferSize, uint32_t DataType,
                   uint32_t DMA_DIR, uint32_t DMA_Mode);
void dma_transfer_start(DMA_Channel_TypeDef* DMAy_Channelx, uint16_t DataNumber);

#endif

