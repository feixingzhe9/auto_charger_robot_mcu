/*******************************************************************************
 * @name    : STM32 Flash虚拟EEPROM驱动
 * @author  : 布谷鸟
 * @web     : WWW.UCORTEX.COM
 * @version : V1.2
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : 介绍通过STM32的FLASH虚拟成EEPROM，非常经典，值得研究！
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
 * 这个驱动文件不是简单的对STM32的FLASH进行页擦除，写，读等操作，而是考虑到FLASH
 * 操作的特性，定义了两个页(Page)对FLASH进行操作，并引入虚拟地址操作，通过这个方
 * 法，可以显著减少页擦除操作，从而成倍的延长FLASH操作寿命和缩短了FLASH操作时间。
 * 当然也可以定义更多的Page满足对FLASH寿命要求非常苛刻的场合。本驱动在ST官方提供
 * 的范例上进行了优化，并对其中的Bug进行了修正。虚拟地址表的定义也使得EEPROM操作
 * 变得非常的便捷。
 * 定义的EEPROM起始地址为FLASH第62KByte(0X08000000+62*1024)开始的2个page，虚拟
 * EEPROM大小为100个16bit的数值（最大可以管理255个16bit数值）。
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * 更改时间：2014-04-06    更改人：布谷鸟
 * 版本记录：V1.0
 * 更改内容：新建
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
#include "stm32_eeprom.h" 

//虚拟地址表，虚拟地址必须是唯一的，且不能是0XFFFF
//这里定义了100个虚拟地址，虚拟地址值与VirtAddrTab下标相同
uint16_t VirtAddVarTab[NumbOfVar] = 
{
	0,	1,	2,	3,	4,	5,	6,	7,	8,	9,
	10,	11,	12,	13,	14,	15,	16,	17,	18,	19,
	20,	21,	22,	23,	24,	25,	26,	27,	28,	29,
	30,	31,	32,	33,	34,	35,	36,	37,	38,	39,
	40,	41,	42,	43,	44,	45,	46,	47,	48,	49,
	50,	51,	52,	53,	54,	55,	56,	57,	58,	59,
	60,	61,	62,	63,	64,	65,	66,	67,	68,	69,
	70,	71,	72,	73,	74,	75,	76,	77,	78,	79,
	80,	81,	82,	83,	84,	85,	86,	87,	88,	89,
	90,	91,	92,	93,	94,	95,	96,	97,	98,	99,
};

//从模拟EEPROM中读取到的虚拟地址对应的数值
uint16_t DataVar = 0;

static FLASH_Status EE_Format(void);
static uint16_t EE_FindValidPage(uint8_t Operation);
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data);

/**
  * @brief  系统上电后，根据页状态初始化EEPROM
  * @param  None.
  * @retval - FLASH操作错误码: 写操作错误
  *         - FLASH_COMPLETE: 操作成功
	* @note 因为系统掉电后，FLASH的内容不会丢失，所以初始化过程是根据上一次PAGE状态进行的
  */
