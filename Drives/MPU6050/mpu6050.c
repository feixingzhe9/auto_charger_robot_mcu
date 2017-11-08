/*******************************************************************************
 * @name    : MPU6050����
 * @author  : ������
 * @web     : WWW.UCORTEX.COM
 * @version : V1.0
 * @date    : 2014-04-03
 * @MDK     : KEIL MDK4.72a & KEL MDK 5.10
 * @brief   : ʵ��MPU5050�����������
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
 * MPU6050��ʵ���Էǳ�ǿ��6���˶���������V1.0�������ļ�ֻ�Ǽ򵥵�ʵ�ֶ�MPU6050��
 * ��ID�Ķ�ȡ��������֤I2Cͨ���Ƿ���������ȫ���MPU6050��������ο���һ�汾��
 *-----------------------------------------------------------------------------
 * @history
 * ----------------------------------------------------------------------------
 * ����ʱ�䣺2014-04-06    �����ˣ�������
 * �汾��¼��V1.0
 * �������ݣ��½�
 * ----------------------------------------------------------------------------
 *
 ******************************************************************************/
#include "mpu6050.h"

/**
  * @brief ��ȡMPU6050�豸ID
  * @param None
  * @retval ��ȡ�����豸ID��:0x68
  * @note
	* 	MPU6050�豸��ַ��:0xD0���豸ID�Ĵ�����ַ��: 0x75���豸IDֵ��0x68
	*		���I2C��ȡ����ֵ��0x68��˵����MPU6050��I2C��������
  */
uint8_t MPU6050_getDeviceID(void)
{
	uint8_t data;
	
	I2C_ReadOneByte(MPU6050_DevAddr, MPU6050_WHO_AM_I, &data);
	return data;
}


/********************* (C) COPYRIGHT 2014 WWW.UCORTEX.COM **********END OF FILE**********/