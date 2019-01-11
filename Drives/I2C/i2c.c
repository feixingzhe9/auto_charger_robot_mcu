#include "i2c.h"

#define I2C_RATE_1K     5000

#define GPIO_SCL        GPIOB
#define GPIO_SDA        GPIOB
#define GPIO_Pin_SCL    GPIO_Pin_12
#define GPIO_Pin_SDA    GPIO_Pin_13
#define BIT_SCL         12
#define BIT_SDA         13

#define I2C_SCL         GPIOout(GPIO_SCL, BIT_SCL)
#define I2C_SDA         GPIOout(GPIO_SDA, BIT_SDA)
#define READ_SDA        GPIOin( GPIO_SDA, BIT_SDA)

#define SDA_IN()        {GPIO_SDA->CRH&=0XFF0FFFFF;GPIO_SDA->CRH|=8<<20;}
#define SDA_OUT()       {GPIO_SDA->CRH&=0XFF0FFFFF;GPIO_SDA->CRH|=3<<20;}

uint32_t i2c_rate = I2C_RATE_1K/100;


void i2c_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //使能GPIOB外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );

    //I2C的SDA和SCL都需要在硬件上连接上拉电阻，因此这里设置为上拉输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13);//根据以上配置初始化GPIO

    //设置SCL和SDA空闲状态为高电平
    I2C_SCL = 1;
    I2C_SDA = 1;

    i2c_rate_set(400);//设置I2C访问速度为400Kbps
}


/**
*I2C通信协议，I2C起始信号：当SCL为高电平时，SDA由高变低
*           _____     |
*        __|__   |    |  ___  ___  ___  ___  ___  ___  ___  ___
*   SDA:   |  \__|____|_/   \/   \/   \/   \/   \/   \/   \/   \     /
*          |     |    | \___/\___/\___/\___/\___/\___/\___/\___/\___/
*        __|_____|_   |   _    _    _    _    _    _    _    _    _
*   SCL:   |     | \__|__/ \__/ \__/ \__/ \__/ \__/ \__/ \__/ \__/ \_
*          |_____|    |
*           start         D7   D6   D5   D4   D3   D2   D1   D0   ACK
*/
void i2c_start(void)
{
    uint32_t i2c_delay = i2c_rate;

    SDA_OUT();//SDA设置为输出
    I2C_SDA = 1;//SDA: 高
    I2C_SCL = 1;//SCL: 高
    i2c_delay = i2c_rate;//延时>4.7us
    while(i2c_delay--){}
    I2C_SDA = 0;//当SCL为高电平时，SDA由高变低
    i2c_delay = i2c_rate;//延时>4us
    while(i2c_delay--){}
    I2C_SCL = 0;//SCL变低，钳住I2C总线，准备发送或接收数据
}


/**
*I2C通信协议，I2C停止信号：当SCL为高电平时，SDA由低变高
*发送完STOP信号后，SCL和SDA都为高电平，即释放了I2C总线
*                                    _____
*         ___  ___  ___  ___        |   __|_
*   SDA: /   \/   \/   \/   \       |  /  |
*        \___/\___/\___/\___/\______|_/   |
*         _    _    _    _    _    _|_____|_
*   SCL: / \__/ \__/ \__/ \__/ \__/ |     |
*                                   |_____|
*        D3   D2   D1   D0   ACK     stop
*/
void i2c_stop(void)
{
    uint32_t i2c_delay = i2c_rate;

    SDA_OUT();//SDA设置为输出
    I2C_SDA = 0;//SDA低电平
    I2C_SCL = 1;//SCL高电平
    i2c_delay = i2c_rate;//延时>4us
    while(i2c_delay--){}
    I2C_SDA = 1;//STOP:当SCL为高电平时，SDA由低变高
    i2c_delay = i2c_rate;
    while(i2c_delay--){}//延时>4.7us
}


/**
*I2C通信协议，检测ACK应答信号：当SCL为高电平时，读取SDA为低电平
*                             ________     _____
*         ___  ___  ___  ___ | _      |   |   __|_
*   SDA: /   \/   \/   \/   \|/ \     |   |  /  |
*        \___/\___/\___/\___/|   \____|___|_/   |
*         _    _    _    _   | _____  |  _|_____|
*   SCL: / \__/ \__/ \__/ \__|/     \_|_/ |     |
*                            |________|   |_____|
*        D3   D2   D1   D0      ACK        stop
*/
uint8_t i2c_ack_wait(void)
{
    uint32_t i2c_delay = i2c_rate;
    uint8_t timeout = 0;

    SDA_IN();//SDA设置为输入
    I2C_SDA = 1;//SDA上拉输入
    I2C_SCL = 1;//SCL设置为高电平
    i2c_delay = i2c_rate;
    while(i2c_delay--){}
    while(READ_SDA == 1)//等待ACK
    {
        if(timeout++ > 250)
        {
            i2c_stop();
            return 1;
        }
    }
    I2C_SCL = 0;//钳住I2C总线：时钟信号设为低电平

    return 0;
}