uint16_t EE_Init(void)
{
  uint16_t PageStatus0 = 6, PageStatus1 = 6;//Page状态，16bit位宽
  uint16_t VarIdx = 0;
  uint16_t EepromStatus = 0, ReadStatus = 0;
  int16_t x = -1;
  uint16_t  FlashStatus;

  //获取PAGE0状态
  PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);
  //获取PAGE1状态
  PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

  //根据PAGE状态配置EEPROM
  switch (PageStatus0)
  {
    case ERASED:
      if (PageStatus1 == VALID_PAGE) // Page0 ERASED, Page1 VALID_PAGE
      {
        //擦除Page0流程
				FLASH_Unlock();
        FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS);//擦除
				FLASH_Lock();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
      }
      else if (PageStatus1 == RECEIVE_DATA) //Page0 ERASED, Page1 RECEIVE_DATA
      {
        //擦除Page0
				FLASH_Unlock();
        FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS);
				FLASH_Lock();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
				
        //将Page1设置为有效Page
        FlashStatus = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS, VALID_PAGE);
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
      }
      else //EEPROM首次访问(Page0&1都是ERRASED状态)或无效状态 -> 格式化EEPROM
      {
				//格式化EEPROM: 擦除Page0和Page1，设置Page0为有效Page
        FlashStatus = EE_Format();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
      }
      break;

    case RECEIVE_DATA:
      if (PageStatus1 == VALID_PAGE) // Page0 RECEIVE_DATA, Page1 VALID_PAGE 
      {
        //说明上次正从Page1转移数据到Page0时，系统掉电了！需要继续完成数据转移。
        for (VarIdx = 0; VarIdx < NumbOfVar; VarIdx++)
        {
					//PAGE0中读出的虚拟地址有效，这个虚拟地址和数据不需要转移
          if (( *(__IO uint16_t*)(PAGE0_BASE_ADDRESS + 6 + 4*VarIdx)) == VirtAddVarTab[VarIdx])
          {
            x = VarIdx;
          }
          if (VarIdx != x)//读出的虚拟地址是无效的，转移数据
          {
            //在PAGE1读出该虚拟地址最近一次存储的数值
            ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
            //如果在PAGE1中找到了该虚拟地址并且读出了相应的数据
            if (ReadStatus != 0x1)
            {
              //将读出的数据转移到Page0中
              EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
              if (EepromStatus != FLASH_COMPLETE) return EepromStatus;//操作失败，返回错误码
            }
          }
        }			
        //将Page0标志为有效页
				FLASH_Unlock();
        FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE);
				FLASH_Lock();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
				
        //擦除Page1
				FLASH_Unlock();
        FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS);
				FLASH_Lock();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
      }
			
      else if (PageStatus1 == ERASED) // Page0 RECEIVE_DATA, Page1 ERASED 
      {
        //擦除Page1
				FLASH_Unlock();
        FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS);
				FLASH_Lock();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
				
        //将Page0标识为有效页
				FLASH_Unlock();
        FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE);
				FLASH_Lock();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
      }
			
      else//其他状态都是错误的，格式化EEPROM。
      {
        //格式化EEPROM: 擦除Page0和Page1，设置Page0为有效Page
        FlashStatus = EE_Format();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
      }
      break;

    case VALID_PAGE:
      if (PageStatus1 == VALID_PAGE) //错误的状态
      {
        //格式化EEPROM: 擦除Page0和Page1，设置Page0为有效Page
        FlashStatus = EE_Format();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
      }
			
      else if (PageStatus1 == ERASED) /* Page0 VALID_PAGE, Page1 ERASED */
      {
        //擦除Page1
				FLASH_Unlock();
        FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS);
				FLASH_Lock();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
      }
			
      else /* Page0 VALID_PAGE, Page1 RECEIVE_DATA */
      {
        //说明从Page0传输数据到Page1的时候，系统掉电了，需要继续完成数据传输！
        for (VarIdx = 0; VarIdx < NumbOfVar; VarIdx++)
        {
					//从Page1读取到的虚拟地址是有效的，说明这个虚拟地址的数据上次已经传输了，不需要再传输。
          if ((*(__IO uint16_t*)(PAGE1_BASE_ADDRESS + 6 + 4*VarIdx)) == VirtAddVarTab[VarIdx])
          {
            x = VarIdx;
          }
					//从Page1读取到的虚拟地址是无效的，需要重新传输！
          if (VarIdx != x)
          {
            //从Page0读取该虚拟地址最近一次的存储值
            ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
            //如果成功的读取到了该虚拟地址的存储值
            if (ReadStatus != 0x1)
            {
              //将该虚拟地址和存储值传输到Page1
              EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
              if (EepromStatus != FLASH_COMPLETE) return EepromStatus;//操作失败，返回错误码
            }
          }
        }
        //将Page1设置为有效页
				FLASH_Unlock();
        FlashStatus = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS, VALID_PAGE);
				FLASH_Lock();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码	
        //擦除Page0
				FLASH_Unlock();
        FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS);
				FLASH_Lock();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码		
      }
      break;

    default:	//其他任何情况
      //格式化EEPROM: 擦除Page0和Page1，设置Page0为有效Page
        FlashStatus = EE_Format();
        if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//操作失败，返回错误码
      break;
  }
	
  return FLASH_COMPLETE;
}

