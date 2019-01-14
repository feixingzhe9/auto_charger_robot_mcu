#include "remote.h"
#include "delay.h"
#include "usart.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "platform.h"

//#define RCC_TIMER4           RCC_APB1Periph_TIM4
//#define RCC_IR               RCC_APB2Periph_GPIOB
#define GPIO_IR_RECV         GPIOB
#define GPIO_Pin_IR_RECV     GPIO_Pin_7
#define BIT_IR               7
#define TIMER_IR             TIM4
#define TIM_CHANNEL_IR       TIM_Channel_2
#define TIM_INT_IR           TIM4_IRQn
#define PERIOD_TIMER4        10000/* ��ʱ���Զ���װ 1us*10000 = 10ms */
#define PRESCALER_TIMER4     71/* Ԥ��Ƶ��,72M/71+1�ļ���Ƶ��:1M, 1us */
#define INPUT_FILTER         0x03/* ���������˲��� 8����ʱ��ʱ�������˲� */

//#define RDATA                GPIOin(GPIO_IR_RECV, BIT_IR)

#define RDATA_0     GPIOin(GPIOB,7)
#define RDATA_1     GPIOin(GPIOB,9)
#define RDATA_2     GPIOin(GPIOB,8)


#define VALUE_NUM            20

#define REMOTE_LEFT          0x68
#define REMOTE_RIGHT         0x9A

#define REMOTE_POWER_ON      0x5A
#define REMOTE_POWER_OFF     0xA5

uint8_t  remote_state[REMOTE_RCV_INTERFACE_NUM] = {0};
uint16_t fall_edge_val[REMOTE_RCV_INTERFACE_NUM] = {0};
uint32_t remote_recv[REMOTE_RCV_INTERFACE_NUM] = {0};
uint8_t  remote_cnt[REMOTE_RCV_INTERFACE_NUM] = {0};
uint32_t remote_start_tick[REMOTE_RCV_INTERFACE_NUM] = {0};
uint32_t timer_tick = 0;

static uint8_t left_value[REMOTE_RCV_INTERFACE_NUM][VALUE_NUM] = {0};
static uint8_t right_value[REMOTE_RCV_INTERFACE_NUM][VALUE_NUM] = {0};


void remote_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 | RCC_APB1Periph_TIM2 /*| RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM5*/, ENABLE);
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);

    TIM_TimeBaseStructure.TIM_Period = 10000;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_TimeBaseStructure.TIM_Period = 5000;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

//    TIM_TimeBaseStructure.TIM_Period = 2500;
//    TIM_TimeBaseStructure.TIM_Prescaler = 71;
//    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
//    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
//    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
//    TIM_ClearFlag(TIM1, TIM_FLAG_Update);


    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter =  0x03;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter =  0x03;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter =  0x03;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);

    TIM_Cmd(TIMER_IR, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
//    TIM_Cmd(TIM1, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM_INT_IR;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);



//    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);



    TIM_ITConfig(TIMER_IR, /*TIM_IT_Update |*/ TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update , ENABLE);
//    TIM_ITConfig(TIM1, TIM_IT_Update , ENABLE);
}


uint32_t tim_1_cnt_test = 0;
uint32_t tim_2_cnt_test = 0;
uint32_t tim_4_cnt_test = 0;
void TIM1_UP_IRQHandler(void)
{
//    uint8_t i = 2;
//    tim_1_cnt_test++;
//    if(TIM_GetITStatus(TIM1, TIM_IT_Update)!= RESET)
//    {
//        if(remote_state[i] & 0x80)/* �ϴ������ݱ����յ��� */
//        {
//            remote_state[i] &= ~0x10;//ȡ���������Ѿ���������
//            if((remote_state[i] & 0x0F) == 0x00)
//            {
//                remote_state[i] |= 1<<6;//����Ѿ����һ�ΰ����ļ�ֵ��Ϣ�ɼ�
//            }

//            if((remote_state[i] & 0x0F) < 14)
//            {
//                remote_state[i]++;
//            }
//            else
//            {
//                remote_state[i] &= ~(1<<7);//���������ʶ
//                remote_state[i] &= 0xF0;//��ռ�����
//            }
//        }
//    }
//    TIM_ClearFlag(TIM1, TIM_IT_Update);
}

