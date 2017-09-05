#include "file.h"
#include "print.h"
#include "fatfs.h"

char FILE_FileName[11];						/* �����ļ��� */
char FILE_PrintFileName[11]; 				/* ��ӡ�ļ��� */
FILE_PatchPackTypedef FILE_PatchPack;		/* �����ļ���Ϣ */

uint64_t dataFileStructCnt;					/* ��ǰ�ļ��ṹ������ */

/******************************************************************************/
//static void FILE_GetFileNameDependOnTime(FILE_RealTime* time, char* fileName);
//static void FILE_GetNextFileName(char* fileName);
//static uint16_t SearchTimeInFile(FILE_RealTime* pTime);
//static void selectDataPrint(char* fileName,
//							uint16_t startPoint, uint16_t endPoint,
//							PRINT_ChannelSelectTypedef* select);
static void SaveInfoFormatConvert(FILE_SaveInfoTypedef* info,
							GPRS_SendInfoTypedef* sendInfo);

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
void FILE_SaveInfoSymbolInit(FILE_SaveInfoTypedef* info)
{
	info->str1   			  = ' ';
	info->str2   			  = ',';
	info->str3  			  = ',';
	info->str4   			  = ',';
	info->str5   			  = ',';
	info->str6   			  = ',';
	info->analogValue[0].str  = ',';
	info->analogValue[1].str  = ',';
	info->analogValue[2].str  = ',';
	info->analogValue[3].str  = ',';
	info->analogValue[4].str  = ',';
	info->analogValue[5].str  = ',';
	info->analogValue[6].str  = ',';
	info->analogValue[7].str  = ',';
	info->end[0] 			  = 0x0D;
	info->end[1] 			  = 0x0A;
}

/*******************************************************************************
 * function:����ṹ�嵽Flash
 * saveInfo������ṹ��ָ��
 * fileStructCount����ǰ�ļ��ṹ����
 */
ErrorStatus FILE_SaveInfo(FILE_SaveInfoTypedef* saveInfo, uint64_t* fileStructCount)
{
	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	/* ��ȡ�ܿռ�Ϳ��пռ� */
	FATFS_GetSpaceInfo();

	/* ����ʱ���ȡ�ļ��� */
//	FILE_GetFileNameDependOnTime(&saveInfo->realTime, FILE_FileName);

	/* ��ȡ�ļ����ļ����������д�룬�������򴴽�д�� */
	if (SUCCESS == FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPEN_ALWAYS_WRITE))
	{
		/* ��дָ��ָ���ļ���ĩβ */
		FATFS_FileSeekEnd();

		/* �ѽṹ��д���ļ� */
		FATFS_FileWrite((BYTE*)saveInfo, sizeof(FILE_SaveInfoTypedef));

		/* ��ȡ�ļ���С */
		*fileStructCount = FATFS_GetFileStructCount();
	}

	FATFS_FileClose();

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function����Flash�ж������һ���ṹ��
 * readInfo����������ṹ��ָ��
 * @patch��������Ϣ
 */
uint16_t FILE_ReadInfo(FILE_SaveInfoTypedef* readInfo, FILE_PatchPackTypedef* patch)
{
	uint16_t readInfoCount;

	/* �����ļ�ϵͳ */
	FATFS_FileLink();

	/* ��ȡ�ļ����ļ����������д�룬�������򴴽�д�� */
	if (SUCCESS == FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPEN_EXISTING_READ))
	{
		if (patch->patchStructOffset == 0)
		{
			readInfoCount = 1;

			/* ָ��ָ�����һ���ṹ�� */
			FATFS_FileSeekBackwardOnePack();

			FATFS_FileRead((BYTE*)readInfo, sizeof(FILE_SaveInfoTypedef));
		}
		else
		{
			FATFS_FileSeek(patch->patchStructOffset * sizeof(FILE_SaveInfoTypedef));

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
			}

			FATFS_FileRead((BYTE*)readInfo, (readInfoCount * sizeof(FILE_SaveInfoTypedef)));
		}

	}

	FATFS_FileClose();

	FATFS_FileUnlink();

	return readInfoCount;
}

/*******************************************************************************
 * function�����ͽṹ���ʽת��
 */
void FILE_SendInfoFormatConvert(uint8_t* saveInfo, uint8_t* sendInfo, uint8_t  sendPackNumb)
{
	uint8_t i;

	for (i = 0; i < sendPackNumb; i++)
	{
		SaveInfoFormatConvert((FILE_SaveInfoTypedef*)saveInfo, (GPRS_SendInfoTypedef*)sendInfo);
		saveInfo += sizeof(FILE_SaveInfoTypedef);
		sendInfo += sizeof(GPRS_SendInfoTypedef);
	}
}

