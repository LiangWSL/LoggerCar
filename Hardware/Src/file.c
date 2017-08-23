#include "file.h"
#include "print.h"
#include "fatfs.h"
#include "gprs.h"

char FILE_FileName[11];						/* �����ļ��� */
char FILE_PrintFileName[11]; 				/* ��ӡ�ļ��� */
FILE_PatchPackTypedef FILE_PatchPack;		/* �����ļ���Ϣ */
FILE_DeviceParamTypedef FILE_DeviceParam;	/* �豸������Ϣ */

/******************************************************************************/
static void FILE_GetFileNameDependOnTime(FILE_RealTime* time, char* fileName);
static void FILE_GetNextFileName(char* fileName);
static void LocationFormatConvert(double value, uint8_t* pBuffer);
static uint16_t SearchTimeInFile(FILE_RealTime* pTime);
static void selectDataPrint(char* fileName,
							uint16_t startPoint, uint16_t endPoint,
							PRINT_ChannelSelectTypedef* select);
static void AnalogDataFormatConvert(float value, DataFormatEnum format,
							uint8_t* pBuffer);

/*******************************************************************************
 *
 */
void FILE_Init(void)
{
	/* ���ļ�����ӹ̶��ĺ�׺�� */
	memcpy(&FILE_FileName[6], ".txt\0", 5);
	memcpy(&FILE_PrintFileName[6], ".txt\0", 5);
}

/*******************************************************************************
 *
 */
ErrorStatus FILE_SaveInfo(FILE_InfoTypedef* saveInfo, uint16_t* fileStructCount)
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
	FILE_GetFileNameDependOnTime(&saveInfo->realTime, FILE_FileName);

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
	if (ERROR == FATFS_FileWrite((BYTE*)saveInfo, sizeof(FILE_InfoTypedef)))
	{
		printf("�ṹ��д��ʧ��\r\n");
		return ERROR;
	}

	/* ��ȡ�ļ���С */
	*fileStructCount = FATFS_GetFileStructCount();

	if (ERROR == FATFS_FileClose())
		return ERROR;

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 *
 */