void TIM2_IRQHandler(void)
{
    uint8_t i = 0;
    tim_2_cnt_test++;
    timer_tick++;
    if(TIM_GetITStatus(TIM2, TIM_IT_Update)!= RESET)
    {
        for(i = 0; i < REMOTE_RCV_INTERFACE_NUM; i++)
        {
            if(timer_tick - remote_start_tick[i] >= 10)
            {
                remote_start_tick[i] = timer_tick;
                if(remote_state[i] & 0x80)/* �ϴ������ݱ����յ��� */
                {
                    remote_state[i] &= ~0x10;//ȡ���������Ѿ���������
                    if((remote_state[i] & 0x0F) == 0x00)
                    {
                        remote_state[i] |= 1<<6;//����Ѿ����һ�ΰ����ļ�ֵ��Ϣ�ɼ�
                    }

                    if((remote_state[i] & 0x0F) < 14)
                    {
                        remote_state[i]++;
                    }
                    else
                    {
                        remote_state[i] &= ~(1<<7);//���������ʶ
                        remote_state[i] &= 0xF0;//��ռ�����
                    }
                }
            }

        }
    }


    TIM_ClearFlag(TIM2, TIM_IT_Update);
}



/*******************************
**ң��������״̬
**[7]:�յ����������־
**[6]:�õ���һ��������������Ϣ
**[5]:����
**[4]:����������Ƿ��Ѿ�������
**[3:0]:�����ʱ
********************************/
void TIM4_IRQHandler(void)
{
    uint8_t i = 1;
    if(TIM_GetITStatus(TIMER_IR, TIM_IT_Update)!= RESET)
    {
        tim_4_cnt_test++;
//        for(i = 0; i < REMOTE_RCV_INTERFACE_NUM; i++)
//        {
//            if(remote_state[i] & 0x80)/* �ϴ������ݱ����յ��� */
//            {
//                remote_state[i] &= ~0x10;//ȡ���������Ѿ���������
//                if((remote_state[i] & 0x0F) == 0x00)
//                {
//                    remote_state[i] |= 1<<6;//����Ѿ����һ�ΰ����ļ�ֵ��Ϣ�ɼ�
//                }

//                if((remote_state[i] & 0x0F) < 14)
//                {
//                    remote_state[i]++;
//                }
//                else
//                {
//                    remote_state[i] &= ~(1<<7);//���������ʶ
//                    remote_state[i] &= 0xF0;//��ռ�����
//                }
//            }
//        }
        TIM_ClearFlag(TIM4, TIM_IT_Update);
    }

    if(TIM_GetITStatus(TIMER_IR, TIM_IT_CC2) != RESET)
    {
        TIM_ClearFlag(TIM4, TIM_IT_CC2);
        i = 1;
        if(RDATA_0)/* �����ز��� */
        {
            TIM_OC2PolarityConfig(TIMER_IR,TIM_ICPolarity_Falling);//CC1P=1 ����Ϊ�½��ز���
            TIM_SetCounter(TIMER_IR, 0);//��ն�ʱ��ֵ
            remote_start_tick[i] = timer_tick;
            remote_state[i] |= 0x10;//����������Ѿ�������
        }
        else/* �½��ز��� */
        {
            fall_edge_val[i] = TIM_GetCapture2(TIMER_IR);//��ȡCCR1Ҳ������CC1IF��־λ
            TIM_OC2PolarityConfig(TIMER_IR, TIM_ICPolarity_Rising);//CC4P=0	����Ϊ�����ز���

            if(remote_state[i] & 0x10)//���һ�θߵ�ƽ����
            {
                if(remote_state[i] & 0x80)//���յ���������
                {
                    if(fall_edge_val[i] > 300 && fall_edge_val[i] < 800)//560Ϊ��׼ֵ,560us
                    {
                        remote_recv[i] |= 0;//���յ�0
                        remote_recv[i] <<= 1;//����һλ.
                    }
                    else if(fall_edge_val[i] > 1400 && fall_edge_val[i] < 1800)//1680Ϊ��׼ֵ,1680us
                    {
                        remote_recv[i] |= 1;//���յ�1
                        remote_recv[i] <<= 1;//����һλ.
                    }
                    else if(fall_edge_val[i] >2200 && fall_edge_val[i] < 2600)//�õ�������ֵ���ӵ���Ϣ 2500Ϊ��׼ֵ2.5ms
                    {
                        remote_cnt[i]++;//������������1��
                        remote_state[i] &= 0xF0;//��ռ�ʱ��
                    }
                }
                else if(fall_edge_val[i] > 4200 && fall_edge_val[i] < 4700)//4500Ϊ��׼ֵ4.5ms
                {
                    remote_state[i] |= 1<<7;//��ǳɹ����յ���������
                    remote_cnt[i] = 0;//�����������������
                }
            }
            remote_state[i] &= ~(1<<4);
        }
    }


    if(TIM_GetITStatus(TIMER_IR, TIM_IT_CC3) != RESET)
    {
        TIM_ClearFlag(TIM4, TIM_IT_CC3);
        i = 2;
        if(RDATA_2)/* �����ز��� */
        {
            TIM_OC3PolarityConfig(TIMER_IR, TIM_ICPolarity_Falling);//CC1P=1 ����Ϊ�½��ز���
            TIM_SetCounter(TIMER_IR, 0);//��ն�ʱ��ֵ
            remote_start_tick[i] = timer_tick;
            remote_state[i] |= 0x10;//����������Ѿ�������
        }
        else/* �½��ز��� */
        {
            fall_edge_val[i] = TIM_GetCapture3(TIMER_IR);//��ȡCCR1Ҳ������CC1IF��־λ
            TIM_OC3PolarityConfig(TIMER_IR, TIM_ICPolarity_Rising);//CC4P=0	����Ϊ�����ز���

            if(remote_state[i] & 0x10)//���һ�θߵ�ƽ����
            {
                if(remote_state[i] & 0x80)//���յ���������
                {
                    if(fall_edge_val[i] > 300 && fall_edge_val[i] < 800)//560Ϊ��׼ֵ,560us
                    {
                        remote_recv[i] |= 0;//���յ�0
                        remote_recv[i] <<= 1;//����һλ.
                    }
                    else if(fall_edge_val[i] > 1400 && fall_edge_val[i] < 1800)//1680Ϊ��׼ֵ,1680us
                    {
                        remote_recv[i] |= 1;//���յ�1
                        remote_recv[i] <<= 1;//����һλ.
                    }
                    else if(fall_edge_val[i] >2200 && fall_edge_val[i] < 2600)//�õ�������ֵ���ӵ���Ϣ 2500Ϊ��׼ֵ2.5ms
                    {
                        remote_cnt[i]++;//������������1��
                        remote_state[i] &= 0xF0;//��ռ�ʱ��
                    }
                }
                else if(fall_edge_val[i] > 4200 && fall_edge_val[i] < 4700)//4500Ϊ��׼ֵ4.5ms
                {
                    remote_state[i] |= 1<<7;//��ǳɹ����յ���������
                    remote_cnt[i] = 0;//�����������������
                }
            }
            remote_state[i] &= ~(1<<4);
        }
    }


    if(TIM_GetITStatus(TIMER_IR, TIM_IT_CC4) != RESET)
    {
        TIM_ClearFlag(TIM4, TIM_IT_CC4);
        i = 0;
        if(RDATA_1)/* �����ز��� */
        {
            TIM_OC4PolarityConfig(TIMER_IR, TIM_ICPolarity_Falling);//CC1P=1 ����Ϊ�½��ز���
            TIM_SetCounter(TIMER_IR, 0);//��ն�ʱ��ֵ
            remote_start_tick[i] = timer_tick;
            remote_state[i] |= 0x10;//����������Ѿ�������
        }
        else/* �½��ز��� */
        {
            fall_edge_val[i] = TIM_GetCapture4(TIMER_IR);//��ȡCCR1Ҳ������CC1IF��־λ
            TIM_OC4PolarityConfig(TIMER_IR, TIM_ICPolarity_Rising);//CC4P=0	����Ϊ�����ز���

            if(remote_state[i] & 0x10)//���һ�θߵ�ƽ����
            {
                if(remote_state[i] & 0x80)//���յ���������
                {
                    if(fall_edge_val[i] > 300 && fall_edge_val[i] < 800)//560Ϊ��׼ֵ,560us
                    {
                        remote_recv[i] |= 0;//���յ�0
                        remote_recv[i] <<= 1;//����һλ.
                    }
                    else if(fall_edge_val[i] > 1400 && fall_edge_val[i] < 1800)//1680Ϊ��׼ֵ,1680us
                    {
                        remote_recv[i] |= 1;//���յ�1
                        remote_recv[i] <<= 1;//����һλ.
                    }
                    else if(fall_edge_val[i] >2200 && fall_edge_val[i] < 2600)//�õ�������ֵ���ӵ���Ϣ 2500Ϊ��׼ֵ2.5ms
                    {
                        remote_cnt[i]++;//������������1��
                        remote_state[i] &= 0xF0;//��ռ�ʱ��
                    }
                }
                else if(fall_edge_val[i] > 4200 && fall_edge_val[i] < 4700)//4500Ϊ��׼ֵ4.5ms
                {
                    remote_state[i] |= 1<<7;//��ǳɹ����յ���������
                    remote_cnt[i] = 0;//�����������������
                }
            }
            remote_state[i] &= ~(1<<4);
        }
    }

//    TIM_ClearFlag(TIMER_IR, TIM_IT_Update|TIM_IT_CC2);
}


