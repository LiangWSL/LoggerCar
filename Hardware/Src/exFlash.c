#include "exFlash.h"

#include "input.h"


/*******************************************************************************
 * ����flashдʹ��ָ��
 */
static void exFLASH_WriteEnable(void)
{
	uint8_t enableCmd = exFLASH_CMD_WRITE_ENABLE;

	exFLASH_CS_ENABLE();

	HAL_SPI_Transmit(&exFLASH_SPI, &enableCmd, 1, 100);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * FLASH оƬ���ڲ��洢����д��������Ҫ����һ����ʱ�䣬������������ͨѶ������һ˲����ɵģ�������д����
 * ����Ҫȷ��FLASHоƬ�����С�ʱ�����ٴ�д��
 */
static void exFLASH_WaitForIdle(void)
{
	uint8_t readRegStatus = exFLASH_CMD_READ_STATUS_REG;
	uint8_t sendByte = DUMMY_BYTE;
	uint8_t readByte = 0x01;

	exFLASH_CS_ENABLE();

	HAL_SPI_Transmit(&exFLASH_SPI, &readRegStatus, 1, 100);

	/* flash��æ���ȴ����Ĵ��������һλΪBUSYλ */
	while(readByte & 0x01)
	{
		HAL_SPI_TransmitReceive(&exFLASH_SPI, &sendByte, &readByte, 1, 100);
	}

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 *
 */
static void exFLASH_SendCmdAddr(uint8_t cmd, uint32_t addr)
{
	/* ������������ָ���������ַ����λ��ǰ�� */
	uint8_t sendBytes[4] = {cmd,
							(addr & 0xFF0000 >> 16),
							(addr & 0x00FF00 >> 8),
							(addr & 0x0000FF)};

	HAL_SPI_Transmit(&exFLASH_SPI, sendBytes, sizeof(sendBytes), 100);
}

/*******************************************************************************
 * ע�⣺��ַ����4KB
 */
void exFLASH_SectorErase(uint32_t sectorAddr)
{
	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* ����������������͵�ַ */
	exFLASH_SendCmdAddr(exFLASH_CMD_SECTOR_ERASE, sectorAddr);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 *
 */
void exFLASH_ChipErase(void)
{
	/* ������������ָ���������ַ����λ��ǰ�� */
	uint8_t chipEraseCmd = exFLASH_CMD_CHIP_ERASE;

	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	HAL_SPI_Transmit(&exFLASH_SPI, &chipEraseCmd, 1, 100);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/******************************************************************************/
uint32_t exFLASH_ReadDeviceID(void)
{
	uint8_t temp[4] = {exFLASH_CMD_JEDEC_DIVICE_ID,
					   DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE};

	uint32_t deviceID;

//	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	HAL_SPI_TransmitReceive(&exFLASH_SPI, temp, temp, sizeof(temp), 1000);

	exFLASH_CS_DISABLE();

	deviceID = (temp[1] << 16) | (temp[2] << 8) | temp[3];
	return deviceID;
}

/*******************************************************************************
 *
 */
static void exFLASH_WritePageBytes(uint32_t writeAddr, uint8_t* pBuffer,
								 uint16_t dataLength)
{
	/* д���ݵĳ��Ȳ��ܴ���flash��ҳ�ֽ� */
	if (dataLength > exFLASH_PAGE_SIZE_BYTES)
		return;

	exFLASH_WriteEnable();

	exFLASH_CS_ENABLE();

	/* ��������͵�ַ */
	exFLASH_SendCmdAddr(exFLASH_CMD_PAGE_PROGRAM, writeAddr);

	/* �������� */
	HAL_SPI_Transmit(&exFLASH_SPI, pBuffer, dataLength, 100);

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
				pBuffer += exFLASH_PAGE_SIZE_BYTES;
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
			writeAddr += pageBytesRemainder;
			pBuffer += pageBytesRemainder;
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
void exFLASH_ReadBuffer(uint32_t readAddr, uint8_t* pBuffer, uint16_t dataLength)
{
	exFLASH_CS_ENABLE();

	exFLASH_SendCmdAddr(exFLASH_CMD_READ_DATA, readAddr);

	HAL_SPI_Receive(&exFLASH_SPI, pBuffer, dataLength, 100);

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
 * ����flash ͣ��
 */
void exFLASH_ModePwrDown(void)
{
	uint8_t modePwrDown = exFLASH_CMD_POWER_DOWN;

	exFLASH_CS_ENABLE();

	HAL_SPI_Transmit(&exFLASH_SPI, &modePwrDown, 1, 100);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * ����flash����
 */
void exFLASH_ModeWakeUp(void)
{
	uint8_t modeWakeUp = exFLASH_CMD_RELEASE_POWER_DOWN;

	exFLASH_CS_ENABLE();

	HAL_SPI_Transmit(&exFLASH_SPI, &modeWakeUp, 1, 100);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 *
 */
void exFLASH_SaveStructInfo(exFLASH_InfoTypedef* saveInfo,
							RT_TimeTypedef* realTime,
							ANALOG_ValueTypedef* analogValue,
							EE_DataFormatEnum format)
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
	saveInfo->externalPowerStatus = INPUT_CheckPwrOnStatus();;

	/* ģ�����ݸ�ʽת�� */
	exFLASH_DataFormatConvert(analogValue->temp1, format,
			(uint8_t*)&saveInfo->analogValue.temp1);
	exFLASH_DataFormatConvert(analogValue->humi1, format,
			(uint8_t*)&saveInfo->analogValue.humi1);
	exFLASH_DataFormatConvert(analogValue->temp2, format,
			(uint8_t*)&saveInfo->analogValue.temp2);
	exFLASH_DataFormatConvert(analogValue->humi2, format,
			(uint8_t*)&saveInfo->analogValue.humi2);
	exFLASH_DataFormatConvert(analogValue->temp3, format,
			(uint8_t*)&saveInfo->analogValue.temp3);
	exFLASH_DataFormatConvert(analogValue->humi3, format,
			(uint8_t*)&saveInfo->analogValue.humi3);
	exFLASH_DataFormatConvert(analogValue->temp4, format,
			(uint8_t*)&saveInfo->analogValue.temp4);
	exFLASH_DataFormatConvert(analogValue->humi4, format,
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
































