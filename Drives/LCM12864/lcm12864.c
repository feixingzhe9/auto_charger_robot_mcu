/*******************************************************************************
 * @name    : 12864点阵模块显示底层驱动
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.0
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 通过并口或串行口驱动12864
 * ---------------------------------------------------------------------------- 
 * @copyright
 *
 * UCORTEX版权所有，违者必究！例程源码仅供大家参考，旨在交流学习和节省开发时间，
 * 对于因为参考本文件内容，导致您的产品直接或间接受到破坏，或涉及到法律问题，作
 * 者不承担任何责任。对于使用过程中发现的问题，如果您能在WWW.UCORTEX.COM网站反
 * 馈给我们，我们会非常感谢，并将对问题及时研究并作出改善。例程的版本更新，将不
 * 做特别通知，请您自行到WWW.UCORTEX.COM下载最新版本，谢谢。
 * 对于以上声明，UCORTEX保留进一步解释的权利！
 * ----------------------------------------------------------------------------
 * @description
 *
 * 并口模式:
 * LCM_RS  -> PB9  : 数据/命令选择
 * LCM_RW  -> PB10 : 读/写选择
 * LCM_E   -> PB11 : 使能信号
 * LCM_D0  -> PB0  : 数据
 * LCM_D1  -> PB1  : 数据
 * LCM_D2  -> PB2  : 数据
 * LCM_D3  -> PB3  : 数据
 * LCM_D4  -> PB4  : 数据
 * LCM_D5  -> PB5  : 数据
 * LCM_D6  -> PB6  : 数据
 * LCM_D7  -> PB7  : 数据
 * LCM_BK  -> PC13 : 背光控制，高电平亮
 * LCM_RST -> PB12 : 复位
 *
 * 串行口模式:
 * LCM_CS  -> PB9  : 选择
 * LCM_SIF -> PB10 : 数据
 * LCM_CLK -> PB11 : 时钟
 * LCM_BK  -> PC13 : 背光控制，高电平亮
 * LCM_RST -> PB12 : 复位
 *
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-03    更改人：布谷鸟
 * 版本记录：V1.0
 * 更改内容：新建
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
#include "stdlib.h"
#include "delay.h"
#include "lcm12864.h"

const uint8_t AC_TABLE[]={
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,      //第一行汉字位置
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,      //第二行汉字位置
0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,      //第三行汉字位置
0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f      //第四行汉字位置
};

/**
  * @brief  12864 LCM模块初始化
  * @param  None
  * @retval None
	* @note 
  */
void LCM12864_Init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
	
 	/* 使能GPIOB,GPIOC，AFIO时钟 */
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);

  /* 配置STM32为SWD调试模式，注意要先开启AFIO时钟 */
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);	//SWD模式
	
	/* PC13配置为推挽输出，用于控制LCM背光控制，注意PC13不支持5V耐压，速度只能配置到2MHz（详情参考规格书） */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	//速度2MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);	//初始化
	GPIO_SetBits(GPIOC, GPIO_Pin_13);//打开背光
#if LCM_MODE	
	/* 配置PB0..7,PB9,PB10,PB11推挽输出 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
	                              GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|
	                              GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);						//初始化
  LCM_E=0;
	
#else
	/* 配置PB0..7,PB9,PB10,PB11推挽输出 */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	LCM_CLK=0;
#endif

	LCM_RST=0;	//复位12864
  delay_ms(50);
	LCM_RST=1;
	delay_ms(30);

	LCM_WriteCmd(0x30);//功能设置，一次送8位数据，基本指令集
	LCM_WriteCmd(0x0C);//0000,1100  整体显示，游标off，游标位置off
	LCM_WriteCmd(0x01);//0000,0001 清DDRAM
	LCM_WriteCmd(0x02);//0000,0010 DDRAM地址归位
	LCM_WriteCmd(0x80);//1000,0000 设定DDRAM 7位地址000，0000到地址计数器AC//     
	LCM_WriteCmd(0x04);//点设定，显示字符/光标从左到右移位，DDRAM地址加 一//   
	LCM_WriteCmd(0x0C);//显示设定，开显示，显示光标，当前显示位反白闪动
}