uint8_t remote_scan(void)
{
    uint8_t value = 0;
    uint8_t sta = 0;
    uint8_t addr_pos_code = 0;
    uint8_t addr_neg_code = 0;
    uint8_t cmd_pos_code = 0;
    uint8_t cmd_neg_code = 0;
    uint8_t i = 0;
//    static uint32_t ir_rcv_cnt = 0;
    for(i = 0; i < REMOTE_RCV_INTERFACE_NUM; i++)
    {
        if(remote_state[i] & (1<<6))//�õ�һ��������������Ϣ��
        {
            addr_pos_code = remote_recv[i]>>24;
            addr_neg_code = (remote_recv[i]>>16) & 0xFF;
            cmd_pos_code  = (remote_recv[i]>>8) & 0xFF;
            cmd_neg_code  = (remote_recv[i]>>0) & 0xFF;

            if(addr_pos_code == (uint8_t)~addr_neg_code)
            {
                if((cmd_pos_code == (uint8_t)~cmd_neg_code) || (cmd_pos_code == (uint8_t)~(cmd_neg_code + 1)))
                {
                    if((addr_pos_code == REMOTE_LEFT) || (addr_pos_code == REMOTE_RIGHT) || (addr_pos_code == REMOTE_POWER_ON) || (addr_pos_code == REMOTE_POWER_OFF))
                    {
                        value = addr_pos_code;
//                        printf("%3d:%d-> 0x%x\r\n", ir_rcv_cnt++, i, value);
                        if(addr_pos_code == REMOTE_LEFT)
                        {
                            ir_info.ir_channel[i].ir_left_num++;
                        }
                        if(addr_pos_code == REMOTE_RIGHT)
                        {
                            ir_info.ir_channel[i].ir_right_num++;
                        }
                    }
                    else
                    {
                        /* cmd id incorrect */
                        value = 0;
                    }
                }
                else
                {
                    /* cmd error */
                    value = 0;
                }
            }
            else
            {
                /* address error */
                value = 0;
            }

            if((sta == 0)||((remote_state[i] & 0x80) == 0))//�������ݴ���/ң���Ѿ�û�а�����
            {
                remote_state[i] &= ~(1<<6);//������յ���Ч������ʶ
                remote_cnt[i] = 0;//�����������������
            }
        }
    }

    return value;
}


