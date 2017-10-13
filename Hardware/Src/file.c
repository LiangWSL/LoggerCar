#include "file.h"
#include "print.h"
#include "fatfs.h"

/******************************************************************************/
FILE_PatchPackTypedef FILE_PatchPack;		/* �����ļ���Ϣ */
uint64_t FILE_DataSaveStructCnt;			/* ���ݴ����ļ��ṹ������ */


FILE_SaveStructTypedef FILE_SaveStruct;			/* ������Ϣд��ṹ�� */
FILE_SaveStructTypedef FILE_ReadStruct[GPRS_PATCH_PACK_NUMB_MAX];
												/* ������Ϣ��ȡ�ṹ�� */

/******************************************************************************/
static void Data2SaveFormat(RT_TimeTypedef* time, GPS_LocateTypedef* location);
static void SendInfoFormatConvert(uint8_t* sendInfo,
							      uint8_t* readInfo,
							      uint8_t  sendPackNumb);
static void SaveStructSymbolInit(void);

/*******************************************************************************
 * function���ļ�ϵͳ��ʼ������ȡ���ݴ����ļ��Ľṹ������
 */
void FILE_Init(void)
{
	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return;

	/* ���ļ���������������ȴ����ļ���ȷ���豸���ڱ�Ҫ���ļ� */
	FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPNE_ALWAYS);
	FATFS_FileClose();
	FATFS_FileOpen(FILE_NAME_PATCH_PACK, FATFS_MODE_OPNE_ALWAYS);
	FATFS_FileClose();
	FATFS_FileOpen(FILE_NAME_PARAM, FATFS_MODE_OPNE_ALWAYS);
	FATFS_FileClose();

	/* �������ļ��Ľṹ���С */
	if (SUCCESS == FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPEN_EXISTING_READ))
	{
		FILE_DataSaveStructCnt = FATFS_GetFileStructCount();
		PARAM_DeviceParam.deviceCapacity = FATFS_GetSpaceInfo();
	}

	FATFS_FileClose();
	FATFS_FileUnlink();

	/* �������ݽṹ��ʼ�� */
	SaveStructSymbolInit();
}

/*******************************************************************************
 * function:����ṹ�嵽Flash
 * saveInfo������ṹ��ָ��
 * fileStructCount����ǰ�ļ��ṹ����
 */
