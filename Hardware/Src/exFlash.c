#include "exFlash.h"

#include "gps.h"

/*******************************************************************************
 * @brief  ����FLASH����
 * @param  SectorAddr��Ҫ������������ַ
 * @retval ��
 */
void SPI_FLASH_SectorErase(uint32_t SectorAddr)
{
	/* ����FLASHдʹ������ */
	SPI_FLASH_WriteEnable();
	SPI_FLASH_WaitForWriteEnd();

	/* �������� */
	/* ѡ��FLASH: CS�͵�ƽ */
	SPI_FLASH_CS_LOW();

	/* ������������ָ��*/
	SPI_FLASH_SendByte(W25X_SectorErase);

	/*���Ͳ���������ַ�ĸ�λ*/
	SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(SectorAddr & 0xFF);

	/* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
	SPI_FLASH_CS_HIGH();

	/* �ȴ��������*/
	SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
 * @brief  ����FLASH��������Ƭ����
 * @param  ��
 * @retval ��
 */
void SPI_FLASH_BulkErase(void)
{
	/* ����FLASHдʹ������ */
	SPI_FLASH_WriteEnable();

	/* ���� Erase */
	/* ѡ��FLASH: CS�͵�ƽ */
	SPI_FLASH_CS_LOW();

	/* �����������ָ��*/
	SPI_FLASH_SendByte(W25X_ChipErase);

	/* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
	SPI_FLASH_CS_HIGH();

	/* �ȴ��������*/
	SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
 * @brief  ��FLASH��ҳд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
 * @param	pBuffer��Ҫд�����ݵ�ָ��
 * @param WriteAddr��д���ַ
 * @param  NumByteToWrite��д�����ݳ��ȣ�����С�ڵ���SPI_FLASH_PerWritePageSize
 * @retval ��
 */
void SPI_FLASH_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	/* ����FLASHдʹ������ */
	SPI_FLASH_WriteEnable();

	/* ѡ��FLASH: CS�͵�ƽ */
	SPI_FLASH_CS_LOW();

	/* дҳдָ��*/
	SPI_FLASH_SendByte(W25X_PageProgram);

	/*����д��ַ*/
	SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(WriteAddr & 0xFF);

	if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
	{
		NumByteToWrite = SPI_FLASH_PerWritePageSize;
	}

	/* д������*/
	while (NumByteToWrite--)
	{
		/* ���͵�ǰҪд����ֽ����� */
		SPI_FLASH_SendByte(*pBuffer);
		/* ָ����һ�ֽ����� */
		pBuffer++;
	}

	/* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
	SPI_FLASH_CS_HIGH();

	/* �ȴ�д�����*/
	SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
 * @brief  ��FLASHд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
 * @param	pBuffer��Ҫд�����ݵ�ָ��
 * @param  WriteAddr��д���ַ
 * @param  NumByteToWrite��д�����ݳ���
 * @retval ��
 */
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	/*mod�������࣬��writeAddr��SPI_FLASH_PageSize��������������AddrֵΪ0*/
	Addr = WriteAddr % SPI_FLASH_PageSize;

	/*��count������ֵ���պÿ��Զ��뵽ҳ��ַ*/
	count = SPI_FLASH_PageSize - Addr;
	/*�����Ҫд��������ҳ*/
	NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	/*mod�������࣬�����ʣ�಻��һҳ���ֽ���*/
	NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

	/* Addr=0,��WriteAddr �պð�ҳ���� aligned  */
	if (Addr == 0)
	{
		/* NumByteToWrite < SPI_FLASH_PageSize */
		if (NumOfPage == 0)
		{
			SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
		}
		else /* NumByteToWrite > SPI_FLASH_PageSize */
		{
			/*�Ȱ�����ҳ��д��*/
			while (NumOfPage--)
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
			SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
		}
	}
	/* ����ַ�� SPI_FLASH_PageSize ������  */
	else
	{
		/* NumByteToWrite < SPI_FLASH_PageSize */
		if (NumOfPage == 0)
		{
			/*��ǰҳʣ���count��λ�ñ�NumOfSingleС��һҳд����*/
			if (NumOfSingle > count)
			{
				temp = NumOfSingle - count;
				/*��д����ǰҳ*/
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);

				WriteAddr +=  count;
				pBuffer += count;
				/*��дʣ�������*/
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
			}
			else /*��ǰҳʣ���count��λ����д��NumOfSingle������*/
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
			}
		}
		else /* NumByteToWrite > SPI_FLASH_PageSize */
		{
			/*��ַ����������count�ֿ������������������*/
			NumByteToWrite -= count;
			NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
			NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

			/* ��д��count�����ݣ�Ϊ��������һ��Ҫд�ĵ�ַ���� */
			SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);

			/* ���������ظ���ַ�������� */
			WriteAddr +=  count;
			pBuffer += count;
			/*������ҳ��д��*/
			while (NumOfPage--)
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
			if (NumOfSingle != 0)
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
			}
		}
	}
}