/**
*I2C通信协议，产生ACK应答信号: 在SDA为低电平时，SCL产生一个正脉冲
*                             _____     _____
*         ___  ___  ___  ___ |     |   |   __|_
*   SDA: /   \/   \/   \/   \|     |   |  /  |
*        \___/\___/\___/\___/|\____|___|_/   |
*         _    _    _    _   |  _  |  _|_____|_
*   SCL: / \__/ \__/ \__/ \__|_/ \_|_/ |     |
*                            |_____|   |_____|
*        D3   D2   D1   D0     ACK      stop
*/
void i2c_ack(void)
{
    uint32_t i2c_delay = i2c_rate;

    I2C_SCL = 0;//低电平
    SDA_OUT();//设置SDA为输出
    I2C_SDA = 0;//ACK信号
    i2c_delay = i2c_rate;
    while(i2c_delay--){}//延时>4us
    I2C_SCL = 1;//高电平
    i2c_delay = i2c_rate;
    while(i2c_delay--){}//延时>4us
    I2C_SCL = 0;//钳住I2C总线：时钟信号设为低电平
}


/**
*I2C通信协议，产生ACK应答信号: 在SDA为高电平时，SCL产生一个正脉冲
*                             _____      ______
*         ___  ___  ___  ___ | ____|_   |    __|_
*   SDA: /   \/   \/   \/   \|/    | \  |   /  |
*        \___/\___/\___/\___/|     |  \_|__/   |
*         _    _    _    _   |  _  |  __|______|_
*   SCL: / \__/ \__/ \__/ \__|_/ \_|_/  |      |
*                            |_____|    |______|
*        D3   D2   D1   D0    NACK        stop
*/
void i2c_no_ack(void)
{
    uint32_t i2c_delay = i2c_rate;

    I2C_SCL = 0;//低电平
    SDA_OUT();//SDA设置为输出
    I2C_SDA = 1;//NACK信号
    i2c_delay = i2c_rate;
    while(i2c_delay--){}//延时>4us
    I2C_SCL = 1;//高电平
    i2c_delay = i2c_rate;
    while(i2c_delay--){}//延时>4us
    I2C_SCL = 0;//钳住I2C总线：时钟信号设为低电平
}


/**
*I2C通信协议，产生ACK应答信号: 在SDA为高电平时，SCL产生一个正脉冲
*
*           _____     |<------------I2C数据发送周期------------>|
*        __|__   |    |  ___  ___  ___  ___  ___  ___  ___  ___ | _
*   SDA:   |  \__|____|_/   \/   \/   \/   \/   \/   \/   \/   \|/
*          |     |    | \___/\___/\___/\___/\___/\___/\___/\___/|\_
*        __|_____|_   |   _    _    _    _    _    _    _    _  |
*   SCL:   |     | \__|__/ \__/ \__/ \__/ \__/ \__/ \__/ \__/ \_|_
*          |_____|    |                                         |
*           start     |   D7   D6   D5   D4   D3   D2   D1   D0 |
*/
void i2c_byte_send(uint8_t pdata)
{
    uint8_t i = 0;
    uint32_t i2c_delay = i2c_rate;

    SDA_OUT();//SDA设为输出
    I2C_SCL = 0;//钳住I2C总线：SCL设为低电平
    for(i = 0; i < 8; i++)
    {
        if(pdata & 0x80)
        {
            I2C_SDA = 1;//高位先传
        }
        else
        {
            I2C_SDA = 0;
        }

        i2c_delay = i2c_rate;
        while(i2c_delay--){}//延时>4us

        I2C_SCL = 1;//在SCL上产生一个正脉冲
        i2c_delay = i2c_rate;
        while(i2c_delay--){}//延时>4us

        I2C_SCL = 0;
        i2c_delay = i2c_rate/3;
        while(i2c_delay--){}//延时>1us
        pdata <<= 1;//右移一位
    }
}


