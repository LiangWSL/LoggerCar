#include "exFlash.h"

#include "gps.h"


/*******************************************************************************
 * ��дһ���ֽ�����
 */
static uint8_t exFLASH_WriteReadByte(uint8_t byte)
{
	while(!__HAL_SPI_GET_FLAG(&exFLASH_SPI, SPI_FLAG_TXE));
	exFLASH_SPI.Instance->DR = byte;

	while(!__HAL_SPI_GET_FLAG(&exFLASH_SPI, SPI_FLAG_RXNE));
	return exFLASH_SPI.Instance->DR;
}

/******************************************************************************/
uint32_t exFLASH_ReadDeviceID(void)
{
	uint32_t temp = RESET;
	uint8_t temp1 = RESET, temp2 = RESET, temp3 = RESET;

	exFLASH_CS_ENABLE();

	exFLASH_WriteReadByte(exFLASH_CMD_JEDEC_DIVICE_ID);

	/* ������ */
	temp1 = exFLASH_WriteReadByte(DUMMY_BYTE);
	temp2 = exFLASH_WriteReadByte(DUMMY_BYTE);
	temp3 = exFLASH_WriteReadByte(DUMMY_BYTE);

	exFLASH_CS_DISABLE();

	temp = (temp1 << 16) | (temp2 << 8) | temp3;

	return temp;
}

/*******************************************************************************
 * ����flashдʹ��ָ��
 */
