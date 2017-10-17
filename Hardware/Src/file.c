#include "file.h"
#include "fatfs.h"

/******************************************************************************/
FILE_PatchPackTypedef FILE_PatchPack;		/* �����ļ���Ϣ */
uint64_t FILE_DataSaveStructCnt;			/* ���ݴ����ļ��ṹ������ */


FILE_SaveStructTypedef FILE_SaveStruct;			/* ������Ϣд��ṹ�� */
FILE_SaveStructTypedef FILE_ReadStruct[GPRS_PATCH_PACK_NUMB_MAX];
												/* ������Ϣ��ȡ�ṹ�� */

/******************************************************************************/
extern FATFS objFileSystem;
extern FIL   objFile;

/******************************************************************************/
static void SaveStructSymbolInit(void);
static void FormatConvert(GPRS_SendInfoTypedef*   sendInfo,
						  FILE_SaveStructTypedef* readInfo);

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
 * @brief ���ļ�
 * @param offset:��ָ��ƫ����
 * @param fileName:�ļ���
 * @param pBuffer������ָ��
 * @param size:��ȡ����
 */
void FILE_ReadFile(char* fileName, DWORD offset, BYTE* pBuffer, UINT size)
{
	UINT byteRead = 0;

	if (FR_OK != f_mount(&objFileSystem, USERPath, 1))
		return;
	/* �����ٽ��� */
	taskENTER_CRITICAL();

	if (FR_OK == f_open(&objFile, fileName, FA_OPEN_EXISTING | FA_READ))
	{
		f_lseek(&objFile, offset);
		f_read(&objFile, pBuffer, size, &byteRead);
		f_close(&objFile);
	}

	f_mount(NULL, USERPath, 1);
	/* �˳��ٽ��� */
	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * @brief д�ļ�
 * @param offset:дָ��ƫ����
 * @param fileName:�ļ���
 * @param pBuffer��д��ָ��
 * @param size:д�볤��
 */
void FILE_WriteFile(char* fileName, DWORD offset, BYTE* pBuffer, UINT size)
{
	UINT byteWrite = 0;

	if (FR_OK != f_mount(&objFileSystem, USERPath, 1))
		return;
	/* �����ٽ��� */
	taskENTER_CRITICAL();

	if (FR_OK == f_open(&objFile, fileName, FA_OPEN_ALWAYS | FA_WRITE))
	{
		f_lseek(&objFile, offset);
		f_read(&objFile, pBuffer, size, &byteWrite);
		f_close(&objFile);
	}

	f_mount(NULL, USERPath, 1);
	/* �˳��ٽ��� */
	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * function:����ṹ�嵽Flash
 * saveInfo������ṹ��ָ��
 * fileStructCount����ǰ�ļ��ṹ����
 */
void FILE_SaveInfo(void)
{
	uint32_t byteWrite = 0;

	if (FR_OK != f_mount(&objFileSystem, USERPath, 1))
		return;
	/* �����ٽ��� */
	taskENTER_CRITICAL();

	if (FR_OK == f_open(&objFile, FILE_NAME_SAVE_DATA, FA_OPEN_ALWAYS | FA_WRITE))
	{
		/* ���ж�д���ַ�Ƿ���룬�����д�룬�������򸲸ǲ���������� */
		if (objFile.fsize % sizeof(FILE_SaveStructTypedef) == 0)
			f_lseek(&objFile, objFile.fsize);
		else
			/* ���ǵ�ǰ�ṹ�� */
			f_lseek(&objFile,
					(objFile.fsize / sizeof(FILE_SaveStructTypedef))
					* sizeof(FILE_SaveStructTypedef));
		/* �ѽṹ��д���ļ� */
		f_write(&objFile, (BYTE*)&FILE_SaveStruct,
				sizeof(FILE_SaveStructTypedef), &byteWrite);
		/* ��ȡ�ļ���С */
		FILE_DataSaveStructCnt = objFile.fsize / sizeof(FILE_SaveStructTypedef);

		f_close(&objFile);
	}

	/* �˳��ٽ��� */
	taskEXIT_CRITICAL();
	f_mount(NULL, USERPath, 1);
}

/*******************************************************************************
 * function����Flash�ж������һ���ṹ��
 * readInfo����������ṹ��ָ��
 * @patch��������Ϣ
 */
uint8_t FILE_ReadInfo(FILE_PatchPackTypedef*  patch)
{
	uint8_t readInfoCount;

	/* û�в������� */
	if (patch->patchStructOffset == 0)
	{
		/* ��ȡ����Ϊ1�� */
		readInfoCount = 1;
		/* ��ȡ���һ���ṹ�� */
		FILE_ReadFile(FILE_NAME_SAVE_DATA,
				(FILE_DataSaveStructCnt - 1) * sizeof(FILE_SaveStructTypedef),
				(uint8_t*)FILE_ReadStruct, sizeof(FILE_SaveStructTypedef));
	}
	else
	{
		/* ��ǰ�ļ����ж��ٸ��ṹ����Զ� */
		readInfoCount = FILE_DataSaveStructCnt - patch->patchStructOffset;

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
		FILE_ReadFile(FILE_NAME_SAVE_DATA,
				patch->patchStructOffset * sizeof(FILE_SaveStructTypedef),
				(uint8_t*)FILE_ReadStruct,
				readInfoCount * sizeof(FILE_SaveStructTypedef));
	}
	return readInfoCount;
}

/*******************************************************************************
 * @brief ������������ת���ɷ��͵ĸ�ʽ
 * @param sendInfo���������ݽṹ��ָ��
 * @param readInfo����ȡ���ݵĽṹ��ָ��
 * @param sendPackNumb����Ҫת���İ���
 */
void FILE_SendInfoFormatConvert(uint8_t* sendInfo, uint8_t* readInfo,
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
 * @brief �������ASCII��ת����float
 * @retval ģ����ASCII��
 */
float FILE_Analog2Float(FILE_SaveInfoAnalogTypedef* value)
{
	char str[6];

	memcpy(str, value->value, 5);
	str[5] = '\0';
	return (float)atof(str);
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

	/* �����ͨ��ֵ��NULL�������Э����дFFFE */
	if (strcmp(analog, "_NULL") == 0)
	{
		*pBuffer       = 0xFF;
		*(pBuffer + 1) = 0xFE;
		return;
	}

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
























