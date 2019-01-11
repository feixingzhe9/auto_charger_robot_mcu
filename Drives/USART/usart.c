#include "usart.h"

#if COM_DMA_TRANSFER
#include "dma.h"
#endif

#ifdef PRINTF_SUPPORT
#pragma import(__use_no_semihosting)
struct __FILE
{
    int handle;
};


FILE __stdout;
int _sys_exit(int x)
{
    return x;
}


int fputc(int ch, FILE *f)
{
    USART_SendData(PRINTF_SUPPORT, (uint8_t) ch);

    while (USART_GetFlagStatus(PRINTF_SUPPORT, USART_FLAG_TC) == RESET)
    {}

    return ch;
}
#endif

#define TX_BUF_NUM                 100
uint8_t uart_tx_buff[TX_BUF_NUM] = {0};
USART_TypeDef* COM_USART[COM_PORT_NUM] = {COM1, COM2, COM3};
GPIO_TypeDef* COM_TX_PORT[COM_PORT_NUM] = {COM1_TX_GPIO_PORT, COM2_TX_GPIO_PORT, COM3_TX_GPIO_PORT};
GPIO_TypeDef* COM_RX_PORT[COM_PORT_NUM] = {COM1_RX_GPIO_PORT, COM2_RX_GPIO_PORT, COM3_RX_GPIO_PORT};
const uint32_t COM_USART_CLK[COM_PORT_NUM] = {COM1_CLK, COM2_CLK, COM3_CLK};
const uint32_t COM_TX_PORT_CLK[COM_PORT_NUM] = {COM1_TX_GPIO_CLK, COM2_TX_GPIO_CLK, COM3_TX_GPIO_CLK};
const uint32_t COM_RX_PORT_CLK[COM_PORT_NUM] = {COM1_RX_GPIO_CLK, COM2_RX_GPIO_CLK, COM3_RX_GPIO_CLK};
const uint16_t COM_TX_PIN[COM_PORT_NUM] = {COM1_TX_PIN, COM2_TX_PIN, COM3_TX_PIN};
const uint16_t COM_RX_PIN[COM_PORT_NUM] = {COM1_RX_PIN, COM2_RX_PIN, COM3_RX_PIN};
const uint8_t COM_IRQn[COM_PORT_NUM] = {COM1_IRQn, COM2_IRQn, COM3_IRQn};

#if COM_DMA_TRANSFER
DMA_TypeDef* COM_DMA[COM_PORT_NUM] = {COM1_DMA, COM2_DMA, COM3_DMA};
DMA_Channel_TypeDef* COM_DMA_CH[COM_PORT_NUM][2] = {{COM1_RX_DMA_CH, COM1_TX_DMA_CH}, {COM2_RX_DMA_CH, COM2_TX_DMA_CH}, {COM3_RX_DMA_CH, COM3_TX_DMA_CH}};
const uint32_t COM_DR_BASE[COM_PORT_NUM] = { COM1_DR_Base, COM2_DR_Base, COM3_DR_Base};
#endif


void uart_gpio_init(COM_PORT_e com)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[com] | COM_RX_PORT_CLK[com] | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[com];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(COM_TX_PORT[com], &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[com];
    GPIO_Init(COM_RX_PORT[com], &GPIO_InitStructure);
}


void uart_init(COM_PORT_e com, uint32_t baud_rate)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    uart_gpio_init(com);
    if (com == COM_PORT1)
    {
        RCC_APB2PeriphClockCmd(COM_USART_CLK[com], ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(COM_USART_CLK[com], ENABLE);
    }

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = COM_IRQn[com];
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_InitStructure.USART_BaudRate = baud_rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx;
    USART_Init(COM_USART[com], &USART_InitStructure);

    USART_ITConfig(COM_USART[com], USART_IT_RXNE, ENABLE);
    USART_Cmd(COM_USART[com], ENABLE);
}


#if COM_DMA_TRANSFER
void com_dma_poll_init(COM_PORT_e com, uint32_t mem_base_addr, uint32_t buff_size, uint32_t dma_dir)
{
    DMA_TypeDef* DMAy = COM_DMA[com];
    DMA_Channel_TypeDef* DMAy_Channelx = COM_DMA_CH[com][dma_dir>>4];

    uint32_t PeripheralBaseAddr = COM_DR_BASE[com];
    uint32_t DataType = 0;

    dma_poll_init(DMAy, DMAy_Channelx, PeripheralBaseAddr, mem_base_addr, buff_size, DataType, dma_dir, DMA_Mode_Normal);

    if(dma_dir == DMA_DIR_PeripheralDST)
    {
        USART_DMACmd(COM_USART[com], USART_DMAReq_Tx, ENABLE);
    }

    if(dma_dir == DMA_DIR_PeripheralSRC)
    {
        USART_DMACmd(COM_USART[com], USART_DMAReq_Rx, ENABLE);
    }
}
#endif


void uart_dma_send_init(uint8_t* send_buffer,uint32_t baud_rate)
{
    DMA_InitTypeDef   DMA_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_BaudRate = baud_rate;

    USART_Init(USART1, &USART_InitStructure);

    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)send_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = TX_BUF_NUM;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC,ENABLE);

    USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);
}


void usart1_dma_init(uint32_t baud_rate)
{
    uart_gpio_init(COM_PORT1);
    uart_dma_send_init(uart_tx_buff, 115200);
}

