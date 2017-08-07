#include "file.h"
#include "print.h"
#include "fatfs.h"

char FILE_FileName[11];
FILE_PatchPackTypedef FILE_PatchPack;

/******************************************************************************/
static ErrorStatus FILE_SaveInfo(FILE_InfoTypedef* info);
static ErrorStatus FILE_ReadInfo(FILE_InfoTypedef* info, uint8_t count);
static void FILE_GetFileNameDependOnTime(FILE_RealTime* time);
static void AnalogDataFormatConvert(float value, EE_DataFormatEnum format,
							uint8_t* pBuffer);
static void LocationFormatConvert(double value, uint8_t* pBuffer);
static uint16_t SearchTimeInFile(FILE_RealTime* pTime);
static void selectDataPrint(uint16_t startPoint, uint16_t endPoint, PRINT_ChannelSelectTypedef* select);


/*******************************************************************************
 *
 */
void FILE_Init(void)
{
	/* ���ļ�����ӹ̶��ĺ�׺�� */
	memcpy(&FILE_FileName[6], ".txt\0", 5);

	/* ���������ļ� */
	FATFS_MakeFile(PATCH_PACK_FILE_NAME);
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
 * function���������ļ�patch.txt
 * pBuffer������ָ��
 */
ErrorStatus FILE_ReadPatchPackFile(FILE_PatchPackTypedef* pBuffer)
{
	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (FATFS_FileOpen(PATCH_PACK_FILE_NAME, FATFS_MODE_OPEN_EXISTING_READ) == ERROR)
		return ERROR;

	/* �ѽṹ��д���ļ� */
	if (FATFS_FileRead((BYTE*)pBuffer, sizeof(FILE_PatchPackTypedef)) == ERROR)
		return ERROR;

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function��д�����ļ�patch.txt
 * pBuffer��д��ָ��
 */
ErrorStatus FILE_WritePatchPackFile(FILE_PatchPackTypedef* pBuffer)
{
	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (FATFS_FileOpen(PATCH_PACK_FILE_NAME, FATFS_MODE_OPEN_ALWAYS_WRITE) == ERROR)
		return ERROR;

	/* �ѽṹ��д���ļ� */
	if (FATFS_FileWrite((BYTE*)pBuffer, sizeof(FILE_PatchPackTypedef)) == ERROR)
		return ERROR;

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function:Ѱ���ļ��и�ʱ�������ݣ����ظ��������ڽṹ���ַ
 * @time��ʱ��ָ�룬ע�⣺��ʱ����ʮ����format
 *
 */
ErrorStatus FILE_PrintDependOnTime(FILE_RealTime* startTime, FILE_RealTime* stopTime,
		PRINT_ChannelSelectTypedef* select)
{
	uint16_t startTimePoint, endTimePoint;

	/* ��ȡ��ʼ��ӡʱ���ļ��� */
	/* ע�⣺��ʱ��ʱ����ʮ���Ƶ� */
	HEX2ASCII(&startTime->year, (uint8_t*)FILE_FileName, 3);

	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (FATFS_FileOpen(FILE_FileName, FATFS_MODE_OPEN_EXISTING_READ) == SUCCESS)
	{
		/* Ѱ�ҿ�ʼʱ��Ľṹ��ƫ���� */
		startTimePoint = SearchTimeInFile(startTime);
	}
	else
		printf("δ�ҵ���Ч�Ŀ�ʼ��ӡʱ���ļ�\r\n");

	/* ��ʼ��ӡʱ��ͽ�����ӡʱ����ͬһ�� */
	if ((startTime->year == stopTime->year) && (startTime->month == stopTime->month)
			&& (startTime->day == stopTime->day))
	{
		/* Ѱ�ҽ���ʱ��Ľṹ��ƫ���� */
		endTimePoint   = SearchTimeInFile(stopTime);

		/* ��ʼ��ӡ */
		selectDataPrint(startTimePoint, endTimePoint, select);
	}
	/* ��ʼ��ӡ�ͽ�����ӡʱ���ǿ���� */
	else
	{
		/* todo */
//		/* �ȹرտ�ʼ��ӡʱ�����ڵ��ļ� */
//		FATFS_FileClose();
//
//		/* ��ȡ������ӡʱ���ļ��� */
//		sprintf(FILE_FileName[0], "%2d", stopTime->year);
//		sprintf(FILE_FileName[2], "%2d", stopTime->month);
//		sprintf(FILE_FileName[4], "%2d", stopTime->day);
//
//		if (FATFS_FileOpen(FILE_FileName, FATFS_MODE_OPEN_EXISTING_READ) == SUCCESS)
//		{
//			/* Ѱ�ҽ���ʱ��Ľṹ��ƫ���� */
//			endTimePoint   = SearchTimeInFile(stopTime);
//		}

	}

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	FATFS_FileUnlink();
	
	return SUCCESS;
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

/*******************************************************************************
 * function:���ļ���Ѱ��ʱ���
 * @pTime:ҪѰ�ҵ�ʱ���
 */
static uint16_t SearchTimeInFile(FILE_RealTime* pTime)
{
	uint16_t sourceTime, destTime;		/* Դʱ��㣬��Ŀ��ʱ��� */
	uint16_t fileStructNumbStart, fileStructNumbEnd;	/* �ļ��нṹ����� */
	uint16_t searchPoint;
	FILE_InfoTypedef info;

	/* �ҵ��ļ��У�����ת��ΪBCD */
	/* Ѱ��ʱ��㣬ֻ�����ʱ����������ȥ���� */
	/* ע�⣺stm32�ڲ�����С��ģʽ */
	HEX2BCD(&pTime->hour, (uint8_t*)(&destTime) + 1, 1);
	HEX2BCD(&pTime->min,  (uint8_t*)(&destTime),     1);

	fileStructNumbStart = 0;
	/* ��ȡ���ݣ�������һ���ṹ�����ʼ��ַ��ʼ�� */
	fileStructNumbEnd = FATFS_GetFileStructCount();

	while (1)
	{
		searchPoint = (fileStructNumbEnd + fileStructNumbStart) / 2;

		FATFS_FileSeek(searchPoint * sizeof(FILE_InfoTypedef));
		if (FATFS_FileRead((BYTE*)&info, sizeof(FILE_InfoTypedef)) == SUCCESS)
		{
			/* �ṹ���л�ȡʱ����� */
			sourceTime = (info.realTime.hour << 8) | (info.realTime.min);

			if (sourceTime == destTime)
				break;
			else if (sourceTime > destTime)
				fileStructNumbEnd = searchPoint;
			else
				fileStructNumbStart = searchPoint;
		}

		/* ������������ṹ��֮��ģ��Ա��ζ���������Ϊ׼ */
		if (searchPoint == fileStructNumbStart)
			break;
	}

	return searchPoint;
}

/*******************************************************************************
 * function:����ʱ����ӡͬһ���ļ��ڵ�����
 */
static void selectDataPrint(uint16_t startPoint, uint16_t endPoint, PRINT_ChannelSelectTypedef* select)
{
	FILE_InfoTypedef info;

	PRINT_PWR_ENABLE();
	
	/* ��ӡ��ʼ�ͽ���ʱ��������� */
	while(startPoint <= endPoint)
	{
		FATFS_FileSeek(startPoint * sizeof(FILE_InfoTypedef));
		if (FATFS_FileRead((BYTE*)&info, sizeof(FILE_InfoTypedef)) == SUCCESS)
		{
			PRINT_DataOut(&info, select);
			startPoint++;
		}
	}
	
//	PRINT_PWR_DISABLE();
}

