ErrorStatus FILE_SaveInfo(RT_TimeTypedef* time, GPS_LocateTypedef* location)
{
	/* ����ת����ASCII�����ʽ */
	Data2SaveFormat(time, location);

	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return ERROR;

	/* ��ȡ�ļ����ļ����������д�룬�������򴴽�д�� */
	if (SUCCESS == FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPEN_ALWAYS_WRITE))
	{
		/* ���ж�д���ַ�Ƿ���룬�����д�룬�������򸲸ǲ���������� */
		FATFS_FileSeekSaveInfoStructAlign();

		/* �ѽṹ��д���ļ� */
		FATFS_FileWrite((BYTE*)&FILE_SaveStruct, sizeof(FILE_SaveStructTypedef));

		/* ��ȡ�ļ���С */
		FILE_DataSaveStructCnt = FATFS_GetFileStructCount();
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
void FILE_ReadInfo(GPRS_SendBufferTypedef* sendBuffer,
				   FILE_PatchPackTypedef*  patch)
{
	uint8_t readInfoCount;

	/* �����ļ�ϵͳ */
	if (ERROR == FATFS_FileLink())
		return;

	/* �����ݴ����ļ� */
	if (SUCCESS == FATFS_FileOpen(FILE_NAME_SAVE_DATA, FATFS_MODE_OPEN_EXISTING_READ))
	{
		/* û�в������� */
		if (patch->patchStructOffset == 0)
		{
			/* ��ȡ����Ϊ1�� */
			readInfoCount = 1;
			/* ָ��ָ�����һ���ṹ�� */
			FATFS_FileSeekBackwardOnePack();
		}
		else
		{
			/* �в������� */
			FATFS_FileSeek(patch->patchStructOffset
						   * sizeof(FILE_SaveStructTypedef));

			/* ��ǰ�ļ����ж��ٸ��ṹ����Զ� */
			readInfoCount = FATFS_GetFileStructCount() - patch->patchStructOffset;

			/* �ļ��нṹ��������һ�ζ��� */
			if (readInfoCount > GPRS_PATCH_PACK_NUMB_MAX)
			{
				/* ����ϴ����������� */
				readInfoCount = GPRS_PATCH_PACK_NUMB_MAX;
				patch->patchStructOffset += GPRS_PATCH_PACK_NUMB_MAX;
			}
			/* ��ǰ�ļ�ʣ��Ľṹ���ܹ���һ�ζ��� */
			else
			{
				/* ����������� */
				patch->patchStructOffset = 0;
			}
		}
		FATFS_FileRead((BYTE*)FILE_ReadStruct,
							(readInfoCount * sizeof(FILE_SaveStructTypedef)));
	}

	FATFS_FileClose();
	FATFS_FileUnlink();

	/* ��ȡ���͵����ݰ��� */
	sendBuffer->dataPackNumbL = HALFWORD_BYTE_L(readInfoCount);
	/* �����������ݸ�ʽת����Э���ʽ */
	SendInfoFormatConvert((uint8_t*)&sendBuffer->dataPack,
						  (uint8_t*)FILE_ReadStruct,
						  readInfoCount);
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
		if (offset % sizeof(FILE_SaveStructTypedef) == 0)
		{
			FATFS_FileSeek(offset);
			FATFS_FileRead(pBuffer, size);
		}
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

/*******************************************************************************
 * @brief �������ASCII��ת����float
 * @retval ģ����ASCII��
 */
float FILE_Analog2Float(SaveInfoAnalogTypedef* value)
{
	char str[6];

	memcpy(str, value->value, 5);
	str[5] = '\0';
	return (float)atof(str);
}

/*******************************************************************************
 * @brief ������ת���ɴ����ʽ
 */
static void Data2SaveFormat(RT_TimeTypedef* time, GPS_LocateTypedef* location)
{
	/* ʱ��ת����ASCII */
	HEX2ASCII((uint8_t*)&FILE_SaveStruct.year[0],  &time->date.Year,    1);
	HEX2ASCII((uint8_t*)&FILE_SaveStruct.month[0], &time->date.Month,   1);
	HEX2ASCII((uint8_t*)&FILE_SaveStruct.day[0],   &time->date.Date,    1);
	HEX2ASCII((uint8_t*)&FILE_SaveStruct.hour[0],  &time->time.Hours,   1);
	HEX2ASCII((uint8_t*)&FILE_SaveStruct.min[0],   &time->time.Minutes, 1);
	HEX2ASCII((uint8_t*)&FILE_SaveStruct.sec[0],   &time->time.Seconds, 1);

	/* ��ȡ�ⲿ��Դ״̬ */
	FILE_SaveStruct.exPwrStatus = INPUT_CheckPwrOnStatus() + '0';

	/* ģ����ת��ΪASCII */
	sprintf((char*)&FILE_SaveStruct.analogValue[0].value, "%5.1f", ANALOG_value.temp1);
	sprintf((char*)&FILE_SaveStruct.analogValue[1].value, "%5.1f", ANALOG_value.humi1);
	sprintf((char*)&FILE_SaveStruct.analogValue[2].value, "%5.1f", ANALOG_value.temp2);
	sprintf((char*)&FILE_SaveStruct.analogValue[3].value, "%5.1f", ANALOG_value.humi2);
	sprintf((char*)&FILE_SaveStruct.analogValue[4].value, "%5.1f", ANALOG_value.temp3);
	sprintf((char*)&FILE_SaveStruct.analogValue[5].value, "%5.1f", ANALOG_value.humi3);
	sprintf((char*)&FILE_SaveStruct.analogValue[6].value, "%5.1f", ANALOG_value.temp4);
	sprintf((char*)&FILE_SaveStruct.analogValue[7].value, "%5.1f", ANALOG_value.humi4);
	sprintf((char*)&FILE_SaveStruct.batQuality[0],        "%3d",   ANALOG_value.batVoltage);

	/* ��λֵת����ASCII */
	sprintf((char*)&FILE_SaveStruct.longitude[0], "%10.5f", location->longitude);
	sprintf((char*)&FILE_SaveStruct.latitude[0],  "%10.5f",  location->latitude);

	FILE_SaveStruct.batQuality[3]	   = '%';		/* ��ص����ٷֺ� */
	FILE_SaveStruct.str7   			   = ',';
	FILE_SaveStruct.str8   			   = ',';
	FILE_SaveStruct.analogValue[0].str = ',';
	FILE_SaveStruct.analogValue[1].str = ',';
	FILE_SaveStruct.analogValue[2].str = ',';
	FILE_SaveStruct.analogValue[3].str = ',';
	FILE_SaveStruct.analogValue[4].str = ',';
	FILE_SaveStruct.analogValue[5].str = ',';
	FILE_SaveStruct.analogValue[6].str = ',';
	FILE_SaveStruct.analogValue[7].str = ',';
}

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

	*pBuffer = HALFWORD_BYTE_H(temp);
	*(pBuffer + 1) = HALFWORD_BYTE_L(temp);

	/* ���������λ��һ */
	if (negative)
		*pBuffer |= 0x80;
}

/*******************************************************************************
 * @brief ���ַ����͵Ķ�λֵת����Э���ʽ
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
 * @brief ������ĸ�ʽ��ASCII�룩ת����Э��涨�ĸ�ʽ
 */
static void FormatConvert(GPRS_SendInfoTypedef*   sendInfo,
						  FILE_SaveStructTypedef* readInfo)
{
	char str[4];

	/* �ṹ�帴λ���������ݳ��� */
	memset(sendInfo, 0, sizeof(GPRS_SendInfoTypedef));

	/* ʱ���ַ���ת����BCD */
	ASCII2BCD(&sendInfo->year,  readInfo->year,  2);
	ASCII2BCD(&sendInfo->month, readInfo->month, 2);
	ASCII2BCD(&sendInfo->day,   readInfo->day,   2);
	ASCII2BCD(&sendInfo->hour,  readInfo->hour,  2);
	ASCII2BCD(&sendInfo->min,   readInfo->min,   2);
	ASCII2BCD(&sendInfo->sec,   readInfo->sec,   2);

	/* ת����ص��� */
	memcpy(str, readInfo->batQuality, 3);
	str[3] = '\0';
	sendInfo->batteryLevel = atoi(str);

	/* ת���ⲿ��Դ״̬ */
	str2numb(&sendInfo->externalPowerStatus, (uint8_t*)&readInfo->exPwrStatus, 1);

	/* ת������ */
	LocationFormatConvert(readInfo->longitude, (uint8_t*)&sendInfo->longitude);
	LocationFormatConvert(readInfo->latitude,  (uint8_t*)&sendInfo->latitude);

	AnalogDataFormatConvert(readInfo->analogValue[0].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[0]);
	AnalogDataFormatConvert(readInfo->analogValue[1].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[1]);
	AnalogDataFormatConvert(readInfo->analogValue[2].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[2]);
	AnalogDataFormatConvert(readInfo->analogValue[3].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[3]);
	AnalogDataFormatConvert(readInfo->analogValue[4].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[4]);
	AnalogDataFormatConvert(readInfo->analogValue[5].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[5]);
	AnalogDataFormatConvert(readInfo->analogValue[6].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[6]);
	AnalogDataFormatConvert(readInfo->analogValue[7].value, ANALOG_VALUE_FORMAT,
			(uint8_t*)&sendInfo->analogValue[7]);
}

/*******************************************************************************
 * @brief ������������ת���ɷ��͵ĸ�ʽ
 * @param sendInfo���������ݽṹ��ָ��
 * @param readInfo����ȡ���ݵĽṹ��ָ��
 * @param sendPackNumb����Ҫת���İ���
 */
static void SendInfoFormatConvert(uint8_t* sendInfo,
						   uint8_t* readInfo,
						   uint8_t  sendPackNumb)
{
	uint8_t i;

	for (i = 0; i < sendPackNumb; i++)
	{
		FormatConvert((GPRS_SendInfoTypedef*)sendInfo, (FILE_SaveStructTypedef*)readInfo);
		readInfo += sizeof(FILE_SaveStructTypedef);
		sendInfo += sizeof(GPRS_SendInfoTypedef);
	}
}

/*******************************************************************************
 * @brief ���ݴ���ṹ������ӷ��ţ�ʹ������Excel�����
 */
static void SaveStructSymbolInit(void)
{
	FILE_SaveStruct.str1   = ' ';
	FILE_SaveStruct.str2   = ':';
	FILE_SaveStruct.str3   = ':';
	FILE_SaveStruct.str4   = ',';
	FILE_SaveStruct.str5   = ',';
	FILE_SaveStruct.str6   = ',';
	FILE_SaveStruct.end[0] = 0x0D;
	FILE_SaveStruct.end[1] = 0x0A;
}
