/**
  * @brief  从EEPROM中读取虚拟地址VirtAddress对应的存储值
  * @param  VirtAddress: 指定的虚拟地址
  * @param  Data: 读取到的值
  * @retval
  *		- 0: 成功地读取到了相应的存储值
  *		- 1: 指定的虚拟地址未找到
  *		- NO_VALID_PAGE: 未找到有效页
  */
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data)
{
  uint16_t ValidPage = PAGE0;
  uint16_t AddressValue = 0, ReadStatus = 1;
  uint32_t Address = PAGE0_END_ADDRESS, PageStartAddress = PAGE0_BASE_ADDRESS;

  //获取读操作有效页
  ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

  //如果未获取到有效页
  if (ValidPage == NO_VALID_PAGE)
  {
    return  NO_VALID_PAGE;
  }

  //有效页首地址
  PageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE));

  //有效页尾地址
  Address = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + ValidPage) * PAGE_SIZE));

  //从尾地址开始查找对应的虚拟地址
  while (Address > (PageStartAddress + 2))
  {
    //读取虚拟地址
    AddressValue = (*(__IO uint16_t*)Address);

    //如果读取到的虚拟地址等于指定的虚拟地址
    if (AddressValue == VirtAddress)
    {
      //读取虚拟地址对应的存储值
      *Data = (*(__IO uint16_t*)(Address - 2));

      //设置成功读取到存储值
      ReadStatus = 0;

      break;
    }
    else
    {
      //查找下一个虚拟地址
      Address = Address - 4;
    }
  }
	
  return ReadStatus;
}

/**
  * @brief  写入一个数据到EEPROM
  * @param  VirtAddress: 指定的虚拟地址
  * @param  Data: 需要写入的16bit数据
  * @retval 
  *		- FLASH_COMPLETE: 写入成功
  *		- PAGE_FULL: 有效页数据已经满了（表示有效数据需要转移）
  *		- NO_VALID_PAGE: 没找到有效页
  *		- 其他FLASH操作错误码
  */
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data)
{
  uint16_t Status = 0;

  //写入数据和虚拟地址到EEPROM
  Status = EE_VerifyPageFullWriteVariable(VirtAddress, Data);

  //如果有效页数据已满
  if (Status == PAGE_FULL)
  {
    //进行也传输：即将有效数据转移到另一页，并更改页状态
    Status = EE_PageTransfer(VirtAddress, Data);
  }
	
  return Status;
}

/**
  * @brief  格式化EEPROM：擦除Page0和Page1，将Page0设为有效页
  * @param  None
  * @retval Flash操作码
  */
static FLASH_Status EE_Format(void)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;

  //擦除Page0
	FLASH_Unlock();
  FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS);
	FLASH_Lock();
  //如果操作失败，返回错误码
  if (FlashStatus != FLASH_COMPLETE) return FlashStatus;

  //将Page0设为有效页
	FLASH_Unlock();
  FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE);
	FLASH_Lock();
  //如果操作失败，返回错误码
  if (FlashStatus != FLASH_COMPLETE) return FlashStatus;

  //擦除Page1
	FLASH_Unlock();
  FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS);
	FLASH_Lock();
  return FlashStatus;
}

/**
  * @brief  查找读EEPROM或写EEPROM操作的有效页
  * @param  Operation: 读操作或写操作
  *     @arg READ_FROM_VALID_PAGE: 读操作
  *     @arg WRITE_IN_VALID_PAGE: 写操作
  * @retval 有效页(PAGE0 or PAGE1)，如果没发现有效页，则返回 NO_VALID_PAGE
	*/
