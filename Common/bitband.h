#ifndef __BITBAND_H
#define __BITBAND_H

#include "stm32f10x.h"

#define BITBAND(addr, bitnum)    ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)          *((__IO uint32_t*)(addr))

#define GPIOout(GPIOx,bit)       MEM_ADDR(BITBAND((uint32_t)(&GPIOx->ODR), bit))
#define GPIOin(GPIOx,bit)        MEM_ADDR(BITBAND((uint32_t)(&GPIOx->IDR), bit))

#endif