static void exFLASH_WriteEnable(void)
{
	exFLASH_CS_ENABLE();

	exFLASH_WriteReadByte(exFLASH_CMD_WRITE_ENABLE);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * FLASH оƬ���ڲ��洢����д��������Ҫ����һ����ʱ�䣬������������ͨѶ������һ˲����ɵģ�������д����
 * ����Ҫȷ��FLASHоƬ�����С�ʱ�����ٴ�д��
 */
static void exFLASH_WaitForIdle(void)
{
	exFLASH_CS_ENABLE();

	exFLASH_WriteReadByte(exFLASH_CMD_READ_STATUS_REG);

	/* flash��æ���ȴ����Ĵ��������һλΪBUSYλ */
	while(exFLASH_WriteReadByte(DUMMY_BYTE) & 0x01);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * ע�⣺��ַ����4KB
 */
void exFLASH_SectorErase(uint32_t sectorAddr)
{
	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* ���������������� */
	exFLASH_WriteReadByte(exFLASH_CMD_SECTOR_ERASE);

	/* ����������ַ����λ��ǰ */
	exFLASH_WriteReadByte(sectorAddr & 0xFF0000 >> 16);
	exFLASH_WriteReadByte(sectorAddr & 0x00FF00 >> 8);
	exFLASH_WriteReadByte(sectorAddr & 0x0000FF);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 * ע�⣺��ַ����64KB
 */
void exFLASH_BlockErase(uint32_t blockAddr)
{
	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* ���������������� */
	exFLASH_WriteReadByte(exFLASH_CMD_BLOCK_ERASE);

	/* ����������ַ����λ��ǰ */
	exFLASH_WriteReadByte(blockAddr & 0xFF0000 >> 16);
	exFLASH_WriteReadByte(blockAddr & 0x00FF00 >> 8);
	exFLASH_WriteReadByte(blockAddr & 0x0000FF);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 *
 */
void exFLASH_ChipErase(void)
{
	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* ���������������� */
	exFLASH_WriteReadByte(exFLASH_CMD_CHIP_ERASE);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 *
 */
static void exFLASH_WritePageBytes(uint32_t writeAddr, uint8_t* pBuffer,
								 uint16_t dataLength)
{
	/* д���ݵĳ��Ȳ��ܴ���flash��ҳ�ֽ� */
	if (dataLength > exFLASH_PAGE_SIZE_BYTES)
		dataLength = exFLASH_PAGE_SIZE_BYTES;

	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* ��������д���� */
	exFLASH_WriteReadByte(exFLASH_CMD_PAGE_PROGRAM);

	/* ����������ַ����λ��ǰ */
	exFLASH_WriteReadByte((writeAddr & 0xFF0000) >> 16);
	exFLASH_WriteReadByte((writeAddr & 0xFF00) >> 8);
	exFLASH_WriteReadByte(writeAddr & 0xFF);

	while (dataLength--)
		exFLASH_WriteReadByte(*pBuffer++);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 *
 */
void exFLASH_WriteBuffer(uint32_t writeAddr, uint8_t* pBuffer, uint16_t dataLength)
{
	uint8_t pageBytesRemainder;				/* ��ҳʣ���д�ֽ��� */
	uint8_t pageWriteNumb;					/* ��Ҫдҳ�� */
	uint8_t dataBytesRemainder;				/* ���һҳ��ʣ�ֽ��� */

	/* д��ַ��flashҳ��ַ���� */
	if (0 == (writeAddr % exFLASH_PAGE_SIZE_BYTES))
	{
		/* �ֽڳ���<=ҳ�ֽڳ��� */
		if (dataLength <=  exFLASH_PAGE_SIZE_BYTES)
			exFLASH_WritePageBytes(writeAddr, pBuffer, dataLength);
		else
		{
			/* ��Ҫд��ҳ�� */
			pageWriteNumb = dataLength / exFLASH_PAGE_SIZE_BYTES;
			/* д��ҳ������д���ֽ��� */
			dataBytesRemainder = dataLength % exFLASH_PAGE_SIZE_BYTES;

			while (pageWriteNumb--)
			{
				exFLASH_WritePageBytes(writeAddr, pBuffer, exFLASH_PAGE_SIZE_BYTES);
				writeAddr += exFLASH_PAGE_SIZE_BYTES;
				pBuffer   += exFLASH_PAGE_SIZE_BYTES;
			}
			exFLASH_WritePageBytes(writeAddr, pBuffer, dataBytesRemainder);
		}
	}
	else
	{
		/* ��ǰҳ��д���ֽ��� */
		pageBytesRemainder = exFLASH_PAGE_SIZE_BYTES - (writeAddr % exFLASH_PAGE_SIZE_BYTES);

		/* ��ǰҳ����д�� */
		if (dataLength <= pageBytesRemainder)
			exFLASH_WritePageBytes(writeAddr, pBuffer, dataLength);
		else
		{
			/* �Ȱѵ�ǰҳд�� */
			exFLASH_WritePageBytes(writeAddr, pBuffer, pageBytesRemainder);
			writeAddr  += pageBytesRemainder;
			pBuffer    += pageBytesRemainder;
			dataLength -= pageBytesRemainder;

			/* ��Ҫд��ҳ�� */
			pageWriteNumb = dataLength / exFLASH_PAGE_SIZE_BYTES;
			while (pageWriteNumb--)
			{
				exFLASH_WritePageBytes(writeAddr, pBuffer, exFLASH_PAGE_SIZE_BYTES);
				writeAddr += exFLASH_PAGE_SIZE_BYTES;
				pBuffer += exFLASH_PAGE_SIZE_BYTES;
			}

			/* д��ҳ������д���ֽ��� */
			dataBytesRemainder = dataLength % exFLASH_PAGE_SIZE_BYTES;
			if (dataBytesRemainder > 0)
				exFLASH_WritePageBytes(writeAddr, pBuffer, dataBytesRemainder);
		}
	}
}


/*******************************************************************************
 *
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
	   exFLASH_WritePageBytes(WriteAddr,pBuffer,  NumByteToWrite);
   }
   else /* NumByteToWrite > SPI_FLASH_PageSize */
   {
			/*�Ȱ�����ҳ��д��*/
     while (NumOfPage--)
     {
    	 exFLASH_WritePageBytes(WriteAddr,pBuffer,  SPI_FLASH_PageSize);
       WriteAddr +=  SPI_FLASH_PageSize;
       pBuffer += SPI_FLASH_PageSize;
     }
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
     exFLASH_WritePageBytes(WriteAddr,pBuffer,  NumOfSingle);
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
       exFLASH_WritePageBytes(WriteAddr,pBuffer,  count);

       WriteAddr +=  count;
       pBuffer += count;
				/*��дʣ�������*/
       exFLASH_WritePageBytes(WriteAddr,pBuffer,  temp);
     }
     else /*��ǰҳʣ���count��λ����д��NumOfSingle������*/
     {
    	 exFLASH_WritePageBytes(WriteAddr,pBuffer,  NumByteToWrite);
     }
   }
   else /* NumByteToWrite > SPI_FLASH_PageSize */
   {
			/*��ַ����������count�ֿ������������������*/
     NumByteToWrite -= count;
     NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
     NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

			/* ��д��count�����ݣ�Ϊ��������һ��Ҫд�ĵ�ַ���� */
     exFLASH_WritePageBytes(WriteAddr,pBuffer,  count);

			/* ���������ظ���ַ�������� */
     WriteAddr +=  count;
     pBuffer += count;
			/*������ҳ��д��*/
     while (NumOfPage--)
     {
    	 exFLASH_WritePageBytes(WriteAddr,pBuffer,  SPI_FLASH_PageSize);
       WriteAddr +=  SPI_FLASH_PageSize;
       pBuffer += SPI_FLASH_PageSize;
     }
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
     if (NumOfSingle != 0)
     {
    	 exFLASH_WritePageBytes(WriteAddr,pBuffer,  NumOfSingle);
     }
   }
 }
}

/*******************************************************************************
 *
 */
void exFLASH_ReadBuffer(uint32_t readAddr, uint8_t* pBuffer, uint16_t dataLength)
{
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* �������������� */
	exFLASH_WriteReadByte(exFLASH_CMD_READ_DATA);

	/* ����������ַ����λ��ǰ */
	exFLASH_WriteReadByte((readAddr & 0xFF0000) >> 16);
	exFLASH_WriteReadByte((readAddr & 0xFF00) >> 8);
	exFLASH_WriteReadByte(readAddr & 0xFF);

	while (dataLength--)
		*pBuffer++ = exFLASH_WriteReadByte(DUMMY_BYTE);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}



/*******************************************************************************
 * ����flash ͣ��
 */
void exFLASH_ModePwrDown(void)
{
	exFLASH_CS_ENABLE();

	exFLASH_WriteReadByte(exFLASH_CMD_POWER_DOWN);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * ����flash����
 */
void exFLASH_ModeWakeUp(void)
{
	exFLASH_CS_ENABLE();

	exFLASH_WriteReadByte(exFLASH_CMD_RELEASE_POWER_DOWN);

	exFLASH_CS_DISABLE();
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
	exFLASH_ReadBuffer(EE_FlashInfoReadAddr, (uint8_t*)info,
			sizeof(exFLASH_InfoTypedef));

	EE_FlashInfoReadAddr += sizeof(exFLASH_InfoTypedef);
	EEPROM_WriteBytes(EE_ADDR_FLASH_INFO_READ_ADDR, &EE_FlashInfoReadAddr,
			sizeof(exFLASH_InfoTypedef));
}
































