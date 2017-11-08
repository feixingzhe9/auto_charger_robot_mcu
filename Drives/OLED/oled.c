/*******************************************************************************
 * @name    : OLED模块底层驱动
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.0
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 通过4线SPI驱动OLED显示
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
 * OLED_PWR  -> PC13 : OLED电源控制口，高电平打开电源
 * OLED_CS   -> PB3  : OLED SPI选通
 * OLED_RST  -> PB4  : OLED复位
 * OLED_DC   -> PB5  : OLED 数据/命令选择口 D/C#
 * OLED_SCLK -> PB6  : OLED SPI时钟
 * OLED_SDIN -> PB7  : OLED SPI数据
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
#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"  	 
#include "delay.h"

/**
  * OLED的显存存放格式
  * -------------------
  * [0]0 1 2 3 ... 127	
  * [1]0 1 2 3 ... 127	
  * [2]0 1 2 3 ... 127	
  * [3]0 1 2 3 ... 127	
  * [4]0 1 2 3 ... 127	
  * [5]0 1 2 3 ... 127	
  * [6]0 1 2 3 ... 127	
  * [7]0 1 2 3 ... 127
	*/
uint8_t OLED_GRAM[128][8];	 

/**
  * @brief 更新显存到OLED
  * @param none
  * @retval none
  * @note 
  */
void OLED_Refresh_Gram(void)
{
	uint8_t page, col;		    
	for(page=0; page<8; page++)  
	{  
		OLED_WR_Byte (0xb0+page,OLED_CMD); //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(col=0; col<128; col++)
		  OLED_WR_Byte(OLED_GRAM[col][page],OLED_DATA); 
	}
}

/**
  * @brief 通过4线SPI向SSD1306写入一个字节。
  * @param dat: 要写入的数据/命令
  * @param DC:  数据/命令标志 1,表示数据; 0,表示命令;
  * @retval none
  * @note MSB -> LSB传输顺序
  */
void OLED_WR_Byte(uint8_t dat,uint8_t DC)
{	
	uint8_t i;
	
	OLED_DC = DC;	
	OLED_CS = 0;
	for(i=0;i<8;i++)
	{			  
		OLED_SCLK = 0;
		if(dat&0x80)
		   OLED_SDIN = 1;
		else 
		   OLED_SDIN = 0;
		OLED_SCLK = 1;
		dat<<=1;   
	}				 		  
	OLED_CS = 1;
	OLED_DC = 1;   	  
}
	  	  
/**
  * @brief 开启OLED显示
  * @param none
  * @retval none
  * @note 开启内部升压电路（charge pump）
  */
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //charge pump setting命令
	OLED_WR_Byte(0X14,OLED_CMD);  //charge pump on，开启内部升压
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}

/**
  * @brief 关闭OLED显示
  * @param none
  * @retval none
  * @note 关闭内部升压电路（charge pump）
  */   
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //charge pump setting命令
	OLED_WR_Byte(0X10,OLED_CMD);  //charge pump off，关闭内部升压
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}		   			 

/**
  * @brief 清空屏幕显示
  * @param none
  * @retval none
  * @note 清空显存，所有像素都关闭
  */ 
void OLED_Clear(void)  
{  
	uint8_t page, col;  
	for(page=0; page<8; page++)
	{
	  for(col=0; col<128; col++)
	    OLED_GRAM[col][page]=0x00;
	}
	OLED_Refresh_Gram();//更新显示
}

/**
  * @brief 画点
	* @param x: X轴坐标（0~127）
	* @param y: Y轴坐标（0~63 ）
	* @param dot: 1 填充；0 清空
  * @retval none
  * @note
  */ 
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t dot)
{
	uint8_t page, bit, data = 0;
	if(x>127 || y>63) return;//超出范围了.
	page = y / 8;
	bit = y % 8;
	data = 1 << bit;
	if(dot)OLED_GRAM[x][page] |= data;
	else OLED_GRAM[x][page] &= ~data;
}