#if LCM_MODE
/**
  * @brief  12864 LCM些数据
  * @param  data : 写入的数据
  * @retval None
	* @note 
  */
void LCM_WriteData(uint8_t data)
{
	SET_DATA_OUT();
	DATAOUT(data);
	LCM_RS = 1;
	LCM_RW = 0;
	LCM_E = 1;
	delay_us(5);
	LCM_E = 0;
}

/**
  * @brief  12864 LCM模块写指令
  * @param  cmd : 写入的指令
  * @retval None
	* @note 
  */
void LCM_WriteCmd(uint8_t cmd) 
{
	SET_DATA_OUT();
	DATAOUT(cmd);
	LCM_RS = 0;
	LCM_RW = 0;
	LCM_E = 1;
	delay_us(5);
	LCM_E = 0;
}

/**
  * @brief  12864 LCM读数据
  * @param  None
  * @retval 读到的数据
	* @note 
  */
uint8_t LCM_ReadData(void)
{
	uint8_t data;
	
	SET_DATA_IN();
	LCM_RS = 1;
	LCM_RW = 1;
	LCM_E = 1;
	delay_us(10);
	data=DATAIN;
	LCM_E = 0;
	return data;
}

#else
/**
  * @brief  通过串行模式向12864发送1Byte数据
  * @param  data : 写入的数据
  * @retval None
	* @note 
  */
void LCM_SendByte(uint8_t data)
{
	uint8_t i=0;
	LCM_CS=1;
	for(i=0;i<8;i++)
	{
		if(data&0x80)LCM_SIF=1;//高位先传
		else LCM_SIF=0;
		data<<=1;//左移1位
		LCM_CLK=1;
		LCM_CLK=0;
	}
	LCM_CS=0;
}

/**
  * @brief  通过串行模式向12864发送1Byte数据
  * @param  data : 写入的数据
  * @retval None
	* @note 
  */
 void LCM_SPIWrite(uint8_t data,uint8_t mode)
 {
   LCM_SendByte(0XF8+(mode<<1));
   LCM_SendByte(data&0XF0);
   LCM_SendByte((data<<4)&0XF0);
 }

 /**
  * @brief  12864 LCM些数据
  * @param  data : 写入的数据
  * @retval None
	* @note 
  */
void LCM_WriteData(uint8_t data)
{
	LCM_SPIWrite(data,1);
}

/**
  * @brief  12864 LCM模块写指令
  * @param  cmd : 写入的指令
  * @retval None
	* @note 
  */
void LCM_WriteCmd(uint8_t cmd) 
{
  LCM_SPIWrite(cmd,0);
	delay_us(100);
}

#endif

/**
  * @brief  字符初始位置设定
  * @param  row - 第几行 (0~3)
  * @param  col - 第几列 (0~7)
  * @retval None
	* @note 
  */
void LCM_SetCursor(uint8_t row, uint8_t col)
{ 
	if(row>3 || col>7)return;
	
	LCM_WriteCmd(0x30);	 //8BitMCU,基本指令集合
	LCM_WriteCmd(AC_TABLE[8*row+col]);	//起始位置
}

/**
  * @brief  在指定位置显示一个字符
  * @param  row - 第几行 (0~3)
  * @param  col - 第几裂 (0~7)
  * @retval None
	* @note 
  */
void LCM_ShowChar(uint8_t row, uint8_t col, uint8_t ch)
{
	if(row>3 || col>7)return;
  LCM_SetCursor(row, col);
  LCM_WriteData(ch);
}