/**
*I2C通信协议，产生ACK应答信号: 在SDA为高电平时，SCL产生一个正脉冲
*
*           _____     |<------------I2C数据读取周期(ACK)------------>|
*        __|__   |    |  ___  ___  ___  ___  ___  ___  ___  ___      |
*   SDA:   |  \__|____|_/   \/   \/   \/   \/   \/   \/   \/   \     |
*          |     |    | \___/\___/\___/\___/\___/\___/\___/\___/\____|_
*        __|_____|_   |   _    _    _    _    _    _    _    _    _  |
*   SCL:   |     | \__|__/ \__/ \__/ \__/ \__/ \__/ \__/ \__/ \__/ \_|_
*          |_____|    |                                              |
*           start     |   D7   D6   D5   D4   D3   D2   D1   D0   ACK
*
*           _____     |<------------I2C数据读取周期(NACK)----------->|
*        __|__   |    |  ___  ___  ___  ___  ___  ___  ___  ___  ____|_
*   SDA:   |  \__|____|_/   \/   \/   \/   \/   \/   \/   \/   \/    |
*          |     |    | \___/\___/\___/\___/\___/\___/\___/\___/     |
*        __|_____|_   |   _    _    _    _    _    _    _    _    _  |
*   SCL:   |     | \__|__/ \__/ \__/ \__/ \__/ \__/ \__/ \__/ \__/ \_|_
*          |_____|    |                                              |
*           start     |   D7   D6   D5   D4   D3   D2   D1   D0  NACK
*/
uint8_t i2c_byte_recv(uint8_t ack)
{
    uint8_t i, receive = 0x00;
    uint32_t i2c_delay = i2c_rate;

    I2C_SCL = 0;//SCL低电平
    SDA_IN();//SDA设置为输入
    for(i = 0; i < 8; i++)
    {
        i2c_delay = i2c_rate;
        while(i2c_delay--);
        I2C_SCL = 1;//高电平
        i2c_delay = i2c_rate;
        while(i2c_delay--);
        receive <<= 1;
        if(READ_SDA)
        {
            receive |= 1;//高位在前
        }
        I2C_SCL = 0;
    }
    if (ack == 0)
    {
        i2c_no_ack();//发送NACK
    }
    else
    {
        i2c_ack();//发送ACK
    }

    return receive;//返回接收到的数据
}


uint16_t i2c_rate_set(uint16_t rate)
{
    uint16_t temp;

    //I2C速度必须小于400Kbps，大于 1Kbps
    if((rate > 400)|| (rate < 1))
    {
        return 0;
    }

    temp = I2C_RATE_1K / i2c_rate;//备份原来的i2c速度
    i2c_rate = I2C_RATE_1K / rate;//设置新的i2c速度

    return temp;//返回设置前的i2c速度
}


/**
* @brief  向设备指定地址写入单一Byte数据
* @param  dev_addr : I2C从设备地址
* @param  data_addr: 需要访问的设备内地址(如寄存器地址，EEPROM地址等)
* @param  pdata    : 写入的数据
* @retval I2C访问的结果: I2C_SUCCESS / I2C_TIMEOUT / I2C_ERROR
* @note
*   1 - 设备地址dev_addr高7bit是固定的，最低为是读/写(R/W)位，1为读，0为写
*	2 - 时序：
*           _______________________________________
*          | |         |   |        |   |    |   | |
*   Master:|S|dev_addr+W|   |data_addr|   |pdata|   |P|
*          |_|_________|___|________|___|____|___|_|
*           _______________________________________
*          | |         |   |        |   |    |   | |
*   Slave: | |         |ACK|        |ACK|    |ACK| |
*          |_|_________|___|________|___|____|___|_|
*/
I2C_STATUS_TYPE_e i2c_byte_write(uint8_t dev_addr, uint8_t data_addr, uint8_t pdata)
{
    i2c_start();//Master发送起始信号
    i2c_byte_send(dev_addr);//Master发送从设备地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_byte_send(data_addr);//发送数据地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }
    i2c_byte_send(pdata);//发送数据
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_stop();//发送停止信号

    return I2C_SUCCESS;
}


