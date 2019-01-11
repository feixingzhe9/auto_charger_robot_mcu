#ifndef __I2C_H
#define __I2C_H

#include "stm32f10x.h"
#include "bitband.h"

typedef enum
{
    I2C_SUCCESS = 0,
    I2C_TIMEOUT,
    I2C_ERROR,
}I2C_STATUS_TYPE_e;

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_ack_wait(void);
void i2c_ack(void);
void i2c_no_ack(void);
void i2c_byte_send(uint8_t pdata);
uint8_t i2c_byte_recv(uint8_t ack);
uint16_t i2c_rate_set(uint16_t rate);

I2C_STATUS_TYPE_e i2c_byte_write(uint8_t dev_addr, uint8_t data_addr, uint8_t pdata);
I2C_STATUS_TYPE_e i2c_byte_read(uint8_t dev_addr, uint8_t data_addr, uint8_t* pdata);
I2C_STATUS_TYPE_e i2c_bit_write(uint8_t dev_addr, uint8_t data_addr, uint8_t bitx, uint8_t bit_set);
I2C_STATUS_TYPE_e i2c_single_byte_write(uint8_t dev_addr, uint16_t data_addr, uint8_t pdata);
I2C_STATUS_TYPE_e i2c_double_byte_write(uint8_t dev_addr, uint16_t data_addr, uint16_t pdata);
I2C_STATUS_TYPE_e i2c_multi_byte_write(uint8_t dev_addr, uint16_t data_addr, uint32_t pdata);
I2C_STATUS_TYPE_e i2c_single_byte_read(uint8_t dev_addr, uint16_t data_addr, uint8_t* pdata);
I2C_STATUS_TYPE_e i2c_multi_byte_read(uint8_t dev_addr, uint16_t data_addr, uint8_t* pdata, uint32_t num);

#endif