ErrorStatus FILE_ReadInfo(FILE_InfoTypedef* readInfo)
{
	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	/* ��ȡ�ļ����ļ����������д�룬�������򴴽�д�� */
	if (ERROR == FATFS_FileOpen(FILE_FileName, FATFS_MODE_OPEN_EXISTING_READ))
	{
		printf("�ļ���ʧ��\r\n");
		return ERROR;
	}

	/* ��дָ��ָ���ļ���ĩβ */
	if (ERROR == FATFS_FileSeekBackwardOnePack())
	{
		printf("ָ���ļ�ĩβʧ��\r\n");
		return ERROR;
	}

	/* �ѽṹ��д���ļ� */
	if (ERROR == FATFS_FileRead((BYTE*)readInfo, sizeof(FILE_InfoTypedef)))
	{
		printf("�ṹ��д��ʧ��\r\n");
		return ERROR;
	}

	if (ERROR == FATFS_FileClose())
		return ERROR;

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function:���ݲ�������Ϣ��ȡ����
 * @patch��������Ϣ
 * @readInfo����������ָ��
 */
uint16_t FILE_ReadPatchInfo(FILE_PatchPackTypedef* patch, FILE_InfoTypedef* readInfo)
{
	uint16_t readInfoCount;

	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return 0;

	/* ��ȡ�ļ����ļ����������д�룬�������򴴽�д�� */
	if (ERROR == FATFS_FileOpen(patch->patchFileName, FATFS_MODE_OPEN_EXISTING_READ))
		return 0;

	/* ����ָ��ָ���ļ���ָ��λ�� */
	if (ERROR == FATFS_FileSeek(patch->patchStructOffset * sizeof(FILE_InfoTypedef)))
		return 0;

	/* ��ǰ�ļ����ж��ٸ��ṹ����Զ� */
	readInfoCount = FATFS_GetFileStructCount() - patch->patchStructOffset;

	/* �ļ��нṹ��������һ�ζ��� */
	if (readInfoCount > GPRS_PATCH_PACK_NUMB_MAX)
	{
		readInfoCount = GPRS_PATCH_PACK_NUMB_MAX;
		patch->patchStructOffset += GPRS_PATCH_PACK_NUMB_MAX;
	}
	/* ��ǰ�ļ�ʣ��Ľṹ���ܹ���һ�ζ��� */
	else
	{
		patch->patchStructOffset = 0;

		/* �Ƚϵ�ǰ�����ļ��Ƿ��ǵ�ǰ�ļ� */
		if (memcmp(patch->patchFileName, FILE_FileName, 6) == 0)
		{
			/* �ǣ���֤��ȫ�����ݲ������,�����ļ���� */
			memcpy(patch->patchFileName, "\0\0\0\0\0\0", 6);
		}
		else
		{
			/* ���򴫵ݵ���һ���ļ� */
			FILE_GetNextFileName(patch->patchFileName);
		}
	}

	if (ERROR == FATFS_FileRead((BYTE*)readInfo, (readInfoCount * sizeof(FILE_InfoTypedef))))
		return 0;

	if (ERROR == FATFS_FileClose())
		return 0;

	FATFS_FileUnlink();

	return readInfoCount;
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
 * function:�豸�����ļ���ʼ��
 */
ErrorStatus FILE_ParamFileInit(void)
{
	/* �����ļ�ϵͳ */
//	if (ERROR == FATFS_FileLink())
//		return ERROR;

	/* ����ļ������� */
//	if (FATFS_FileOpen(PARAM_FILE_NAME, FATFS_MODE_OPEN_EXISTING_READ) == ERROR)
	{
		memcpy(FILE_DeviceParam.deviceSN, "1708151515", sizeof(FILE_DeviceParam.deviceSN));
		FILE_DeviceParam.locationType    = LOCATION_GPS;
		FILE_DeviceParam.firmwareVersion = 10;
		FILE_DeviceParam.recordInterval  = 2;
		FILE_DeviceParam.overLimitRecordInterval = 2;
		FILE_DeviceParam.exitAnalogChannelNumb = ANALOG_CHANNEL_NUMB;
		FILE_DeviceParam.param[0].channelType = TYPE_TEMP;
		FILE_DeviceParam.param[0].channelUnit = UNIT_TEMP;
		FILE_DeviceParam.param[0].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[1].channelType = TYPE_HUMI;
		FILE_DeviceParam.param[1].channelUnit = UNIT_HUMI;
		FILE_DeviceParam.param[1].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[2].channelType = TYPE_TEMP;
		FILE_DeviceParam.param[2].channelUnit = UNIT_TEMP;
		FILE_DeviceParam.param[2].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[3].channelType = TYPE_HUMI;
		FILE_DeviceParam.param[3].channelUnit = UNIT_HUMI;
		FILE_DeviceParam.param[3].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[4].channelType = TYPE_TEMP;
		FILE_DeviceParam.param[4].channelUnit = UNIT_TEMP;
		FILE_DeviceParam.param[4].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[5].channelType = TYPE_HUMI;
		FILE_DeviceParam.param[5].channelUnit = UNIT_HUMI;
		FILE_DeviceParam.param[5].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[6].channelType = TYPE_TEMP;
		FILE_DeviceParam.param[6].channelUnit = UNIT_TEMP;
		FILE_DeviceParam.param[6].dataFormat  = FORMAT_ONE_DECIMAL;
		FILE_DeviceParam.param[7].channelType = TYPE_HUMI;
		FILE_DeviceParam.param[7].channelUnit = UNIT_HUMI;
		FILE_DeviceParam.param[7].dataFormat  = FORMAT_ONE_DECIMAL;

		FILE_DeviceParam.temp1.alarmValueUp       = 40;
		FILE_DeviceParam.temp1.alarmValueLow      = -10;
		FILE_DeviceParam.temp1.perwarningValueUp  = 30;
		FILE_DeviceParam.temp1.perwarningValueLow = -5;
		FILE_DeviceParam.temp2.alarmValueUp       = 40;
		FILE_DeviceParam.temp2.alarmValueLow      = -10;
		FILE_DeviceParam.temp2.perwarningValueUp  = 30;
		FILE_DeviceParam.temp2.perwarningValueLow = -5;
		FILE_DeviceParam.temp3.alarmValueUp       = 40;
		FILE_DeviceParam.temp3.alarmValueLow      = -10;
		FILE_DeviceParam.temp3.perwarningValueUp  = 30;
		FILE_DeviceParam.temp3.perwarningValueLow = -5;
		FILE_DeviceParam.temp4.alarmValueUp       = 40;
		FILE_DeviceParam.temp4.alarmValueLow      = -10;
		FILE_DeviceParam.temp4.perwarningValueUp  = 30;
		FILE_DeviceParam.temp4.perwarningValueLow = -5;

		FATFS_FileOpen(PARAM_FILE_NAME, FATFS_MODE_OPEN_ALWAYS_WRITE);
		FATFS_FileWrite((uint8_t*)&FILE_DeviceParam, sizeof(FILE_DeviceParam));
	}
	/* �򿪳ɹ���˵���ļ����� */
//	else
//	{
//		FATFS_FileRead((uint8_t*)&FILE_DeviceParam, sizeof(FILE_DeviceParam));
//	}

//	if (FATFS_FileClose() == ERROR)
//		return ERROR;

//	FATFS_FileUnlink();

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
	uint16_t startTimeStructOffset;			/* ��ʼ��ӡʱ�䣬�ṹ��ƫ�� */
	uint16_t endTimePoint;					/* ����ʱ��� */
	char stopPrintFileName[6];					/* ֹͣ��ӡ�ļ��� */

	/* ��ȡ��ʼ��ӡʱ���ļ��� */
	/* ע�⣺��ʱ��ʱ����ʮ���Ƶ� */
	HEX2ASCII(&startTime->year, (uint8_t*)FILE_PrintFileName, 3);
	HEX2ASCII(&stopTime->year,  (uint8_t*)stopPrintFileName,  3);

	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (FATFS_FileOpen(FILE_PrintFileName, FATFS_MODE_OPEN_EXISTING_READ) == SUCCESS)
	{
		/* Ѱ�ҿ�ʼʱ��Ľṹ��ƫ���� */
		startTimeStructOffset = SearchTimeInFile(startTime);
	}
	else
	{
		printf("δ�ҵ���Ч�Ŀ�ʼ��ӡʱ���ļ�\r\n");
		FATFS_FileUnlink();
		return ERROR;
	}

	if (FATFS_FileClose() == ERROR)
		return ERROR;

	PRINT_SetMode();
	osDelay(1000);
	/* �ȴ�ӡ���� */
	PRINT_TitleOut();

	/* ��û�д�ӡ��ֹͣʱ���ļ� */
	while (memcmp(FILE_PrintFileName, stopPrintFileName, 6) != 0)
	{
		/* ���ļ�����ӡ���ļ����� */
		selectDataPrint(FILE_PrintFileName, startTimeStructOffset, FILE_PRINT_TO_END, select);
		/* ת������һ���ļ� */
		FILE_GetNextFileName(FILE_PrintFileName);
		/* ��һ���ļ��ṹ��ƫ��Ϊ0 */
		startTimeStructOffset = 0;
	}

	/* ���ɽ�����ӡʱ�� */
	HEX2BCD(&stopTime->hour, (uint8_t*)(&endTimePoint) + 1, 1);
	HEX2BCD(&stopTime->min,  (uint8_t*)(&endTimePoint),     1);
	/* ��ʼ��ӡ */
	selectDataPrint(FILE_PrintFileName, startTimeStructOffset, endTimePoint, select);

	/* ��ӡ���� */
	PRINT_TailOut();

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function:���ݽṹ���ʱ�䣬ת�����ļ���
 */
static void FILE_GetFileNameDependOnTime(FILE_RealTime* time, char* fileName)
{
	/* ��ʱ��ת����ASCII�� */
	BCD2ASCII(fileName, (uint8_t*)time, 3);
}

/*******************************************************************************
 * function������FILE_FileName�õ���һ���ļ���
 */
static void FILE_GetNextFileName(char* fileName)
{
	uint8_t temp1, temp2;

	uint8_t year, month, day;

	str2numb((uint8_t*)&fileName[0], &temp1, 1);
	str2numb((uint8_t*)&fileName[1], &temp2, 1);
	year = temp1 * 10 + temp2;

	str2numb((uint8_t*)&fileName[2], &temp1, 1);
	str2numb((uint8_t*)&fileName[3], &temp2, 1);
	month = temp1 * 10 + temp2;

	str2numb((uint8_t*)&fileName[4], &temp1, 1);
	str2numb((uint8_t*)&fileName[5], &temp2, 1);
	day = temp1 * 10 + temp2;

	/* ʱ����� */
	if((month == 1U) || (month == 3U) || (month == 5U) || (month == 7U) || \
	   (month == 8U) || (month == 10U) || (month == 12U))
	{
		if(day < 31U)
		{
			day++;
		}
		/* Date structure member: day = 31 */
		else
		{
			if(month != 12U)
			{
				month++;
				day = 1U;
			}
			/* Date structure member: day = 31 & month =12 */
			else
			{
				month = 1U;
				day = 1U;
				year++;
			}
		}
	}
	else if((month == 4U) || (month == 6U) || (month == 9U) || (month == 11U))
	{
		if(day < 30U)
		{
			day++;
		}
		/* Date structure member: day = 30 */
		else
		{
			month++;
			day = 1U;
		}
	}
	else if(month == 2U)
	{
		if(day < 28U)
		{
			day++;
		}
		else if(day == 28U)
		{
			/* Leap year */
			if(year % 4 == 0)
			{
				day++;
			}
			else
			{
				month++;
				day = 1U;
			}
		}
		else if(day == 29U)
		{
			month++;
			day = 1U;
		}
	}

	/* ����ת��ASCII */
	HEX2ASCII(&year,  (uint8_t*)&fileName[0], 1);
	HEX2ASCII(&month, (uint8_t*)&fileName[2], 1);
	HEX2ASCII(&day,   (uint8_t*)&fileName[4], 1);
}

/*******************************************************************************
 * function:ģ�������ݸ�ʽת��
 */
static void AnalogDataFormatConvert(float value, DataFormatEnum format,
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
		*pBuffer |= 0x80;
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
		
		/* ������������ṹ��֮��ģ��Ա��ζ���������Ϊ׼ */
		if (searchPoint == fileStructNumbStart)
			break;

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
	}

	return searchPoint;
}

/*******************************************************************************
 * function:����ʱ����ӡͬһ���ļ��ڵ�����,endPointΪ0xffff�����ӡ���ļ���β
 * >>>����GSP�涨����ͨ����5���Ӵ�ӡһ������������2���Ӵ�ӡһ����
 */
static void selectDataPrint(char* fileName,
							uint16_t startPoint, uint16_t endPoint,
							PRINT_ChannelSelectTypedef* select)
{
	FILE_InfoTypedef info;
	BOOL status = FALSE;

	/* ���ļ� */
	FATFS_FileOpen(fileName, FATFS_MODE_OPEN_EXISTING_READ);

	/* �ȴ�ӡ���� */
	PRINT_Date(fileName);

	/* ��ӡ��ʼ�ͽ���ʱ��������� */
	while(startPoint <= (FATFS_GetFileStructCount() - 1))
	{
		FATFS_FileSeek(startPoint * sizeof(FILE_InfoTypedef));
		if (FATFS_FileRead((BYTE*)&info, sizeof(FILE_InfoTypedef)) == SUCCESS)
		{
			/* �жϽ���ʱ�䣬�����������ֹͣ��ӡ */
			if (endPoint < ((info.realTime.hour << 8) | (info.realTime.min)))
				break;
			else
			{
				status = PRINT_DataOut(&info, select);
				if (status)
					startPoint += 2;
				else
					startPoint += 5;
			}
		}
	}
	
	FATFS_FileClose();
}






























