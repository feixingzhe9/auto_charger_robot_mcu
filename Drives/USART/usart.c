/*******************************************************************************
 * @name    : ����ͨ�ŵײ�����
 * @author  : ������
 * @web     : WWW.UCORTEX.COM
 * @version : V1.1
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : STM32���ڵ����ú�ʹ��
 * ---------------------------------------------------------------------------- 
 * @copyright
 *
 * UCORTEX��Ȩ���У�Υ�߱ؾ�������Դ�������Ҳο���ּ�ڽ���ѧϰ�ͽ�ʡ����ʱ�䣬
 * ������Ϊ�ο����ļ����ݣ��������Ĳ�Ʒֱ�ӻ����ܵ��ƻ������漰���������⣬��
 * �߲��е��κ����Ρ�����ʹ�ù����з��ֵ����⣬���������WWW.UCORTEX.COM��վ��
 * �������ǣ����ǻ�ǳ���л�����������⼰ʱ�о����������ơ����̵İ汾���£�����
 * ���ر�֪ͨ���������е�WWW.UCORTEX.COM�������°汾��лл��
 * ��������������UCORTEX������һ�����͵�Ȩ����
 * ----------------------------------------------------------------------------
 * @description
 * 
 * ע��ʹ�ñ����������ļ�ʱ����Ҫ��stm32f10x_it.c��д��Ӧ�Ĵ��ڽ����жϺ���
 * void USART1_IRQHandler(void)��
 *
 * COM1:
 * USART1_TX -> PA9
 * USART1_RX -> PA10
 * 
 * COM2:
 * USART2_TX -> PA2
 * USART2_RX -> PA3
 * 
 * COM3:
 * USART3_TX -> PB10
 * USART3_RX -> PB11
 *
 * LED -> PB12
 * KEY -> PA0
 *
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * ����ʱ�䣺2014-04-03    �����ˣ�������
 * �汾��¼��V1.0
 * �������ݣ��½�
 * ----------------------------------------------------------------------------
 * ����ʱ�䣺2014-05-01    �����ˣ�������
 * �汾��¼��V1.1
 * �������ݣ�������DMA���䷽ʽ��֧��
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
#include "usart.h"
#if COM_DMA_TRANSFER
#include "dma.h"
#endif

#if COM_RX_LEN
/* COM_RX_STA: ���ڽ���״̬��
 * bit15��	������ɱ�־
 * bit14��	���յ�0x0d
 * bit13~0��	���յ�����Ч�ֽ���Ŀ
*/
uint16_t COM_RX_STA = 0;

/* ���ڽ��ջ��� */
uint8_t COM_RX_BUF[COM_RX_LEN];
#endif

/******************************* @printf  *************************************/

#ifdef PRINTF_SUPPORT //���´���,֧��printf����,������Ҫѡ��use MicroLIB
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
int _sys_exit(int x) 
{ 
	return x;
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(PRINTF_SUPPORT, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(PRINTF_SUPPORT, USART_FLAG_TC) == RESET)
  {}    
	return ch;
}
#endif
/********************************* @end ***************************************/

/**
  * @brief ���崮�����PORT��CLK��
  */
USART_TypeDef* COM_USART[COMn] = {UCORTEX_COM1, UCORTEX_COM2, UCORTEX_COM3}; 

GPIO_TypeDef* COM_TX_PORT[COMn] = {UCORTEX_COM1_TX_GPIO_PORT, UCORTEX_COM2_TX_GPIO_PORT, UCORTEX_COM3_TX_GPIO_PORT};
 
GPIO_TypeDef* COM_RX_PORT[COMn] = {UCORTEX_COM1_RX_GPIO_PORT, UCORTEX_COM2_RX_GPIO_PORT, UCORTEX_COM3_RX_GPIO_PORT};
 
const uint32_t COM_USART_CLK[COMn] = {UCORTEX_COM1_CLK, UCORTEX_COM2_CLK, UCORTEX_COM3_CLK};

const uint32_t COM_TX_PORT_CLK[COMn] = {UCORTEX_COM1_TX_GPIO_CLK, UCORTEX_COM2_TX_GPIO_CLK, UCORTEX_COM3_TX_GPIO_CLK};
 
