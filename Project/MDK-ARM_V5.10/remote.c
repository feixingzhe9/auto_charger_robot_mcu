#include "remote.h"
#include "delay.h"
#include "usart.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"


//����ң�س�ʼ��
//����IO�Լ���ʱ��4�����벶��
void Remote_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); //ʹ��PORTBʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);	//TIM4 ʱ��ʹ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_9;				 //PB9 ����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		//��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_7);	//��ʼ��GPIOB.9

    TIM_TimeBaseStructure.TIM_Period = 10000; //�趨�������Զ���װֵ ���10ms���
    TIM_TimeBaseStructure.TIM_Prescaler =(72-1); 	//Ԥ��Ƶ��,1M�ļ���Ƶ��,1us��1.
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ

    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;  // ѡ������� IC4ӳ�䵽TI4��
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ
    TIM_ICInitStructure.TIM_ICFilter = 0x03;//IC4F=0011 ���������˲��� 8����ʱ��ʱ�������˲�
    TIM_ICInit(TIM4, &TIM_ICInitStructure);//��ʼ����ʱ�����벶��ͨ��

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;  // ѡ������� IC4ӳ�䵽TI4��
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ
    TIM_ICInitStructure.TIM_ICFilter = 0x03;//IC4F=0011 ���������˲��� 8����ʱ��ʱ�������˲�
    TIM_ICInit(TIM4, &TIM_ICInitStructure);//��ʼ����ʱ�����벶��ͨ��

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;  // ѡ������� IC4ӳ�䵽TI4��
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ
    TIM_ICInitStructure.TIM_ICFilter = 0x03;//IC4F=0011 ���������˲��� 8����ʱ��ʱ�������˲�
    TIM_ICInit(TIM4, &TIM_ICInitStructure);//��ʼ����ʱ�����벶��ͨ��

    TIM_Cmd(TIM4,ENABLE ); 	//ʹ�ܶ�ʱ��4

    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
    NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

    TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE);//��������ж� ,����CC4IE�����ж�
}