/*******************************************************************************
 * @brief  ��ȡFLASH����
 * @param 	pBuffer���洢�������ݵ�ָ��
 * @param   ReadAddr����ȡ��ַ
 * @param   NumByteToRead����ȡ���ݳ���
 * @retval ��
 */
void SPI_FLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	/* ѡ��FLASH: CS�͵�ƽ */
	SPI_FLASH_CS_LOW();

	/* ���� �� ָ�� */
	SPI_FLASH_SendByte(W25X_ReadData);

	/* ���� �� ��ַ��λ */
	SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
	SPI_FLASH_SendByte(ReadAddr & 0xFF);

	/* ��ȡ���� */
	while (NumByteToRead--) /* while there is data to be read */
	{
		/* ��ȡһ���ֽ�*/
		*pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
		/* ָ����һ���ֽڻ����� */
		pBuffer++;
	}

	/* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * @brief  ��ȡFLASH ID
 * @param 	��
 * @retval FLASH ID
 */
uint32_t SPI_FLASH_ReadID(void)
{
	uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

	/* ��ʼͨѶ��CS�͵�ƽ */
	SPI_FLASH_CS_LOW();

	/* ����JEDECָ���ȡID */
	SPI_FLASH_SendByte(W25X_JedecDeviceID);

	/* ��ȡһ���ֽ����� */
	Temp0 = SPI_FLASH_SendByte(Dummy_Byte);
	Temp1 = SPI_FLASH_SendByte(Dummy_Byte);
	Temp2 = SPI_FLASH_SendByte(Dummy_Byte);

	/* ֹͣͨѶ��CS�ߵ�ƽ */
	SPI_FLASH_CS_HIGH();

	/*�����������������Ϊ�����ķ���ֵ*/
	Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

	return Temp;
}

/*******************************************************************************
 * @brief  ��ȡFLASH Device ID
 * @param 	��
 * @retval FLASH Device ID
 */
uint32_t SPI_FLASH_ReadDeviceID(void)
{
	uint32_t Temp = 0;

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "RDID " instruction */
	SPI_FLASH_SendByte(W25X_DeviceID);
	SPI_FLASH_SendByte(Dummy_Byte);
	SPI_FLASH_SendByte(Dummy_Byte);
	SPI_FLASH_SendByte(Dummy_Byte);

	/* Read a byte from the FLASH */
	Temp = SPI_FLASH_SendByte(Dummy_Byte);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	return Temp;
}

/*******************************************************************************
* Function Name  : SPI_FLASH_StartReadSequence
* Description    : Initiates a read data byte (READ) sequence from the Flash.
*                  This is done by driving the /CS line low to select the device,
*                  then the READ instruction is transmitted followed by 3 bytes
*                  address. This function exit and keep the /CS line low, so the
*                  Flash still being selected. With this technique the whole
*                  content of the Flash is read with a single READ instruction.
* Input          : - ReadAddr : FLASH's internal address to read from.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_StartReadSequence(uint32_t ReadAddr)
{
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Read from Memory " instruction */
	SPI_FLASH_SendByte(W25X_ReadData);

	/* Send the 24-bit address of the address to read from -------------------*/
	/* Send ReadAddr high nibble address byte */
	SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	/* Send ReadAddr medium nibble address byte */
	SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
	/* Send ReadAddr low nibble address byte */
	SPI_FLASH_SendByte(ReadAddr & 0xFF);
}