/**
  * @brief x1,y1,x2,y2 填充区域的对角坐标
	* @param (x1,y1): 填充区域起始坐标
	* @param (x2,y2): 填充区域结束坐标
	* @param dot: 1 填充；0 清空
  * @retval none
  * @note 确保x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63	 
  */ 
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot)  
{  
	uint8_t x,y;  
	for(x=x1;x<=x2;x++)
	{
		for(y=y1;y<=y2;y++)OLED_DrawPoint(x,y,dot);
	}													    
	OLED_Refresh_Gram();//更新显示
}

/**
  * @brief 在指定位置显示一个字符
	* @param x:（0~127）
	* @param y:（0~63 ）
	* @param size: 选择字体 16/12 
  * @param mode: 0 反白显示；1 正常显示
  * @retval none
  * @note
  */ 
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size,uint8_t mode)
{      			    
	uint8_t temp, t, t1;
	uint8_t y0 = y;
	chr = chr - ' ';//得到偏移后的值				   
	for(t = 0; t < size; t++)
	{   
		if(size==12)temp = oled_asc2_1206[chr][t];	//调用1206字体
		else temp = oled_asc2_1608[chr][t];	//调用1608字体 	                          
		for(t1 = 0; t1 < 8; t1++)
		{
			if(temp & 0x80)OLED_DrawPoint(x, y, mode);
			else OLED_DrawPoint(x, y, !mode);
			temp <<= 1;
			y++;
			if((y-y0) == size)
			{
				y = y0;
				x++;
				break;
			}
		}  	 
   }          
}

/**
  * @brief m^n函数
	* @param m:基数
	* @param n:幂
  * @retval m^n
  * @note
  */ 
uint32_t oled_pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}				  

/**
  * @brief 在指定位置显示数字
	* @param x:（0~127）
	* @param y:（0~63 ）
  * @param num:数值(0~4294967295);
  * @param len: 数字位数
	* @param size: 选择字体 16/12 
  * @retval none
  * @note
  */ 
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size/2)*t,y,' ',size,1);
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size/2)*t,y,temp+'0',size,1); 
	}
}

/**
  * @brief 显示数字,高位为0,还是显示
	* @param x:（0~127）
	* @param y:（0~63 ）
  * @param num:数值(0~4294967295);
  * @param len: 数字位数
	* @param size: 选择字体 16/12 
  * @param mode:
  * [7]:0,不填充;1,填充0.
  * [6:3]:保留
  * [2:1]:00,十进制 01,十六进制 
  * [0]:0,反白显示;1,正常显示
  * @retval none
  * @note
  */ 
void OLED_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
{  
	uint8_t t,temp;
	uint8_t enshow=0;
	uint8_t system=0;
	
	system=(mode>>1)&0x03;
	
	for(t=0;t<len;t++)
	{
		if(system==1)//16进制
		{
			temp=(num>>(4*(len-t-1)))&0x0f;
		}
		else//10进制
		{
			temp=(num/oled_pow(10,len-t-1))%10;
		}
		
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0x80)OLED_ShowChar(x+(size/2)*t,y,'0',size,mode&0x01);  
				else OLED_ShowChar(x+(size/2)*t,y,' ',size,mode&0x01);  
 				continue;
			}else enshow=1; 
		 	 
		}
		if(system==1&&temp>=10)
			OLED_ShowChar(x+(size/2)*t,y,temp-10+'A',size,mode&0x01);
		else
			OLED_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0x01); 
	}
}

/**
  * @brief 显示字符串
	* @param x:（0~127）
	* @param y:（0~63 ）
  * @param *p:字符串起始地址
  * @retval none
  * @note 用16字体
  */
void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p)
{
#define MAX_CHAR_POSX 122
#define MAX_CHAR_POSY 58          
    while(*p!='\0')
    {       
        if(x>MAX_CHAR_POSX){x=0;y+=16;}
        if(y>MAX_CHAR_POSY){y=x=0;OLED_Clear();}
        OLED_ShowChar(x,y,*p,16,1);	 
        x+=8;
        p++;
    }  
}	   