#define NEC_IR_COM_PERIOD   124 / SYSTICK_PERIOD    //NECЭ��һ��ͨѶ110+9+4.5 = 123.5ms
void remote_calculate(void)
{
    uint8_t i, j;

    uint8_t left[REMOTE_RCV_INTERFACE_NUM] = {0};
    uint8_t right[REMOTE_RCV_INTERFACE_NUM] = {0};
    static uint32_t start_tick = 0;
//    static uint8_t time_out = 0;

//    time_out++;//NECЭ��һ��ͨѶ110+9+4.5 = 123.5ms
//    if((scan_value != REMOTE_LEFT) && (scan_value != REMOTE_RIGHT) && (time_out <= 10))
//    if((scan_value != REMOTE_LEFT) && (scan_value != REMOTE_RIGHT) && (get_tick() - start_tick < NEC_IR_COM_PERIOD))
    if((get_tick() - start_tick < NEC_IR_COM_PERIOD))
    {
        return ;
    }

    start_tick = get_tick();
    for(j = 0; j < REMOTE_RCV_INTERFACE_NUM; j++)
    {
        for(i = VALUE_NUM - 1; i > 0; i--)
        {
            left_value[j][i] = left_value[j][i - 1];
            right_value[j][i] = right_value[j][i - 1];
        }

        left_value[j][0] = ir_info.ir_channel[j].ir_left_num;
        right_value[j][0] = ir_info.ir_channel[j].ir_right_num;

        ir_info.ir_channel[j].ir_left_num = 0;
        ir_info.ir_channel[j].ir_right_num = 0;

//        time_out = 0;
        for(i = 0; i < VALUE_NUM; i++)
        {
            left[j] += left_value[j][i];
            right[j] += right_value[j][i];
        }

        ir_signal_intensity.intensity[j].left_intensity = left[j];
        ir_signal_intensity.intensity[j].right_intensity = right[j];
    }
}