/*******************************************************************************
 * @brief  ʹ��SPI��ȡһ���ֽڵ�����
 * @param  ��
 * @retval ���ؽ��յ�������
 */
uint8_t SPI_FLASH_ReadByte(void)
{
	return (SPI_FLASH_SendByte(Dummy_Byte));
}

/*******************************************************************************
 * @brief  ʹ��SPI����һ���ֽڵ�����
 * @param  byte��Ҫ���͵�����
 * @retval ���ؽ��յ�������
 */
uint8_t SPI_FLASH_SendByte(uint8_t byte)
{
	while(!__HAL_SPI_GET_FLAG(&exFLASH_SPI, SPI_FLAG_TXE));
	exFLASH_SPI.Instance->DR = byte;

	while(!__HAL_SPI_GET_FLAG(&exFLASH_SPI, SPI_FLAG_RXNE));
	return exFLASH_SPI.Instance->DR;
}

/*******************************************************************************
 * @brief  ��FLASH���� дʹ�� ����
 * @param  none
 * @retval none
 */
void SPI_FLASH_WriteEnable(void)
{
	/* ͨѶ��ʼ��CS�� */
	SPI_FLASH_CS_LOW();

	/* ����дʹ������*/
	SPI_FLASH_SendByte(W25X_WriteEnable);

	/*ͨѶ������CS�� */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * @brief  �ȴ�WIP(BUSY)��־����0�����ȴ���FLASH�ڲ�����д�����
 * @param  none
 * @retval none
 */
void SPI_FLASH_WaitForWriteEnd(void)
{
	uint8_t FLASH_Status = 0;

	/* ѡ�� FLASH: CS �� */
	SPI_FLASH_CS_LOW();

	/* ���� ��״̬�Ĵ��� ���� */
	SPI_FLASH_SendByte(W25X_ReadStatusReg);

	/* ��FLASHæµ����ȴ� */
	do
	{
		/* ��ȡFLASHоƬ��״̬�Ĵ��� */
		FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);
	}
	while ((FLASH_Status & WIP_Flag) == SET);  /* ����д���־ */

	/* ֹͣ�ź�  FLASH: CS �� */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * �������ģʽ
 */
void SPI_Flash_PowerDown(void)
{
	/* ͨѶ��ʼ��CS�� */
	SPI_FLASH_CS_LOW();

	/* ���� ���� ���� */
	SPI_FLASH_SendByte(W25X_PowerDown);

	/*ͨѶ������CS�� */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * ����
 */
void SPI_Flash_WAKEUP(void)
{
	/*ѡ�� FLASH: CS �� */
	SPI_FLASH_CS_LOW();

	/* ���� �ϵ� ���� */
	SPI_FLASH_SendByte(W25X_ReleasePowerDown);

	/* ֹͣ�ź� FLASH: CS �� */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * ģ�������ݸ�ʽת��
 */
static void exFLASH_DataFormatConvert(float value, EE_DataFormatEnum format,
							uint8_t* pBuffer)
{
	BOOL negative = FALSE;
	uint16_t temp = 0;

	/* �ж��Ƿ�Ϊ���� */
	if (value < 0)
		negative = TRUE;

	switch (format)
	{
	case FORMAT_INT:
		temp = (uint16_t)abs((int)(value));
		break;

	case FORMAT_ONE_DECIMAL:
		temp = (uint16_t)abs((int)(value * 10));
		break;

	case FORMAT_TWO_DECIMAL:
		temp = (uint16_t)abs((int)(value * 100));
		break;
	default:
		break;
	}

	*pBuffer = HalfWord_GetHighByte(temp);
	*(pBuffer + 1) = HalfWord_GetLowByte(temp);

	/* ���������λ��һ */
	if (negative)
		*pBuffer |= 0x8000;
}

/*******************************************************************************
 *
 */
static void exFLASH_LocationFormatConvert(double value, uint8_t* pBuffer)
{
	BOOL negative = FALSE;
	uint32_t temp;

	if (value < 0)
		negative = TRUE;

	/* ��ȡ�������� */
	*pBuffer = abs((int)value);

	temp = (uint32_t)((value - (*pBuffer)) * 1000000);

	if (negative)
		temp |= 0x800000;

	*(pBuffer + 1) = (uint8_t)((temp & 0x00FF0000) >> 16);
	*(pBuffer + 2) = (uint8_t)((temp & 0x0000FF00) >> 8);
	*(pBuffer + 3) = (uint8_t)(temp & 0x000000FF);
}


/*******************************************************************************
 *
 */
void exFLASH_SaveStructInfo(exFLASH_InfoTypedef* saveInfo,
							RT_TimeTypedef*      realTime,
							ANALOG_ValueTypedef* analogValue,
							GPS_LocateTypedef* location)
{
	/* �ṹ�帴λ���������ݳ��� */
	memset(saveInfo, 0, sizeof(exFLASH_InfoTypedef));

	/* ��ȡʱ�� */
	saveInfo->realTime.year  = realTime->date.Year;
	saveInfo->realTime.month = realTime->date.Month;
	saveInfo->realTime.day   = realTime->date.Date;
	saveInfo->realTime.hour  = realTime->time.Hours;
	saveInfo->realTime.min   = realTime->time.Minutes;
	/* Ϊ�����ݵ����룬������λ0 */
	saveInfo->realTime.sec = 0;

	/* �����ص��� */
	saveInfo->batteryLevel = analogValue->batVoltage;

	/* �ⲿ���״̬ */
	saveInfo->externalPowerStatus = INPUT_CheckPwrOnStatus();

	exFLASH_LocationFormatConvert(location->latitude,  (uint8_t*)&saveInfo->latitude);
	exFLASH_LocationFormatConvert(location->longitude, (uint8_t*)&saveInfo->longitude);

	/* ģ�����ݸ�ʽת�� */
	exFLASH_DataFormatConvert(analogValue->temp1, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp1);
	exFLASH_DataFormatConvert(analogValue->humi1, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi1);
	exFLASH_DataFormatConvert(analogValue->temp2, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp2);
	exFLASH_DataFormatConvert(analogValue->humi2, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi2);
	exFLASH_DataFormatConvert(analogValue->temp3, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp3);
	exFLASH_DataFormatConvert(analogValue->humi3, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi3);
	exFLASH_DataFormatConvert(analogValue->temp4, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp4);
	exFLASH_DataFormatConvert(analogValue->humi4, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi4);

//	exFLASH_WriteBuffer(EE_FlashInfoSaveAddr, (uint8_t*)&exFLASH_SaveInfo,
//			sizeof(exFLASH_InfoTypedef));
//
//	EE_FlashInfoSaveAddr += sizeof(exFLASH_InfoTypedef);
//	EEPROM_WriteBytes(EE_ADDR_FLASH_INFO_SAVE_ADDR, &EE_FlashInfoSaveAddr,
//			sizeof(EE_FlashInfoSaveAddr));
}

/*******************************************************************************
 *
 */
void exFLASH_ReadStructInfo(exFLASH_InfoTypedef* info)
{
//	exFLASH_ReadBuffer(EE_FlashInfoReadAddr, (uint8_t*)info,
//			sizeof(exFLASH_InfoTypedef));

//	EE_FlashInfoReadAddr += sizeof(exFLASH_InfoTypedef);
//	EEPROM_WriteBytes(EE_ADDR_FLASH_INFO_READ_ADDR, &EE_FlashInfoReadAddr,
//			sizeof(exFLASH_InfoTypedef));
}
































