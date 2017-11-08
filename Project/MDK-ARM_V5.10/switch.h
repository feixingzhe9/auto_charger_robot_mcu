
#include "stm32f10x.h"
#include "bitband.h"	//使用位带操作前，先包含此头文件
#include "global.h"

#define SWITCH_VALUE_NUM	30

#define SWITCH1_SCAN		GPIOin(GPIOA, 0)	
#define SWITCH2_SCAN		GPIOin(GPIOA, 1)	

#define SWITCH_IN			0
#define SWITCH_OUT		1

#define MOTOR1_DIR		GPIOout(GPIOB, 7)	//定义PB12位带操作

void swtich_init(void);

uint8_t switch_scan(void);