const uint32_t COM_RX_PORT_CLK[COMn] = {UCORTEX_COM1_RX_GPIO_CLK, UCORTEX_COM2_RX_GPIO_CLK, UCORTEX_COM3_RX_GPIO_CLK};

const uint16_t COM_TX_PIN[COMn] = {UCORTEX_COM1_TX_PIN, UCORTEX_COM2_TX_PIN, UCORTEX_COM3_TX_PIN};

const uint16_t COM_RX_PIN[COMn] = {UCORTEX_COM1_RX_PIN, UCORTEX_COM2_RX_PIN, UCORTEX_COM3_RX_PIN};

const uint8_t COM_IRQn[COMn] = {UCORTEX_COM1_IRQn, UCORTEX_COM2_IRQn, UCORTEX_COM3_IRQn};

#if COM_DMA_TRANSFER
DMA_TypeDef* COM_DMA[COMn] = {UCORTEX_COM1_DMA, UCORTEX_COM2_DMA, UCORTEX_COM3_DMA}; 

DMA_Channel_TypeDef* COM_DMA_CH[COMn][2] = {{UCORTEX_COM1_RX_DMA_CH, UCORTEX_COM1_TX_DMA_CH},
																						{UCORTEX_COM2_RX_DMA_CH, UCORTEX_COM2_TX_DMA_CH},
																						{UCORTEX_COM3_RX_DMA_CH, UCORTEX_COM3_TX_DMA_CH}};
																						
const uint32_t COM_DR_BASE[COMn] = { UCORTEX_COM1_DR_Base, UCORTEX_COM2_DR_Base, UCORTEX_COM3_DR_Base};

#endif


/**
  * @brief  ��ʼ������GPIO
  * @param  COM: ָ��Ҫ��ʼ����COM PORT   
  *     @arg COM1 ����1
  *     @arg COM2 ����2
  *     @arg COM3 ����3
  * @retval None
  * @note None
  */
void COM_GPIOInit(COM_TypeDef COM)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* ����GPIOʱ�Ӻ͸��ù���ʱ��RCC_APB2Periph_AFIO */
  RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);

  /* ���� USART Tx ����������� */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

  /* ���� USART Rx �������� */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);
}

/**
  * @brief  ���ô���
  * @param  COM: ָ��Ҫ��ʼ����COM PORT   
  *     @arg COM1 ����1
  *     @arg COM2 ����2
  *     @arg COM3 ����3
  * @param  Baudrate ���ڲ�����
  * @retval None
  * @note None
  */
void COM_Init(COM_TypeDef COM, uint32_t BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
  COM_GPIOInit(COM);//��ʼ������GPIO

  /* ��������ʱ�ӣ�ע�⴮��1������APB1���������£��������ڹ���APB2����������*/
  if (COM == COM1)
    RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE); //��������ʱ��
  else
    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);	//��������ʱ��
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�������ȼ���2
	/* USARTx NVIC������Ϣ */
	NVIC_InitStructure.NVIC_IRQChannel = COM_IRQn[COM];
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;				//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;						//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//���������������Ϣ��ʼ��NVIC�Ĵ���
	
	
  /*    ����������Ϣ
	      -------------
        - �����ʸ��ݲ���ָ��
        - ���ݳ���8bit
        - 1��ֹͣλ
        - ����żУ��λ
        - ��Ӳ������������(RTS �� CTS �ź�)
        - �������ڷ��ͺͽ���
  */
  USART_InitStructure.USART_BaudRate = BaudRate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx;
  USART_Init(COM_USART[COM], &USART_InitStructure);			//���ݴ���������Ϣ��ʼ������ 

	USART_ITConfig(COM_USART[COM], USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(COM_USART[COM], ENABLE);										//��������
	
}

#if COM_DMA_TRANSFER
/**
  * @brief  ��ѯ��ʽ(Polling)����DMA��ʼ��
  * @param  COM: Ҫ���õĴ��� 
  *     @arg COM1 ����1(USART1)
  *     @arg COM2 ����2(USART2)
  *     @arg COM3 ����3(USART3)
  * @param  MemoryBaseAddr : DMA�����ַ��һ�������Ƕ���Ļ��������ַ��
	* @param  BufferSize : �����С
	* @param  DMA_DIR : DMA���䷽��
  *     @arg DMA_DIR_PeripheralDST : ���ڷ���DMA���� Buffer->USART ��DMA����
  *     @arg DMA_DIR_PeripheralSRC : ���ڽ���DMA���� USART->Buffer ��DMA����
  * @retval None
  * @note �ڱ������У�������Ӧ�Ĵ���DMA�ӿڣ�����δ����DMA���䣬����Ҫ����DMA����ʱ��
	*				�����DMA_Polling_Enable(DMA_Channel_TypeDef* DMAy_Channelx, uint16_t DataNumber)
  */
