#ifndef __VL6180X_H
#define __VL6180X_H

#include "stm32f10x.h"
#include "bitband.h"


#define VL6180X_ADDRESS                        0x52
#define EXPANDER_I2C_ADDRESS                   (0x42*2)
#define SYSTEM__MODE_GPIO0                     0x10
#define SYSTEM__HISTORY_CTRL                   0x12
#define SYSTEM__INTERRUPT_CONFIG_GPIO          0x14
#define I2C_SLAVE__DEVICE_ADDRESS              0x212
#define RESULT__RANGE_VAL                      0x062
#define READOUT__AVERAGING_SAMPLE_PERIOD       0x10A
#define SYSRANGE__EARLY_CONVERGENCE_ESTIMATE   0x22
#define SYSRANGE__START                        0x18
#define RESULT__INTERRUPT_STATUS_GPIO          0x4F
#define SYSTEM__INTERRUPT_CLEAR                0x15

#define SYSRANGE_MAX_CONVERGENCE_TIME          0x01C
#define SYSTEM_MODE_GPIO1                      0x011
#define SYSTEM_INTERRUPT_CLEAR                 0x015
#define RESULT__RANGE_STATUS                   0x4D

#define SYSRANGE__RANGE_IGNORE_VALID_HEIGHT    0x025
#define RANGE_SCALER                           0x096
#define SYSRANGE_PART_TO_PART_RANGE_OFFSET     0x024

#define MAX_RANGE                              500


extern volatile uint16_t range_value;


void vl6180x_init(void);
uint8_t vl6180x_start(void);
uint8_t vl6180x_finish(void);

#endif

