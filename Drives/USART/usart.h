#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include "remote.h"

#define PRINTF_SUPPORT      PRINT_COM

#ifdef PRINTF_SUPPORT
#include <stdio.h>
#endif

#define COM_DMA_TRANSFER    1
#define PRINT_COM           COM1

#define COM1                USART1
#define COM1_CLK            RCC_APB2Periph_USART1
#define COM1_TX_PIN         GPIO_Pin_9
#define COM1_TX_GPIO_PORT   GPIOA
#define COM1_TX_GPIO_CLK    RCC_APB2Periph_GPIOA
#define COM1_RX_PIN         GPIO_Pin_10
#define COM1_RX_GPIO_PORT   GPIOA
#define COM1_RX_GPIO_CLK    RCC_APB2Periph_GPIOA
#define COM1_IRQn           USART1_IRQn
#define COM1_DR_Base        ((uint32_t)0x40013804)
#define COM1_DMA            ((DMA_TypeDef*)DMA1)
#define COM1_TX_DMA_CH      ((DMA_Channel_TypeDef*)DMA1_Channel4)
#define COM1_RX_DMA_CH      ((DMA_Channel_TypeDef*)DMA1_Channel5)

#define COM2                USART2
#define COM2_CLK            RCC_APB1Periph_USART2
#define COM2_TX_PIN         GPIO_Pin_2
#define COM2_TX_GPIO_PORT   GPIOA
#define COM2_TX_GPIO_CLK    RCC_APB2Periph_GPIOA
#define COM2_RX_PIN         GPIO_Pin_3
#define COM2_RX_GPIO_PORT   GPIOA
#define COM2_RX_GPIO_CLK    RCC_APB2Periph_GPIOA
#define COM2_IRQn           USART2_IRQn
#define COM2_DR_Base        ((uint32_t)0x40004404)
#define COM2_DMA            ((DMA_TypeDef*)DMA1)
#define COM2_TX_DMA_CH      ((DMA_Channel_TypeDef*)DMA1_Channel7)
#define COM2_RX_DMA_CH      ((DMA_Channel_TypeDef*)DMA1_Channel6)

#define COM3                USART3
#define COM3_CLK            RCC_APB1Periph_USART3
#define COM3_TX_PIN         GPIO_Pin_10
#define COM3_TX_GPIO_PORT   GPIOB
#define COM3_TX_GPIO_CLK    RCC_APB2Periph_GPIOB
#define COM3_RX_PIN         GPIO_Pin_11
#define COM3_RX_GPIO_PORT   GPIOB
#define COM3_RX_GPIO_CLK    RCC_APB2Periph_GPIOB
#define COM3_IRQn           USART3_IRQn
#define COM3_DR_Base        ((uint32_t)0x40004804)
#define COM3_DMA            ((DMA_TypeDef*)DMA1)
#define COM3_TX_DMA_CH      ((DMA_Channel_TypeDef*)DMA1_Channel2)
#define COM3_RX_DMA_CH      ((DMA_Channel_TypeDef*)DMA1_Channel3)

typedef enum
{
    COM_PORT1 = 0,
    COM_PORT2 = 1,
    COM_PORT3 = 2,
    COM_PORT_NUM
} COM_PORT_e;

void uart_init(COM_PORT_e com, uint32_t baud_rate);
void com_dma_poll_init(COM_PORT_e com, uint32_t mem_base_addr, uint32_t buff_size, uint32_t dma_dir);
void usart1_dma_init(uint32_t baud_rate);

#endif