void COM_DMA_Polling_Init(COM_TypeDef COM, uint32_t MemoryBaseAddr, uint32_t BufferSize, uint32_t DMA_DIR)
{
	//���ڶ�Ӧ��DMA
	DMA_TypeDef* DMAy = COM_DMA[COM];
	//��Ӧ��DMAͨ����ע�⴮�ڽ��պͷ��Ͷ�Ӧ��ͬ��DMAͨ����
	DMA_Channel_TypeDef* DMAy_Channelx = COM_DMA_CH[COM][DMA_DIR>>4];
	//�������ݼĴ�����ַ
	uint32_t PeripheralBaseAddr = COM_DR_BASE[COM];
	//����λ��: 0 -> 8bit, 1 -> 16bit, 2-> 32bit
	uint32_t DataType = 0; //8-bitλ��
	
	//����һ�����ö�Ӧ��DMAͨ��
	DMA_Polling_Init(DMAy, DMAy_Channelx, PeripheralBaseAddr, MemoryBaseAddr, BufferSize, DataType, DMA_DIR, DMA_Mode_Normal);
	
	//��������򿪴���DMA���ͻ���սӿ�
	if(DMA_DIR==DMA_DIR_PeripheralDST)//�򿪴���DMA����
		USART_DMACmd(COM_USART[COM], USART_DMAReq_Tx, ENABLE);
	
	if(DMA_DIR==DMA_DIR_PeripheralSRC)//�򿪴���DMA����
		USART_DMACmd(COM_USART[COM], USART_DMAReq_Rx, ENABLE);
}

#endif

	
/**********************************************************
*	���ܣ���������
*	������buf ���ͻ������׵�ַ
*		  len �����͵��ֽ���
**********************************************************/
void RS232_Send_Data(u8 *buf,u8 len)
{
	u8 t;
	
  for(t=0;t<len;t++)		//ѭ����������
	{		   
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	//�������  
		USART_SendData(USART1,buf[t]);
	}	 
 
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
//	COM1_RX_CNT=0;	  
}


//***********************���ڷ�DMA����**************************//
void COM_DMA_send_init(uint8_t* send_buffer,uint32_t BaudRate)
{
	DMA_InitTypeDef 		DMA_InitStructure;
	USART_InitTypeDef 	USART_InitStructure;
	NVIC_InitTypeDef 		NVIC_InitStructure;
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE);     		//�򿪴��ڶ�Ӧ����ʱ��   
    //??DMA??  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  
    //DMA??????  
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  
	
		/*  ����������Ϣ
			-------------
			- �����ʸ��ݲ���ָ��
			- ���ݳ���8bit
			- 1��ֹͣλ
			- ����żУ��λ
			- ��Ӳ������������(RTS �� CTS �ź�)
			- �������ڷ��ͺͽ���
	*/
	//USART_InitStructure.USART_BaudRate = DEFAULT_BAUD;    
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;    
	USART_InitStructure.USART_StopBits = USART_StopBits_1;    
	USART_InitStructure.USART_Parity = USART_Parity_No;    
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;    
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;      
	USART_InitStructure.USART_BaudRate = BaudRate;   
	
	USART_Init(USART1,&USART_InitStructure);    				//��ʼ������
	
    DMA_DeInit(DMA1_Channel4);  //DMA1ͨ��4����
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);  	//�����ַ
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)send_buffer;  		//�ڴ��ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  							//DMA���䷽����
    DMA_InitStructure.DMA_BufferSize = 100;  													//����DMA�ڴ���ʱ�������ĳ���
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//����DMA���������ģʽ��������  
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  					//����DMA���ڴ����ģʽ������
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //�������ݵ�λ����byte
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;  		 //�ڴ����ݵ�λ����byte
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  										//DMA����ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;  							//DMA���ȼ���
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  										//��ֹDMA�е�����memory�������
    DMA_Init(DMA1_Channel4,&DMA_InitStructure);  
    DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);  
 	
		//����DMA��ʽ����
    USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);      
    //ʹ��DMA1ͨ��4