/**
  * @brief  从指定位置开始显示字符串
  * @param  row - 第几行 (0~3)
  * @param  col - 第几列 (0~7)
  * @param *str - 字符串指针
  * @retval None
	* @note 
  */
void LCM_ShowString(uint8_t row, uint8_t col, uint8_t *str) 
{ 
	LCM_WriteCmd(0x30);	//8BitMCU,基本指令集合
	LCM_WriteCmd(AC_TABLE[8*row+col]);      //起始位置
	while(*str != '\0')	//判断字符串是否显示完毕
	{
		if(col==8)				//判断换行
		{									//若不判断,则自动从第一行到第三行
			col=0;
			row++;
		}
		if(row==4) row=0;		//一屏显示完,回到屏左上角
		LCM_WriteCmd(AC_TABLE[8*row+col]);
		LCM_WriteData(*str);	//一个汉字要写两次
		str++;
		LCM_WriteData(*str);
		str++;
		col++;
	}
}

/*----------------------------------------绘图-------------------------------------------*/
/**
  * @brief  发送16bit的数据
  * @param  data - 带发送的数据
  * @retval None
	* @note 
  */
void LCM_SendInt(uint16_t data)
{
	LCM_WriteData(data>>8);
	LCM_WriteData(data);
}

/**
  * @brief  在指定位置画点
  * @param  X - 第几行 (0~127)
  * @param  Y - 第几列 (0~63)
  * @retval None
	* @note 
  */
void LCM_Gra(uint8_t X, uint8_t Y)
{
	uint8_t XX;
	uint8_t YY;
	LCM_WriteCmd(0x34);
	LCM_WriteCmd(0x36);
	XX=X/16;
	YY=63-Y;
	if(YY>=32){XX=XX+8;YY-=32;}
	LCM_WriteCmd(0x80+YY);
	LCM_WriteCmd(0x80+XX);
	LCM_SendInt(0x8000>>(X%16));
}

void LCM_ShowBMP(uint8_t *bmp)
{
	uint16_t x=0;
	uint8_t i,j;
	LCM_WriteCmd(0x34);	//8Bit扩充指令集,即使是36H也要写两次
	LCM_WriteCmd(0x36);	//绘图ON,基本指令集里面36H不能开绘图
	for(i=0;i<32;i++)		//12864实际为256x32
	{
		LCM_WriteCmd(0x80|i);	//行位置
		LCM_WriteCmd(0x80);		//列位置
		for(j=0;j<32;j++)			//256/8=32 byte, 列位置每行自动增加
		{
			LCM_WriteData(bmp[x]);
			x++;
		}
	}
}

void LCM_ClrBMP(void)
{
	uint8_t  i,j,x,y;
	
	LCM_WriteCmd(0x3e);// RE=1  扩展指令选择  G=1  开图形显示 
	x=0x80;
	y=0x80;
	for(j=0;j<64;j++)
	{
		y=0x80;
		LCM_WriteCmd(x);
		LCM_WriteCmd(y);
		for(i=0;i<32;i++)
		{
			LCM_WriteData(0x00);
      
		}
		x=x+1;
	}    
}

/*----------------------------------------其他-------------------------------------------*/

/**
  * @brief  清屏
  * @param  none
  * @retval none
	* @note 
  */
void LCM_ClrScreen(void)
{
	LCM_WriteCmd(0x30);	//8BitMCU,基本指令集合
	LCM_WriteCmd(0x01);	//清屏
	delay_us(10);
}

/**
  * @brief  关闭显示和背光
  * @param  None
  * @retval None
	* @note 
  */
void LCM_Off(void)
{
	LCM_BK=0;//关背光
	LCM_WriteCmd(0x08);//关显示
}

/**
  * @brief  开显示和背光
  * @param  None
  * @retval None
	* @note 
  */
void LCM_On(void)
{
	LCM_WriteCmd(0x0C);//关显示
	LCM_BK=1;//关背光
}


/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/

