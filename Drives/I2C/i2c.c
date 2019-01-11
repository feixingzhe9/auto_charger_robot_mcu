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

    //ʹ��GPIOB����ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );

    //I2C��SDA��SCL����Ҫ��Ӳ���������������裬�����������Ϊ�������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//��©���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13);//�����������ó�ʼ��GPIO

    //����SCL��SDA����״̬Ϊ�ߵ�ƽ
    I2C_SCL = 1;
    I2C_SDA = 1;

    i2c_rate_set(400);//����I2C�����ٶ�Ϊ400Kbps
}


/**
*I2Cͨ��Э�飬I2C��ʼ�źţ���SCLΪ�ߵ�ƽʱ��SDA�ɸ߱��
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

    SDA_OUT();//SDA����Ϊ���
    I2C_SDA = 1;//SDA: ��
    I2C_SCL = 1;//SCL: ��
    i2c_delay = i2c_rate;//��ʱ>4.7us
    while(i2c_delay--){}
    I2C_SDA = 0;//��SCLΪ�ߵ�ƽʱ��SDA�ɸ߱��
    i2c_delay = i2c_rate;//��ʱ>4us
    while(i2c_delay--){}
    I2C_SCL = 0;//SCL��ͣ�ǯסI2C���ߣ�׼�����ͻ��������
}


/**
*I2Cͨ��Э�飬I2Cֹͣ�źţ���SCLΪ�ߵ�ƽʱ��SDA�ɵͱ��
*������STOP�źź�SCL��SDA��Ϊ�ߵ�ƽ�����ͷ���I2C����
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

    SDA_OUT();//SDA����Ϊ���
    I2C_SDA = 0;//SDA�͵�ƽ
    I2C_SCL = 1;//SCL�ߵ�ƽ
    i2c_delay = i2c_rate;//��ʱ>4us
    while(i2c_delay--){}
    I2C_SDA = 1;//STOP:��SCLΪ�ߵ�ƽʱ��SDA�ɵͱ��
    i2c_delay = i2c_rate;
    while(i2c_delay--){}//��ʱ>4.7us
}


/**
*I2Cͨ��Э�飬���ACKӦ���źţ���SCLΪ�ߵ�ƽʱ����ȡSDAΪ�͵�ƽ
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

    SDA_IN();//SDA����Ϊ����
    I2C_SDA = 1;//SDA��������
    I2C_SCL = 1;//SCL����Ϊ�ߵ�ƽ
    i2c_delay = i2c_rate;
    while(i2c_delay--){}
    while(READ_SDA == 1)//�ȴ�ACK
    {
        if(timeout++ > 250)
        {
            i2c_stop();
            return 1;
        }
    }
    I2C_SCL = 0;//ǯסI2C���ߣ�ʱ���ź���Ϊ�͵�ƽ

    return 0;
}


/**
*I2Cͨ��Э�飬����ACKӦ���ź�: ��SDAΪ�͵�ƽʱ��SCL����һ��������
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

    I2C_SCL = 0;//�͵�ƽ
    SDA_OUT();//����SDAΪ���
    I2C_SDA = 0;//ACK�ź�
    i2c_delay = i2c_rate;
    while(i2c_delay--){}//��ʱ>4us
    I2C_SCL = 1;//�ߵ�ƽ
    i2c_delay = i2c_rate;
    while(i2c_delay--){}//��ʱ>4us
    I2C_SCL = 0;//ǯסI2C���ߣ�ʱ���ź���Ϊ�͵�ƽ
}


/**
*I2Cͨ��Э�飬����ACKӦ���ź�: ��SDAΪ�ߵ�ƽʱ��SCL����һ��������
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

    I2C_SCL = 0;//�͵�ƽ
    SDA_OUT();//SDA����Ϊ���
    I2C_SDA = 1;//NACK�ź�
    i2c_delay = i2c_rate;
    while(i2c_delay--){}//��ʱ>4us
    I2C_SCL = 1;//�ߵ�ƽ
    i2c_delay = i2c_rate;
    while(i2c_delay--){}//��ʱ>4us
    I2C_SCL = 0;//ǯסI2C���ߣ�ʱ���ź���Ϊ�͵�ƽ
}


/**
*I2Cͨ��Э�飬����ACKӦ���ź�: ��SDAΪ�ߵ�ƽʱ��SCL����һ��������
*
*           _____     |<------------I2C���ݷ�������------------>|
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

    SDA_OUT();//SDA��Ϊ���
    I2C_SCL = 0;//ǯסI2C���ߣ�SCL��Ϊ�͵�ƽ
    for(i = 0; i < 8; i++)
    {
        if(pdata & 0x80)
        {
            I2C_SDA = 1;//��λ�ȴ�
        }
        else
        {
            I2C_SDA = 0;
        }

        i2c_delay = i2c_rate;
        while(i2c_delay--){}//��ʱ>4us

        I2C_SCL = 1;//��SCL�ϲ���һ��������
        i2c_delay = i2c_rate;
        while(i2c_delay--){}//��ʱ>4us

        I2C_SCL = 0;
        i2c_delay = i2c_rate/3;
        while(i2c_delay--){}//��ʱ>1us
        pdata <<= 1;//����һλ
    }
}


/**
*I2Cͨ��Э�飬����ACKӦ���ź�: ��SDAΪ�ߵ�ƽʱ��SCL����һ��������
*
*           _____     |<------------I2C���ݶ�ȡ����(ACK)------------>|
*        __|__   |    |  ___  ___  ___  ___  ___  ___  ___  ___      |
*   SDA:   |  \__|____|_/   \/   \/   \/   \/   \/   \/   \/   \     |
*          |     |    | \___/\___/\___/\___/\___/\___/\___/\___/\____|_
*        __|_____|_   |   _    _    _    _    _    _    _    _    _  |
*   SCL:   |     | \__|__/ \__/ \__/ \__/ \__/ \__/ \__/ \__/ \__/ \_|_
*          |_____|    |                                              |
*           start     |   D7   D6   D5   D4   D3   D2   D1   D0   ACK
*
*           _____     |<------------I2C���ݶ�ȡ����(NACK)----------->|
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

    I2C_SCL = 0;//SCL�͵�ƽ
    SDA_IN();//SDA����Ϊ����
    for(i = 0; i < 8; i++)
    {
        i2c_delay = i2c_rate;
        while(i2c_delay--);
        I2C_SCL = 1;//�ߵ�ƽ
        i2c_delay = i2c_rate;
        while(i2c_delay--);
        receive <<= 1;
        if(READ_SDA)
        {
            receive |= 1;//��λ��ǰ
        }
        I2C_SCL = 0;
    }
    if (ack == 0)
    {
        i2c_no_ack();//����NACK
    }
    else
    {
        i2c_ack();//����ACK
    }

    return receive;//���ؽ��յ�������
}


uint16_t i2c_rate_set(uint16_t rate)
{
    uint16_t temp;

    //I2C�ٶȱ���С��400Kbps������ 1Kbps
    if((rate > 400)|| (rate < 1))
    {
        return 0;
    }

    temp = I2C_RATE_1K / i2c_rate;//����ԭ����i2c�ٶ�
    i2c_rate = I2C_RATE_1K / rate;//�����µ�i2c�ٶ�

    return temp;//��������ǰ��i2c�ٶ�
}


/**
* @brief  ���豸ָ����ַд�뵥һByte����
* @param  dev_addr : I2C���豸��ַ
* @param  data_addr: ��Ҫ���ʵ��豸�ڵ�ַ(��Ĵ�����ַ��EEPROM��ַ��)
* @param  pdata    : д�������
* @retval I2C���ʵĽ��: I2C_SUCCESS / I2C_TIMEOUT / I2C_ERROR
* @note
*   1 - �豸��ַdev_addr��7bit�ǹ̶��ģ����Ϊ�Ƕ�/д(R/W)λ��1Ϊ����0Ϊд
*	2 - ʱ��
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
    i2c_start();//Master������ʼ�ź�
    i2c_byte_send(dev_addr);//Master���ʹ��豸��ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_byte_send(data_addr);//�������ݵ�ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }
    i2c_byte_send(pdata);//��������
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_stop();//����ֹͣ�ź�

    return I2C_SUCCESS;
}


/**
* @brief  ���豸ָ����ַ����д������(Burstдģʽ)
* @param  dev_addr : I2C���豸��ַ
* @param  data_addr: ��Ҫ���ʵ��豸�ڵ�ַ(��Ĵ�����ַ��EEPROM��ַ��)
*����Burstģʽ��data_addrһ�����豸��FIFO,���棬��洢�豸�����ݵ�ַ
* @param  *pdata  : д��������׵�ַ
* @param     Num  : ����д������ݸ���
* @retval I2C���ʵĽ��: I2C_SUCCESS / I2C_TIMEOUT / I2C_ERROR
* @note
*   1 - �豸��ַdev_addr��7bit�ǹ̶��ģ����Ϊ�Ƕ�/д(R/W)λ��1Ϊ����0Ϊд
*   2 - ʱ��
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
    i2c_start();//Master������ʼ�ź�
    i2c_byte_send(dev_addr);//Master���ʹ��豸��ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    //i2c_byte_send(data_addr);//�������ݵ�ַ

    i2c_byte_send((uint8_t)(data_addr>>8));
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_byte_send((uint8_t)data_addr);
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_byte_send(pdata);//��������
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_stop();//����ֹͣ�ź�

    return I2C_SUCCESS;
}


I2C_STATUS_TYPE_e i2c_double_byte_write(uint8_t dev_addr, uint16_t data_addr, uint16_t pdata)
{
    uint8_t buffer[2];

    i2c_start();//Master������ʼ�ź�
    i2c_byte_send(dev_addr);//Master���ʹ��豸��ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    //i2c_byte_send(data_addr);//�������ݵ�ַ

    i2c_byte_send((uint8_t)(data_addr>>8));
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_byte_send((uint8_t)data_addr);
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    buffer[0] = pdata>>8;
    buffer[1] = pdata&0xFF;
    i2c_byte_send(buffer[0]);//��������
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_byte_send(buffer[1]);//��������
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_stop();//����ֹͣ�ź�

    return I2C_SUCCESS;
}


I2C_STATUS_TYPE_e i2c_multi_byte_write(uint8_t dev_addr, uint16_t data_addr, uint32_t pdata)
{
    uint8_t buffer[4];

    i2c_start();//Master������ʼ�ź�
    i2c_byte_send(dev_addr);//Master���ʹ��豸��ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    //i2c_byte_send(data_addr);//�������ݵ�ַ

    i2c_byte_send((uint8_t)(data_addr >> 8));
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_byte_send((uint8_t)data_addr);
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    buffer[0] = pdata >> 24;
    buffer[1] = (pdata >> 16) & 0xFF;
    buffer[2] = (pdata >> 8) & 0xFF;
    buffer[3] = pdata & 0xFF;
    i2c_byte_send(buffer[0]);//��������
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }
    i2c_byte_send(buffer[1]);//��������
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }
    i2c_byte_send(buffer[2]);//��������
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }
    i2c_byte_send(buffer[3]);//��������
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_stop();//����ֹͣ�ź�

	return I2C_SUCCESS;
}


/**
* @brief  ��ָ���豸��ȡ1Byte����
* @param  dev_addr : I2C���豸��ַ
* @param  data_addr: ��Ҫ���ʵ��豸�ڵ�ַ(��Ĵ�����ַ��EEPROM��ַ��)
* @param  *pdata   : ���ݵĴ�ŵ�ַ
* @retval I2C���ʵĽ��: I2C_SUCCESS / I2C_TIMEOUT / I2C_ERROR
* @note
*   1 - �豸��ַdev_addr��7bit�ǹ̶��ģ����Ϊ�Ƕ�/д(R/W)λ��1Ϊ����0Ϊд
*   2 - ʱ��
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
    i2c_start();//Master������ʼ�ź�
    i2c_byte_send(dev_addr);//Master���ʹ��豸��ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_byte_send(data_addr);//�������ݵ�ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_start();//Master������ʼ�ź�
    i2c_byte_send(dev_addr+1);//Master���ʹ��豸����ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    *pdata = i2c_byte_recv(0);//�����ݣ�NACK
    i2c_stop();//����ֹͣ�ź�

    return I2C_SUCCESS;
}


/**
* @brief  ���豸ָ����ַ����д������(Burstдģʽ)
* @param  dev_addr : I2C���豸��ַ
* @param  data_addr: ��Ҫ���ʵ��豸�ڵ�ַ(��Ĵ�����ַ��EEPROM��ַ��)
* ����Burstģʽ��data_addrһ�����豸��FIFO,���棬��洢�豸�����ݵ�ַ
* @param  *pdata  : д��������׵�ַ
* @param     Num  : ����д������ݸ���
* @retval I2C���ʵĽ��: I2C_SUCCESS / I2C_TIMEOUT / I2C_ERROR
* @note
*   1 - �豸��ַdev_addr��7bit�ǹ̶��ģ����Ϊ�Ƕ�/д(R/W)λ��1Ϊ����0Ϊд
*   2 - ʱ��
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
    i2c_start();//Master������ʼ�ź�
    i2c_byte_send(dev_addr);//Master���ʹ��豸��ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }
    //i2c_byte_send(data_addr);//�������ݵ�ַ

    i2c_byte_send((uint8_t)(data_addr >> 8));
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }
    i2c_byte_send((uint8_t)data_addr);
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }
    i2c_stop();//����ֹͣ�ź�

    i2c_start();//Master������ʼ�ź�
    i2c_byte_send(dev_addr + 1);//Master���ʹ��豸����ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    *pdata = i2c_byte_recv(0);//�����ݣ�NACK
    i2c_stop();//����ֹͣ�ź�

    return I2C_SUCCESS;
}


I2C_STATUS_TYPE_e i2c_multi_byte_read(uint8_t dev_addr, uint16_t data_addr, uint8_t* pdata, uint32_t num)
{
    uint32_t i = 0;

    i2c_start();//Master������ʼ�ź�
    i2c_byte_send(dev_addr);//Master���ʹ��豸��ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    //i2c_byte_send(data_addr);//�������ݵ�ַ

    i2c_byte_send((uint8_t)(data_addr>>8));
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }
    i2c_byte_send((uint8_t)data_addr);
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    i2c_stop();//����ֹͣ�ź�

    i2c_start();//Master������ʼ�ź�
    i2c_byte_send(dev_addr+1);//Master���ʹ��豸����ַ
    if(i2c_ack_wait())
    {
        return I2C_TIMEOUT;//�ȴ�ACK��ʱ����
    }

    for(i = 0; i < (num-1); i++)
    {
        *(pdata+i) = i2c_byte_recv(0);//�����ݣ�NACK
    }
    *(pdata+i) = i2c_byte_recv(0);//�����ݣ�NACK
    i2c_stop();//����ֹͣ�ź�

    return I2C_SUCCESS;
}


/**
* @brief  �������ݵ�ĳһλ
* @param  dev_addr : I2C���豸��ַ
* @param  data_addr: ��Ҫ���ʵ��豸�ڵ�ַ(��Ĵ�����ַ��EEPROM��ַ��)
* @param  bitx  : �ڼ�λ
* @param  bit_set: ��Ҫ���õ�ֵ
* @retval I2C���ʵĽ��: I2C_SUCCESS / I2C_TIMEOUT / I2C_ERROR
* @note
*/
I2C_STATUS_TYPE_e i2c_bit_write(uint8_t dev_addr, uint8_t data_addr, uint8_t bitx, uint8_t bit_set)
{
    I2C_STATUS_TYPE_e status = I2C_ERROR;
    uint8_t tempdata = 0;

    status = i2c_byte_read(dev_addr, data_addr, &tempdata);//��ȡԭ������
    if(status != I2C_SUCCESS)
    {
        return status;//I2C�����򷵻�
    }
    tempdata &= ~(1 << bitx);//��Ҫ�趨��λ����
    tempdata |= (bit_set<<bitx);//����ָ����bit
    status = i2c_byte_write(dev_addr, data_addr, tempdata);//д������

    return status;//����״̬
}