//    DMA_Cmd(DMA1_Channel4, ENABLE);  
}

/***********DMA���͵��ú���*************/
void COM1_DMA_send_data(uint8_t length)
{
	DMA_Cmd(DMA1_Channel4, DISABLE );	//�ر�DMAͨ��  
 	DMA_SetCurrDataCounter(DMA1_Channel4, length);//���ñ���DMA�����������
 	DMA_Cmd(DMA1_Channel4, ENABLE);		//��DMAͨ��
}

uint8_t Uart_Tx[100];

/***********��ʼ��COM_DMA*************/
void COM1_DMA_init(uint32_t BaudRate)
{	
	COM_GPIOInit(COM1);
	COM_DMA_send_init(Uart_Tx, 115200);
//	COM_DMA_receive_init(Uart_Rx, 115200);	
}

void reply_usart1(void)
{
	uint8_t head = 0x5a;
	uint8_t end  = 0xa5;
	uint8_t check = 0;
	uint8_t i = 0;
	
	Uart_Tx[0] = head;
	Uart_Tx[1] = power_ctl.power_state;
	Uart_Tx[2] = power_ctl.err_type;
	Uart_Tx[3] = power_ctl.v>>8;
	Uart_Tx[4] = power_ctl.v;
	Uart_Tx[5] = power_ctl.w>>8;
	Uart_Tx[6] = power_ctl.w;
	Uart_Tx[7] = (uint8_t)(range_value/10);
	for(i = 0;i<8;i++)
	{
		check += Uart_Tx[i];
	}
	Uart_Tx[8] = check;
	Uart_Tx[9] = end;
	
	COM1_DMA_send_data(10);
}

void send_straight(void)
{
	uint8_t v;
	uint8_t straight_buf[12] = {0};
	v = (navigation_mode == NAVIGATION_1)?V_1:(navigation_mode == NAVIGATION_2)?V_2:V_3;
	
	straight_buf[0] = 0x55;
	straight_buf[1] = 0x00;
	straight_buf[2] = v;
	straight_buf[3] = 0x00;
	straight_buf[4] = 0x00;
	straight_buf[5] = 0x00;
	straight_buf[6] = 0x00;
	straight_buf[7] = 0x00;
	straight_buf[8] = 0x00;
	straight_buf[9] = 0x00;
	straight_buf[10] = (uint8_t)(v+0x55);
	straight_buf[11] = 0xAA;
	
	RS232_Send_Data(straight_buf,12);
}

void send_left(void)
{
	uint8_t w;
	uint8_t left_buf[12] = {0};
	w = (navigation_mode == NAVIGATION_1)?W_1:(navigation_mode == NAVIGATION_2)?W_2:W_3;
	
	left_buf[0] = 0x55;
	left_buf[1] = 0x00;
	left_buf[2] = 0x00;
	left_buf[3] = 0x00;
	left_buf[4] = w;
	left_buf[5] = 0x00;
	left_buf[6] = 0x00;
	left_buf[7] = 0x00;
	left_buf[8] = 0x00;
	left_buf[9] = 0x00;
	left_buf[10] = (uint8_t)(w+0x55);
	left_buf[11] = 0xAA;
	
	RS232_Send_Data(left_buf,12);
}

void send_right(void)
{
	uint8_t w;
	uint8_t right_buf[12] = {0};
	w = (navigation_mode == NAVIGATION_1)?W_1:(navigation_mode == NAVIGATION_2)?W_2:W_3;
	
	right_buf[0] = 0x55;
	right_buf[1] = 0x00;
	right_buf[2] = 0x00;
	right_buf[3] = 0xFF;
	right_buf[4] = (256-w);
	right_buf[5] = 0x00;
	right_buf[6] = 0x00;
	right_buf[7] = 0x00;
	right_buf[8] = 0x00;
	right_buf[9] = 0x00;
	right_buf[10] = (uint8_t)((256-w)+0x55+0xFF);
	right_buf[11] = 0xAA;
	
	RS232_Send_Data(right_buf,12);
}


/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/