static uint16_t EE_FindValidPage(uint8_t Operation)
{
  uint16_t PageStatus0 = 6, PageStatus1 = 6;

  //读取Page0状态
  PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);

  //读取Page1状态
  PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

	//根据读操作或写操作散转
  switch (Operation)
  {
    case WRITE_IN_VALID_PAGE:	//写操作
      if (PageStatus1 == VALID_PAGE)
      {
        //正从Page1传输数据到Page0
        if (PageStatus0 == RECEIVE_DATA)
        {
          return PAGE0;	//Page0为写操作有效页
        }
        else
        {
          return PAGE1;	//Page1为写操作有效页
        }
      }
      else if (PageStatus0 == VALID_PAGE)
      {
        //正从Page0传输数据到Page1
        if (PageStatus1 == RECEIVE_DATA)
        {
          return PAGE1;	//Page1为写操作有效页
        }
        else
        {
          return PAGE0;	//Page0为写操作有效页
        }
      }
      else
      {
        return NO_VALID_PAGE;//未找到有效页
      }

    case READ_FROM_VALID_PAGE:	//读操作
      if (PageStatus0 == VALID_PAGE)
      {
        return PAGE0;	//Page0为读有效页
      }
      else if (PageStatus1 == VALID_PAGE)
      {
        return PAGE1;	//Page1为读有效页
      }
      else
      {
        return NO_VALID_PAGE ;	//未找到有效页
      }

    default:
      return PAGE0;	//返回Page0，正常情况下，不应该执行到这里！
  }
}

/**
  * @brief  判断有效页是否满了，如果没有，则将虚拟地址和数值写入EEPROM
  * @param  VirtAddress: 指定的虚拟地址
  * @param  Data: 需要写入的数值
  * @retval
  *		- FLASH_COMPLETE: 操作成功
  *		- PAGE_FULL: 有效页满，需要转移数据
  *		- NO_VALID_PAGE: 未找到有效页
  *		- Flash error code: 其他FLASH操作错误码
  */
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;
  uint16_t ValidPage = PAGE0;
  uint32_t Address = PAGE0_BASE_ADDRESS, PageEndAddress = PAGE0_END_ADDRESS;

  //获取写操作有效页
  ValidPage = EE_FindValidPage(WRITE_IN_VALID_PAGE);

  //未找到有效页
  if (ValidPage == NO_VALID_PAGE)
  {
    return  NO_VALID_PAGE;
  }

  //计算有效页首地址
  Address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE));

  //计算有效页尾地址
  PageEndAddress = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + ValidPage) * PAGE_SIZE));
	
  //从页首开始查找可以写入的空地址
  while (Address < PageEndAddress)
  {
    // 如果Address 和 Address+2 内容是 0XFFFFFFFF，说明这个地址是空的
    if ((*(__IO uint32_t*)Address) == 0xFFFFFFFF)
    {
      //写入数据
			FLASH_Unlock();
      FlashStatus = FLASH_ProgramHalfWord(Address, Data);
			FLASH_Lock();
      //如果操作失败，返回错误码
			if (FlashStatus != FLASH_COMPLETE) return FlashStatus;
      //写入虚拟地址
			FLASH_Unlock();
      FlashStatus = FLASH_ProgramHalfWord(Address + 2, VirtAddress);
			FLASH_Lock();
      return FlashStatus;
    }
    else
    {
      Address = Address + 4;//查找下一个位置
    }
  }

  //没找到空地址，返回有效页满！
  return PAGE_FULL;
}

