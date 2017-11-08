#ifndef _CAN__H
#define _CAN__H


#include "stm32f10x.h"

extern uint8_t CanInit(void);

//CAN1
#define CAN1_GPIO_RX_PORT		GPIOA
#define CAN1_GPIO_RX_PIN		GPIO_Pin_11
#define CAN1_GPIO_TX_PORT		GPIOA
#define CAN1_GPIO_TX_PIN		GPIO_Pin_12

extern void CanLongBufInit(void);






#endif