/*******************************************************************************
 * function�����ļ�
 * @offset:��ָ��ƫ����
 * @fileName:�ļ���
 * @pBuffer������ָ��
 * @size:��ȡ����
 */
ErrorStatus FILE_ReadFile(char* fileName, uint64_t offset, BYTE* pBuffer, uint32_t size)
{
	/* �����ļ�ϵͳ */
	if (FATFS_FileLink() == ERROR)
		return ERROR;

	if (FATFS_FileOpen(fileName, FATFS_MODE_OPEN_EXISTING_READ) == SUCCESS)
	{
		FATFS_FileSeek(offset);

		FATFS_FileRead(pBuffer, size);
	}

	FATFS_FileClose();

	FATFS_FileUnlink();

	return SUCCESS;
}

/*******************************************************************************
 * function��д�ļ�
 * @offset:дָ��ƫ����
 * @fileName:�ļ���
 * @pBuffer��д��ָ��
 * @size:д�볤��
 */
ErrorStatus FILE_WriteFile(char* fileName, uint64_t offset, BYTE* pBuffer, uint32_t size)
{
	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	if (FATFS_FileOpen(fileName, FATFS_MODE_OPEN_ALWAYS_WRITE) == SUCCESS)
	{
		FATFS_FileSeek(offset);

		FATFS_FileWrite(pBuffer, size);
	}

	FATFS_FileClose();

	FATFS_FileUnlink();

	return SUCCESS;
}

#if 0
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
#endif

/*******************************************************************************
 * function:ģ�������ݸ�ʽת��
 */
static void AnalogDataFormatConvert(char* analog, DataFormatEnum format, uint8_t* pBuffer)
{
	char str[6];
	float value;
	BOOL negative = FALSE;
	uint16_t temp = 0;

	/* ���ַ���תΪfloat */
	memcpy(str, analog, 5);
	str[5] = '\0';
	value = (float)atof(str);

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
 * function�����ַ����͵Ķ�λֵת����Э���ʽ
 */
static void LocationFormatConvert(char* lacation, uint8_t* pBuffer)
{
	char str[11];
	double value;
	BOOL negative = FALSE;
	uint32_t temp;

	/* ���ַ���תΪdouble */
	memcpy(str, lacation, 10);
	str[10] = '\0';
	value = atof(str);

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
 * funtion:������ĸ�ʽ��ASCII�룩ת����Э��涨�ĸ�ʽ
 */
static void SaveInfoFormatConvert(FILE_SaveInfoTypedef* info,
							GPRS_SendInfoTypedef* sendInfo)
{
	char str[4];

	/* �ṹ�帴λ���������ݳ��� */
	memset(sendInfo, 0, sizeof(GPRS_SendInfoTypedef));

	/* ʱ���ַ���ת����BCD */
	ASCII2BCD(info->year,  &sendInfo->year,  2);
	ASCII2BCD(info->month, &sendInfo->month, 2);
	ASCII2BCD(info->day,   &sendInfo->day,   2);
	ASCII2BCD(info->hour,  &sendInfo->hour,  2);
	ASCII2BCD(info->min,   &sendInfo->min,   2);
	ASCII2BCD(info->sec,   &sendInfo->sec,   2);

	/* ת����ص��� */
	memcpy(str, info->batQuality, 3);
	str[3] = '\0';
	sendInfo->batteryLevel = atoi(str);

	/* ת���ⲿ��Դ״̬ */
	str2numb((uint8_t*)&info->exPwrStatus, &sendInfo->externalPowerStatus, 1);

	/* ת������ */
	LocationFormatConvert(info->longitude, (uint8_t*)&sendInfo->longitude);
	LocationFormatConvert(info->latitude,  (uint8_t*)&sendInfo->latitude);

	AnalogDataFormatConvert(info->analogValue[0].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[0]);
	AnalogDataFormatConvert(info->analogValue[1].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[1]);
	AnalogDataFormatConvert(info->analogValue[2].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[2]);
	AnalogDataFormatConvert(info->analogValue[3].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[3]);
	AnalogDataFormatConvert(info->analogValue[4].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[4]);
	AnalogDataFormatConvert(info->analogValue[5].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[5]);
	AnalogDataFormatConvert(info->analogValue[6].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[6]);
	AnalogDataFormatConvert(info->analogValue[7].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[7]);
}

#if 0
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

#endif




