/**
  * @brief  将最后修改的有效值从已满的Page传到另一Page
  * @param  VirtAddress: 16bit虚拟地址
  * @param  Data: 16bit数据
  * @retval
  *		- FLASH_COMPLETE: 操作成功
  *		- PAGE_FULL: 页已满
  *		- NO_VALID_PAGE: 未找到有效页
  *		- Flash error code: 其他FLASH操作错误码
  */
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;
  uint32_t NewPageAddress = 0x080103FF, OldPageAddress = 0x08010000;
  uint16_t ValidPage = PAGE0, VarIdx = 0;
  uint16_t EepromStatus = 0, ReadStatus = 0;

  //获取读操作有效页
  ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

  if (ValidPage == PAGE1)	//当前Page1有效
  {
    //设置新的Page地址为Page0首地址，数据转移的目标Page
    NewPageAddress = PAGE0_BASE_ADDRESS;

    //设置老的Page为Page1首地址，数据从老Page转移到新Page
    OldPageAddress = PAGE1_BASE_ADDRESS;
  }
  else if (ValidPage == PAGE0)//当前Page0有效
  {
    ///设置新的Page地址为Page1首地址，数据转移的目标Page
    NewPageAddress = PAGE1_BASE_ADDRESS;

    //设置老的Page为Page0首地址，数据从老Page转移到新Page
    OldPageAddress = PAGE0_BASE_ADDRESS;
  }
  else
  {
    return NO_VALID_PAGE; //未找到有效页
  }

  //设置新Page的页状态为RECEIVE_DATA
	FLASH_Unlock();
  FlashStatus = FLASH_ProgramHalfWord(NewPageAddress, RECEIVE_DATA);
	FLASH_Lock();
  if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//Flash操作失败，返回错误码！

  //将虚拟地址和相应的数值写入RCEIVE_DATA页
  EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddress, Data);
  if (EepromStatus != FLASH_COMPLETE) return EepromStatus;//EEPROM操作失败，返回错误码！

  //开始转移数据: 将最后的最近更新的有效地址和对应的数值传输到新Page中
	//按虚拟地址表顺序依次传输，并且整个虚拟地址表都查一遍
  for (VarIdx = 0; VarIdx < NumbOfVar; VarIdx++)
  {
    if (VirtAddVarTab[VarIdx] != VirtAddress)//非刚才传输的虚拟地址（因为刚才已经传输过最新的值了）
    {
      ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);//读取对应虚拟地址表的数值
      if (ReadStatus != 0x1)//成功读取到了对应虚拟地址的数值
      {
        //将虚拟地址和数值传输到新Page中
        EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
        if (EepromStatus != FLASH_COMPLETE) return EepromStatus;//EEPROM操作失败，返回错误码！
      }
    }
  }

  //擦除老Page
	FLASH_Unlock();
  FlashStatus = FLASH_ErasePage(OldPageAddress);
	FLASH_Lock();
  if (FlashStatus != FLASH_COMPLETE) return FlashStatus;//Flash操作失败，返回错误码！

  //将新Page设置为当前有效Page
	FLASH_Unlock();
  FlashStatus = FLASH_ProgramHalfWord(NewPageAddress, VALID_PAGE);
	FLASH_Lock();
  return FlashStatus;
}

/**
  * @brief  从EEPROM中读取指定长度的数据
  * @param  VirtAddrOffset: 指定的虚拟地址偏移量
  * @param  Data: 数值存放起始地址
	* @param	NbrOfData: 读取的数据个数
  * @retval
  *		- 0: 读取成功
  *		- 1: 指定的虚拟地址未找到
  *		- NO_VALID_PAGE: 未找到有效页
  */
uint16_t EE_ReadBuf(uint16_t VirtAddrOffset, uint16_t* Data, uint8_t NbrOfData)
{
	uint16_t ReadStatus = 1;
	uint8_t VarIdx = 0;
	
	if(NbrOfData > NumbOfVar) return 1;//如果读取的数据个数超出范围
	
	for (VarIdx = 0; VarIdx < NbrOfData; VarIdx++)
	{
		//从虚拟地址读取数值
		ReadStatus = EE_ReadVariable(VirtAddVarTab[VirtAddrOffset+VarIdx], Data+VarIdx);
	}
	return ReadStatus;
}

/**
  * @brief  向EEPROM指定地址开始写入一段数值
  * @param  VirtAddrOffset: 指定的虚拟地址偏移量
  * @param  Data: 需要写入的16bit数据起始地址
	* @param	NbrOfData: 需要写入的数据个数
  * @retval 
  *		- FLASH_COMPLETE: 写入成功
  *		- PAGE_FULL: 有效页数据已经满了（表示有效数据需要转移）
	*		- PAGE_OVER: 待写入的数据个数超超出范围
  *		- NO_VALID_PAGE: 没找到有效页
  *		- 其他FLASH操作错误码
  */
uint16_t EE_WriteBuf(uint16_t VirtAddrOffset, uint16_t* Data, uint8_t NbrOfData)
{
	uint16_t status = 0;
	uint8_t VarIdx = 0;
	
	if(NbrOfData > NumbOfVar) return	PAGE_OVER;//待写入的数据个数超超出范围
	
	for (VarIdx = 0; VarIdx < NbrOfData; VarIdx++)
	{
		//向EEPROM虚拟地址写入数据
		status = EE_WriteVariable(VirtAddVarTab[VirtAddrOffset+VarIdx], *(Data + VarIdx));
	}
	
	return status;
	
}

/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/
