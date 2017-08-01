#include "file.h"

char FILE_FileName[10];
FILE_PatchPackTypedef FILE_PatchPack;

/******************************************************************************/
static ErrorStatus FILE_SaveInfo(FILE_InfoTypedef* info);
static ErrorStatus FILE_ReadInfo(FILE_InfoTypedef* info, uint8_t count);
static void FILE_GetFileNameDependOnTime(FILE_RealTime* time);
static void AnalogDataFormatConvert(float value, EE_DataFormatEnum format,
							uint8_t* pBuffer);
static void LocationFormatConvert(double value, uint8_t* pBuffer);


/*******************************************************************************
 *
 */
void FILE_Init(void)
{
	/* ���ļ�����ӹ̶��ĺ�׺�� */
	memcpy(&FILE_FileName[6], ".txt", 4);
}

/*******************************************************************************
 *
 */
ErrorStatus FILE_SaveReadInfo(FILE_InfoTypedef* saveInfo,
		FILE_InfoTypedef* readInfo, uint8_t readInfoCount)
{
	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	/* ��ȡ�ܿռ�Ϳ��пռ� */
	if (ERROR == FATFS_GetSpaceInfo())
	{
		printf("���ݿռ��ȡʧ��\r\n");
	}

	/* ����ʱ���ȡ�ļ��� */
	FILE_GetFileNameDependOnTime(&saveInfo->realTime);

	if (ERROR == FILE_SaveInfo(saveInfo))
	{
		/* ��������� */
		/* todo */
		printf("���ݴ���ʧ��\r\n");
	}

	if (ERROR == FILE_ReadInfo(readInfo, readInfoCount))
	{
		/* ��ȡ���������� */
		/* todo */
		printf("���ݶ�ȡʧ��\r\n");
	}

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 *
 */
void FILE_InfoFormatConvert(FILE_InfoTypedef*    saveInfo,
							RT_TimeTypedef*      realTime,
							GPS_LocateTypedef*   location,
							ANALOG_ValueTypedef* analogValue)
{
	/* �ṹ�帴λ���������ݳ��� */
	memset(saveInfo, 0, sizeof(FILE_InfoTypedef));

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
//	saveInfo->externalPowerStatus = INPUT_CheckPwrOnStatus();

	LocationFormatConvert(location->latitude,  (uint8_t*)&saveInfo->latitude);
	LocationFormatConvert(location->longitude, (uint8_t*)&saveInfo->longitude);

	/* ģ�����ݸ�ʽת�� */
	AnalogDataFormatConvert(analogValue->temp1, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp1);
	AnalogDataFormatConvert(analogValue->humi1, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi1);
	AnalogDataFormatConvert(analogValue->temp2, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp2);
	AnalogDataFormatConvert(analogValue->humi2, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi2);
	AnalogDataFormatConvert(analogValue->temp3, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp3);
	AnalogDataFormatConvert(analogValue->humi3, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi3);
	AnalogDataFormatConvert(analogValue->temp4, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp4);
	AnalogDataFormatConvert(analogValue->humi4, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi4);
}

/*******************************************************************************
 *
 */
static ErrorStatus FILE_SaveInfo(FILE_InfoTypedef* info)
{
	/* ��ȡ�ļ����ļ����������д�룬�������򴴽�д�� */
	if (ERROR == FATFS_FileOpen(FILE_FileName, FATFS_MODE_OPEN_ALWAYS_WRITE))
	{
		printf("�ļ���ʧ��\r\n");
		return ERROR;
	}

	/* ��дָ��ָ���ļ���ĩβ */
	if (ERROR == FATFS_FileSeekEnd())
	{
		printf("ָ���ļ�ĩβʧ��\r\n");
		return ERROR;
	}

	/* �ѽṹ��д���ļ� */
	if (ERROR == FATFS_FileWrite((BYTE*)info, sizeof(FILE_InfoTypedef)))
	{
		printf("�ṹ��д��ʧ��\r\n");
		return ERROR;
	}

	if (ERROR == FATFS_FileClose())
		return ERROR;

	return SUCCESS;
}

/*******************************************************************************
 *
 */
static ErrorStatus FILE_ReadInfo(FILE_InfoTypedef* info, uint8_t count)
{
	WORD byteToRead;

	/* ��ȡ�ļ����ļ����������д�룬�������򴴽�д�� */
	if (FATFS_FileOpen(FILE_FileName, FATFS_MODE_OPEN_EXISTING_READ) == ERROR)
		return ERROR;

	byteToRead = count * sizeof(FILE_InfoTypedef);

	/* ��дָ��ָ���ļ���ĩβ */
	if (FATFS_FileSeekBackward(byteToRead) == ERROR)
		return ERROR;

	/* �ѽṹ��д���ļ� */
	if (FATFS_FileRead((BYTE*)info, byteToRead) == ERROR)
		return ERROR;

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	return SUCCESS;
}

/*******************************************************************************
 * function:���ݽṹ���ʱ�䣬ת�����ļ���
 */
static void FILE_GetFileNameDependOnTime(FILE_RealTime* time)
{
	/* ��ʱ��ת����ASCII�� */
	BCD2ASCII(FILE_FileName, (uint8_t*)time, 3);
}

/*******************************************************************************
 * function:ģ�������ݸ�ʽת��
 */
static void AnalogDataFormatConvert(float value, EE_DataFormatEnum format,
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
static void LocationFormatConvert(double value, uint8_t* pBuffer)
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





