/**
  * @brief  SSD1306初始化	
  * @param  None
  * @retval None
	* @note  
  *    4线SPI接线方式：
  *    CS   -> PB3 （复用了JTAG的 JTDO）
  *    RST  -> PB4 （复用了JTAG的 JNTRST）
  *    D/C# -> PB5
  *    SCLK -> PB6
  *    SDIN -> PB7
  *    - OLED模块电源可控，控制管脚是PC13
  *    - PB3和PB4复用了JTAG的JTDO和JNTRST，因此STM32需配置成SWD模式
  */
void OLED_Init(void)
{ 	
 
 	GPIO_InitTypeDef  GPIO_InitStructure;
	
 	/* 使能GPIOB,GPIOC，AFIO时钟 */
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);
	
	/* PC13配置为推挽输出，用于控制OLED电源开关，注意PC13不支持5V耐压，速度只能配置到2MHz（详情参考规格书） */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	//速度2MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);	//初始化
	GPIO_SetBits(GPIOC,GPIO_Pin_13);//打开OLED电源
	
  /* 配置STM32为SWD调试模式，注意要先开启AFIO时钟 */
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);	//SWD模式
	
	/* 配置PB3,PB4,PB5,PB6,PB7推挽输出 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);						//初始化
 	GPIO_SetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);
  							  
	OLED_RST = 0;
	delay_ms(100);
	OLED_RST = 1; 
					  
	OLED_WR_Byte(0xAE,OLED_CMD); //关闭显示
	OLED_WR_Byte(0xD5,OLED_CMD); //设置时钟分频因子,振荡频率
	OLED_WR_Byte(80,OLED_CMD);   //[3:0],分频因子;[7:4],振荡频率
	OLED_WR_Byte(0xA8,OLED_CMD); //设置驱动路数
	OLED_WR_Byte(0X3F,OLED_CMD); //默认0X3F(1/64) 
	OLED_WR_Byte(0xD3,OLED_CMD); //设置显示偏移
	OLED_WR_Byte(0X00,OLED_CMD); //默认为0

	OLED_WR_Byte(0x40,OLED_CMD); //设置显示开始行 [5:0],行数.
													    
	OLED_WR_Byte(0x8D,OLED_CMD); //电荷泵设置
	OLED_WR_Byte(0x14,OLED_CMD); //bit2，开启/关闭
	OLED_WR_Byte(0x20,OLED_CMD); //设置内存地址模式
	OLED_WR_Byte(0x02,OLED_CMD); //[1:0],00:水平寻址模式; 01:垂直寻址模式; 10:页寻址模式;默认10;
	OLED_WR_Byte(0xA1,OLED_CMD); //SEG重定义设置,bit0:0,0->0;1,0->127;
	OLED_WR_Byte(0xC8,OLED_CMD); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
	OLED_WR_Byte(0xDA,OLED_CMD); //设置COM硬件引脚配置
	OLED_WR_Byte(0x12,OLED_CMD); //[5:4]配置
		 
	OLED_WR_Byte(0x81,OLED_CMD); //对比度设置
	OLED_WR_Byte(0xEF,OLED_CMD); //1~255;默认0X7F (亮度设置,越大越亮)
	OLED_WR_Byte(0xD9,OLED_CMD); //设置预充电周期
	OLED_WR_Byte(0xf1,OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB,OLED_CMD); //设置VCOMH 电压倍率
	OLED_WR_Byte(0x30,OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA4,OLED_CMD); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	OLED_WR_Byte(0xA6,OLED_CMD); //设置显示方式;bit0:1,反相显示;0,正常显示	    						   
	OLED_WR_Byte(0xAF,OLED_CMD); //开启显示	 
	OLED_Clear();
}

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/