/**
* @brief  向设备指定地址连续写入数据(Burst写模式)
* @param  dev_addr : I2C从设备地址
* @param  data_addr: 需要访问的设备内地址(如寄存器地址，EEPROM地址等)
*对于Burst模式，data_addr一般是设备的FIFO,缓存，或存储设备的数据地址
* @param  *pdata  : 写入的数据首地址
* @param     Num  : 连续写入的数据个数
* @retval I2C访问的结果: I2C_SUCCESS / I2C_TIMEOUT / I2C_ERROR
* @note
*   1 - 设备地址dev_addr高7bit是固定的，最低为是读/写(R/W)位，1为读，0为写
*   2 - 时序：
*           ____________________________________________________
*          | |         |   |        |   |    |   |   |    |   | |
*   Master:|S|dev_addr+W|   |data_addr|   |pdata|   |...|pdata|   |P|
*          |_|_________|___|________|___|____|___|___|____|___|_|
*           ____________________________________________________
*          | |         |   |        |   |    |   |   |    |   | |
*   Slave: | |         |ACK|        |ACK|    |ACK|...|    |ACK| |
*          |_|_________|___|________|___|____|___|___|____|___|_|
*/
I2C_STATUS_TYPE_e i2c_single_byte_write(uint8_t dev_addr, uint16_t data_addr, uint8_t pdata)
{
    i2c_start();//Master发送起始信号
    i2c_byte_send(dev_addr);//Master发送从设备地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    //i2c_byte_send(data_addr);//发送数据地址

    i2c_byte_send((uint8_t)(data_addr>>8));
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_byte_send((uint8_t)data_addr);
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_byte_send(pdata);//发送数据
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_stop();//发送停止信号

    return I2C_SUCCESS;
}


I2C_STATUS_TYPE_e i2c_double_byte_write(uint8_t dev_addr, uint16_t data_addr, uint16_t pdata)
{
    uint8_t buffer[2];

    i2c_start();//Master发送起始信号
    i2c_byte_send(dev_addr);//Master发送从设备地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    //i2c_byte_send(data_addr);//发送数据地址

    i2c_byte_send((uint8_t)(data_addr>>8));
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_byte_send((uint8_t)data_addr);
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    buffer[0] = pdata>>8;
    buffer[1] = pdata&0xFF;
    i2c_byte_send(buffer[0]);//发送数据
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_byte_send(buffer[1]);//发送数据
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_stop();//发送停止信号

    return I2C_SUCCESS;
}


I2C_STATUS_TYPE_e i2c_multi_byte_write(uint8_t dev_addr, uint16_t data_addr, uint32_t pdata)
{
    uint8_t buffer[4];

    i2c_start();//Master发送起始信号
    i2c_byte_send(dev_addr);//Master发送从设备地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    //i2c_byte_send(data_addr);//发送数据地址

    i2c_byte_send((uint8_t)(data_addr >> 8));
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_byte_send((uint8_t)data_addr);
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    buffer[0] = pdata >> 24;
    buffer[1] = (pdata >> 16) & 0xFF;
    buffer[2] = (pdata >> 8) & 0xFF;
    buffer[3] = pdata & 0xFF;
    i2c_byte_send(buffer[0]);//发送数据
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }
    i2c_byte_send(buffer[1]);//发送数据
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }
    i2c_byte_send(buffer[2]);//发送数据
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }
    i2c_byte_send(buffer[3]);//发送数据
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_stop();//发送停止信号

	return I2C_SUCCESS;
}


/**
* @brief  从指定设备读取1Byte数据
* @param  dev_addr : I2C从设备地址
* @param  data_addr: 需要访问的设备内地址(如寄存器地址，EEPROM地址等)
* @param  *pdata   : 数据的存放地址
* @retval I2C访问的结果: I2C_SUCCESS / I2C_TIMEOUT / I2C_ERROR
* @note
*   1 - 设备地址dev_addr高7bit是固定的，最低为是读/写(R/W)位，1为读，0为写
*   2 - 时序：
*           _________________________________________________________
*          | |         |   |        |    | |         |   |    |    | |
*   Master:|S|dev_addr+W|   |data_addr|    |S|dev_addr+R|   |    |NACK|P|
*          |_|_________|___|________|____|_|_________|___|____|____|_|
*           _________________________________________________________
*          | |         |   |        |    | |         |   |    |    | |
*   Slave: | |         |ACK|        |ACK | |         |ACK|pdata|    | |
*          |_|_________|___|________|____|_|_________|___|____|____|_|
*/
I2C_STATUS_TYPE_e i2c_byte_read(uint8_t dev_addr, uint8_t data_addr, uint8_t* pdata)
{
    i2c_start();//Master发送起始信号
    i2c_byte_send(dev_addr);//Master发送从设备地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_byte_send(data_addr);//发送数据地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_start();//Master发送起始信号
    i2c_byte_send(dev_addr+1);//Master发送从设备读地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    *pdata = i2c_byte_recv(0);//读数据，NACK
    i2c_stop();//发送停止信号

    return I2C_SUCCESS;
}