//ң��������״̬
//[7]:�յ����������־
//[6]:�õ���һ��������������Ϣ
//[5]:����
//[4]:����������Ƿ��Ѿ�������
//[3:0]:�����ʱ��
u8 	RmtSta[REMOTE_RCV_INTERFACE_NUM]={0};
u16 Dval[REMOTE_RCV_INTERFACE_NUM];		//�½���ʱ��������ֵ
u32 RmtRec[REMOTE_RCV_INTERFACE_NUM]={0};	//������յ�������
u8  RmtCnt[REMOTE_RCV_INTERFACE_NUM]={0};	//�������µĴ���
//��ʱ��2�жϷ������
void TIM4_IRQHandler(void)
{
    uint8_t i = 0;
    if(TIM_GetITStatus(TIM4,TIM_IT_Update)!=RESET)
    {
        for(i = 0; i < REMOTE_RCV_INTERFACE_NUM; i++)
        {
            if(RmtSta[i]&0x80)//�ϴ������ݱ����յ���
            {
                RmtSta[i] &= ~0X10;						//ȡ���������Ѿ���������
                if((RmtSta[i] & 0X0F) == 0X00)
                {
                    RmtSta[i] |= 1 << 6;//����Ѿ����һ�ΰ����ļ�ֵ��Ϣ�ɼ�
                }
                if((RmtSta[i]&0X0F)<14)
                {
                    RmtSta[i]++;
                }
                else
                {
                    RmtSta[i]&=~(1<<7);//���������ʶ
                    RmtSta[i]&=0XF0;	//��ռ�����
                }
            }
        }
        TIM_ClearFlag(TIM4, TIM_IT_Update);
    }

    if(TIM_GetITStatus(TIM4,TIM_IT_CC2)!=RESET)
    {
        TIM_ClearFlag(TIM4, TIM_IT_CC2);
        i = 0;
        if(RDATA_0)//�����ز���
        {
            TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
            TIM_SetCounter(TIM4,0);	   	//��ն�ʱ��ֵ
            RmtSta[i]|=0X10;					//����������Ѿ�������
        }
        else //�½��ز���
        {
            Dval[i]=TIM_GetCapture2(TIM4);//��ȡCCR1Ҳ������CC1IF��־λ
            TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC4P=0	����Ϊ�����ز���

            if(RmtSta[i]&0X10)					//���һ�θߵ�ƽ����
            {
                if(RmtSta[i]&0X80)//���յ���������
                {

                    if(Dval[i]>300&&Dval[i]<800)			//560Ϊ��׼ֵ,560us
                    {
                        RmtRec[i]|=0;	//���յ�0
                        RmtRec[i]<<=1;	//����һλ.
                    }
                    else if(Dval[i]>1400&&Dval[i]<1800)	//1680Ϊ��׼ֵ,1680us
                    {
                        RmtRec[i]|=1;	//���յ�1
                        RmtRec[i]<<=1;	//����һλ.
                    }
                    else if(Dval[i]>2200&&Dval[i]<2600)	//�õ�������ֵ���ӵ���Ϣ 2500Ϊ��׼ֵ2.5ms
                    {
                        RmtCnt[i]++; 		//������������1��
                        RmtSta[i]&=0XF0;	//��ռ�ʱ��
                    }
                }
                else if(Dval[i]>4200&&Dval[i]<4700)		//4500Ϊ��׼ֵ4.5ms
                {
                    RmtSta[i]|=1<<7;	//��ǳɹ����յ���������
                    RmtCnt[i]=0;		//�����������������
                }
            }
            RmtSta[i]&=~(1<<4);
        }
    }

    if(TIM_GetITStatus(TIM4,TIM_IT_CC4)!=RESET)
    {
        TIM_ClearFlag(TIM4, TIM_IT_CC4);
        i = 1;
        if(RDATA_1)//�����ز���
        {
            TIM_OC4PolarityConfig(TIM4, TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
            TIM_SetCounter(TIM4,0);	   	//��ն�ʱ��ֵ
            RmtSta[i]|=0X10;					//����������Ѿ�������
        }
        else //�½��ز���
        {
            Dval[i]=TIM_GetCapture4(TIM4);//��ȡCCR1Ҳ������CC1IF��־λ
            TIM_OC4PolarityConfig(TIM4, TIM_ICPolarity_Rising); //CC4P=0	����Ϊ�����ز���

            if(RmtSta[i]&0X10)					//���һ�θߵ�ƽ����
            {
                if(RmtSta[i]&0X80)//���յ���������
                {

                    if(Dval[i]>300&&Dval[i]<800)			//560Ϊ��׼ֵ,560us
                    {
                        RmtRec[i]|=0;	//���յ�0
                        RmtRec[i]<<=1;	//����һλ.
                    }
                    else if(Dval[i]>1400&&Dval[i]<1800)	//1680Ϊ��׼ֵ,1680us
                    {
                        RmtRec[i]|=1;	//���յ�1
                        RmtRec[i]<<=1;	//����һλ.
                    }else if(Dval[i]>2200&&Dval[i]<2600)	//�õ�������ֵ���ӵ���Ϣ 2500Ϊ��׼ֵ2.5ms
                    {
                        RmtCnt[i]++; 		//������������1��
                        RmtSta[i]&=0XF0;	//��ռ�ʱ��
                    }
                }
                else if(Dval[i]>4200&&Dval[i]<4700)		//4500Ϊ��׼ֵ4.5ms
                {
                    RmtSta[i]|=1<<7;	//��ǳɹ����յ���������
                    RmtCnt[i]=0;		//�����������������
                }
            }
            RmtSta[i]&=~(1<<4);
        }
    }

    if(TIM_GetITStatus(TIM4,TIM_IT_CC3)!=RESET)
    {
        TIM_ClearFlag(TIM4, TIM_IT_CC3);
        //        printf("ch 4 \r\n");
        i = 2;
        if(RDATA_2)//�����ز���
        {
            TIM_OC3PolarityConfig(TIM4, TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
            TIM_SetCounter(TIM4,0);	   	//��ն�ʱ��ֵ
            RmtSta[i]|=0X10;					//����������Ѿ�������
        }
        else //�½��ز���
        {
            Dval[i]=TIM_GetCapture3(TIM4);//��ȡCCR1Ҳ������CC1IF��־λ
            TIM_OC3PolarityConfig(TIM4, TIM_ICPolarity_Rising); //CC4P=0	����Ϊ�����ز���

            if(RmtSta[i]&0X10)					//���һ�θߵ�ƽ����
            {
                if(RmtSta[i]&0X80)//���յ���������
                {

                    if(Dval[i]>300&&Dval[i]<800)			//560Ϊ��׼ֵ,560us
                    {
                        RmtRec[i]|=0;	//���յ�0
                        RmtRec[i]<<=1;	//����һλ.
                    }else if(Dval[i]>1400&&Dval[i]<1800)	//1680Ϊ��׼ֵ,1680us
                    {
                        RmtRec[i]|=1;	//���յ�1
                        RmtRec[i]<<=1;	//����һλ.
                    }
                    else if(Dval[i]>2200&&Dval[i]<2600)	//�õ�������ֵ���ӵ���Ϣ 2500Ϊ��׼ֵ2.5ms
                    {
                        RmtCnt[i]++; 		//������������1��
                        RmtSta[i]&=0XF0;	//��ռ�ʱ��
                    }
                }
                else if(Dval[i]>4200&&Dval[i]<4700)		//4500Ϊ��׼ֵ4.5ms
                {
                    RmtSta[i]|=1<<7;	//��ǳɹ����յ���������
                    RmtCnt[i]=0;		//�����������������
                }
            }
            RmtSta[i]&=~(1<<4);
        }
    }

    //    TIM_ClearFlag(TIM4, TIM_IT_Update | TIM_IT_CC2 |TIM_IT_CC4);
}

//����������
//����ֵ:
//	 0,û���κΰ�������
//����,���µİ�����ֵ.
u8 Remote_Scan(void)
{
    u8 value=0;
    u8 sta=0;
    u8 t1=0;
    u8 t2=0;
    u8 c1=0;
    u8 c2=0;
    uint8_t i = 0;
    static uint32_t cnt = 0;
    for(i = 0; i < REMOTE_RCV_INTERFACE_NUM; i++)
    {
        if(RmtSta[i]&(1<<6))//�õ�һ��������������Ϣ��
        {
            t1=RmtRec[i]>>24;			//�õ���ַ��
            t2=(RmtRec[i]>>16)&0xff;	//�õ���ַ����
            c1=(RmtRec[i]>>8)&0xff;	//�õ�������
            c2=(RmtRec[i]>>0)&0xff;	//�õ������
            if((t1==(u8)~t2)&&((c1==(u8)~c2)||(c1==(u8)~(c2+1)))&&((t1==REMOTE_ID1)||(t1==REMOTE_ID2)||(t1==REMOTE_ID_POWER_ON)||(t1==REMOTE_ID_POWER_OFF)))//����ң��ʶ����(ID)����ַ
            {
                //				printf("t1=%x,t2=%x,c1=%x,c2=%x\r\n",t1,t2,c1,c2);
                //	        t1=RmtRec>>8;
                //	        t2=RmtRec;
                //	        if(t1==(u8)~t2)sta=t1;//��ֵ��ȷ
                value = t1;
                printf("%3d:%d->0x%x \r\n",cnt++, i, value);
                if(t1==REMOTE_ID1)
                {
                    ir_info.ir_channel[i].ir_left_num++;
                }
                if(t1==REMOTE_ID2)
                {
                    ir_info.ir_channel[i].ir_right_num++;
                }
            }
            else
            {
                value = 0;
            }
            if((sta==0)||((RmtSta[i]&0X80)==0))//�������ݴ���/ң���Ѿ�û�а�����
            {
                RmtSta[i]&=~(1<<6);//������յ���Ч������ʶ
                RmtCnt[i]=0;		//�����������������
            }
        }
    }

    return value;
}

u8 left_value[REMOTE_RCV_INTERFACE_NUM][VALUE_NUM] = {{0}};
u8 right_value[REMOTE_RCV_INTERFACE_NUM][VALUE_NUM] = {{0}};


void remote_calculate(uint8_t scan_value)
{
    int8_t i;
    uint8_t j = 0;
    static u8 time_out = 0;
    u8 left[REMOTE_RCV_INTERFACE_NUM] = {0};
    u8 right[REMOTE_RCV_INTERFACE_NUM] = {0};

    if(power_ctl.control_flag == CONTROL_STOP)
    {
        return;
    }
    else if(power_ctl.control_flag == CONTORL_GOTO_INIT)
    {
        if(range_value > INIT_CTL_RANGE)
        {
            return;
        }
        else
        {
            return;
        }
    }
    else
    {
        time_out++;				//NECЭ��һ��ͨѶ110+9+4.5 = 123.5ms
        if((scan_value != REMOTE_ID1)&&(scan_value != REMOTE_ID2)&&(time_out <= 10))
        {
            return ;
        }
#if 1
        for(j = 0; j < REMOTE_RCV_INTERFACE_NUM; j++)
        {
            for(i = VALUE_NUM - 1; i > 0; i--)
            {
                left_value[j][i] = left_value[j][i - 1];
                right_value[j][i] = right_value[j][i - 1];
            }
//            if(ir_info.ir_channel[j].ir_left_num > 0)
            {
                left_value[j][0] = ir_info.ir_channel[j].ir_left_num;
                right_value[j][0] = ir_info.ir_channel[j].ir_right_num;
                if(ir_info.ir_channel[j].ir_left_num > 0)
                {
                    time_out = 0;
                }
                if(ir_info.ir_channel[j].ir_right_num > 0)
                {
                    time_out = 0;
                }
                ir_info.ir_channel[j].ir_left_num = 0;
                ir_info.ir_channel[j].ir_right_num = 0;
            }

            if(time_out > 10)
            {
                time_out = 0;
            }

            for(i = 0; i < VALUE_NUM; i++)
            {
                left[j] += left_value[j][i];
                right[j] += right_value[j][i];
            }

            ir_signal_intensity.intensity[j].left_intensity = left[j];
            ir_signal_intensity.intensity[j].right_intensity = right[j];
        }
#endif
//        for(i = 0; i < REMOTE_RCV_INTERFACE_NUM; i++)
//        {
//            ir_signal_intensity.intensity[i].left_intensity = ir_info.ir_channel[i].ir_left_num;
//            if(ir_info.ir_channel[i].ir_left_num != 0)
//            {
//                ir_info.ir_channel[i].ir_left_num = 0;
//                time_out = 0;
//            }
//
//            ir_signal_intensity.intensity[i].right_intensity = ir_info.ir_channel[i].ir_right_num;
//            if(ir_info.ir_channel[i].ir_right_num != 0)
//            {
//                ir_info.ir_channel[i].ir_right_num = 0;
//                time_out = 0;
//            }
//            if(time_out > 10)
//            {
//                time_out = 0;
//            }
//        }
    }
}