/**
* @brief  向设备指定地址连续写入数据(Burst写模式)
* @param  dev_addr : I2C从设备地址
* @param  data_addr: 需要访问的设备内地址(如寄存器地址，EEPROM地址等)
* 对于Burst模式，data_addr一般是设备的FIFO,缓存，或存储设备的数据地址
* @param  *pdata  : 写入的数据首地址
* @param     Num  : 连续写入的数据个数
* @retval I2C访问的结果: I2C_SUCCESS / I2C_TIMEOUT / I2C_ERROR
* @note
*   1 - 设备地址dev_addr高7bit是固定的，最低为是读/写(R/W)位，1为读，0为写
*   2 - 时序：
*           _____________________________________________________________________
*          | |         |   |        |   | |         |   |    |   |   |    |    | |
*   Master:|S|dev_addr+W|   |data_addr|   |S|dev_addr+R|   |    |ACK|...|    |NACK|P|
*          |_|_________|___|________|___|_|_________|___|____|___|___|____|____|_|
*           _____________________________________________________________________
*          | |         |   |        |   | |         |   |    |   |   |    |    | |
*   Slave: | |         |ACK|        |ACK| |         |ACK|pdata|   |...|pdata|    | |
*          |_|_________|___|________|___|_|_________|___|____|___|___|____|____|_|
*/
I2C_STATUS_TYPE_e i2c_single_byte_read(uint8_t dev_addr, uint16_t data_addr, uint8_t* pdata)
{
    i2c_start();//Master发送起始信号
    i2c_byte_send(dev_addr);//Master发送从设备地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }
    //i2c_byte_send(data_addr);//发送数据地址

    i2c_byte_send((uint8_t)(data_addr >> 8));
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }
    i2c_byte_send((uint8_t)data_addr);
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }
    i2c_stop();//发送停止信号

    i2c_start();//Master发送起始信号
    i2c_byte_send(dev_addr + 1);//Master发送从设备读地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    *pdata = i2c_byte_recv(0);//读数据，NACK
    i2c_stop();//发送停止信号

    return I2C_SUCCESS;
}


I2C_STATUS_TYPE_e i2c_multi_byte_read(uint8_t dev_addr, uint16_t data_addr, uint8_t* pdata, uint32_t num)
{
    uint32_t i = 0;

    i2c_start();//Master发送起始信号
    i2c_byte_send(dev_addr);//Master发送从设备地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    //i2c_byte_send(data_addr);//发送数据地址

    i2c_byte_send((uint8_t)(data_addr>>8));
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }
    i2c_byte_send((uint8_t)data_addr);
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    i2c_stop();//发送停止信号

    i2c_start();//Master发送起始信号
    i2c_byte_send(dev_addr+1);//Master发送从设备读地址
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//等待ACK超时错误
    }

    for(i = 0; i < (num-1); i++)
    {
        *(pdata+i) = i2c_byte_recv(0);//读数据，NACK
    }
    *(pdata+i) = i2c_byte_recv(0);//读数据，NACK
    i2c_stop();//发送停止信号

    return I2C_SUCCESS;
}


/**
* @brief  设置数据的某一位
* @param  dev_addr : I2C从设备地址
* @param  data_addr: 需要访问的设备内地址(如寄存器地址，EEPROM地址等)
* @param  bitx  : 第几位
* @param  bit_set: 需要设置的值
* @retval I2C访问的结果: I2C_SUCCESS / I2C_TIMEOUT / I2C_ERROR
* @note
*/
I2C_STATUS_TYPE_e i2c_bit_write(uint8_t dev_addr, uint8_t data_addr, uint8_t bitx, uint8_t bit_set)
{
    I2C_STATUS_TYPE_e status = I2C_ERROR;
    uint8_t tempdata = 0;

    status = i2c_byte_read(dev_addr, data_addr, &tempdata);//获取原有数据
    if(status != I2C_SUCCESS)
    {
        return status;//I2C错误，则返回
    }
    tempdata &= ~(1 << bitx);//将要设定的位清零
    tempdata |= (bit_set<<bitx);//设置指定的bit
    status = i2c_byte_write(dev_addr, data_addr, tempdata);//写入数据

    return status;//返回状态
}

