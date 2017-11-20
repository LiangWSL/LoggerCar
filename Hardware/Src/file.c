#include "file.h"

/******************************************************************************/
FILE_PatchPackTypedef FILE_PatchPack;		/* �����ļ���Ϣ */
uint64_t FILE_DataSaveStructCnt;			/* ���ݴ����ļ��ṹ������ */


FILE_SaveStructTypedef FILE_SaveStruct;			/* ������Ϣд��ṹ�� */
FILE_SaveStructTypedef FILE_ReadStruct[SEND_PACK_CNT_MAX];
												/* ������Ϣ��ȡ�ṹ�� */

/******************************************************************************/
extern FATFS objFileSystem;
extern FIL   objFile;

/******************************************************************************/
static void SaveStructSymbolInit(void);
static void Analog2ASCII(FILE_SaveInfoAnalogTypedef* buffer, float value);

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
		f_write(&objFile, pBuffer, size, &byteWrite);
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
static void FILE_SaveInfo(BYTE* saveInfo, UINT size)
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
		f_write(&objFile, saveInfo, size, &byteWrite);
		/* ��ȡ�ļ��нṹ���� */
		FILE_DataSaveStructCnt = objFile.fsize / sizeof(FILE_SaveStructTypedef);
		f_close(&objFile);
	}

	/* �˳��ٽ��� */
	taskEXIT_CRITICAL();
	f_mount(NULL, USERPath, 1);
}

/*******************************************************************************
 *
 */
void FILE_SaveSendInfo(FILE_SaveStructTypedef* saveInfo, RT_TimeTypedef* curtime,
		GPS_LocateTypedef* location, ANALOG_ValueTypedef* analog)
{
	/* ʱ��ת�� */
	HEX2ASCII(saveInfo->year,  &curtime->date.Year,    1);
	HEX2ASCII(saveInfo->month, &curtime->date.Month,   1);
	HEX2ASCII(saveInfo->day,   &curtime->date.Date,    1);
	HEX2ASCII(saveInfo->hour,  &curtime->time.Hours,   1);
	HEX2ASCII(saveInfo->min,   &curtime->time.Minutes, 1);
	HEX2ASCII(saveInfo->sec,   &curtime->time.Seconds, 1);
	/* ��ȡ�ⲿ��Դ״̬ */
	FILE_SaveStruct.exPwrStatus = INPUT_CheckPwrOnStatus() + '0';

	/* ģ����ת��ΪASCII */
	sprintf(saveInfo->batQuality, "%3d", ANALOG_value.batVoltage);
	Analog2ASCII(&saveInfo->analogValue[0], ANALOG_value.channel1);
	Analog2ASCII(&saveInfo->analogValue[1], ANALOG_value.channel2);
	Analog2ASCII(&saveInfo->analogValue[2], ANALOG_value.channel3);
	Analog2ASCII(&saveInfo->analogValue[3], ANALOG_value.channel4);
	Analog2ASCII(&saveInfo->analogValue[4], ANALOG_value.channel5);
	Analog2ASCII(&saveInfo->analogValue[5], ANALOG_value.channel6);
	Analog2ASCII(&saveInfo->analogValue[6], ANALOG_value.channel7);
	Analog2ASCII(&saveInfo->analogValue[7], ANALOG_value.channel8);


	/* ��λֵ */
	saveInfo->locationStatus = GPS_LOCATION_TYPE_GPS + '0';
	sprintf(saveInfo->longitude, "%10.5f", location->longitude);
	sprintf(saveInfo->latitude,  "%10.5f", location->latitude);

	/* CVS�ļ���ʽ */
	saveInfo->batQuality[3] = '%';		/* ��ص����ٷֺ� */
	saveInfo->str8   		= ',';
	saveInfo->str9   		= ',';

	/* �������� */
	FILE_SaveInfo((uint8_t*)saveInfo, sizeof(FILE_SaveStructTypedef));
}

/*******************************************************************************
 *
 */
uint8_t FILE_ReadSaveInfo(FILE_SaveStructTypedef* readInfo, uint32_t* structoffset)
{
	uint16_t sendPackCnt = 0;			/* ���͵İ��� */
	uint64_t offset = 0;

	/* ��������һ��������¼ָ�����ݣ�����ָ���ͻ */
	offset = *structoffset;

	/* ����û�в������ݣ���ȡ���µ�һ������ */
	if (offset == 0)
	{
		FILE_ReadFile(FILE_NAME_SAVE_DATA,
			(FILE_DataSaveStructCnt - 1) * sizeof(FILE_SaveStructTypedef),
			(uint8_t*)readInfo, sizeof(FILE_SaveStructTypedef));
		return 1;
	}

	sendPackCnt = FILE_DataSaveStructCnt - offset;
	/* ���ܹ�һ���Է������ */
	if (sendPackCnt > SEND_PACK_CNT_MAX)
	{
		sendPackCnt = SEND_PACK_CNT_MAX;
		offset += SEND_PACK_CNT_MAX;
		*structoffset = offset;
	}
	else
	{
		/* �Ѿ�������� */
		*structoffset = 0;
	}
	FILE_ReadFile(FILE_NAME_SAVE_DATA,
			offset * sizeof(FILE_SaveStructTypedef),
			(uint8_t*)readInfo, sendPackCnt * sizeof(FILE_SaveStructTypedef));

	/* ���ر��ζ�ȡ�Ľṹ���� */
	return sendPackCnt;
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
	FILE_SaveStruct.str7   = ',';
	FILE_SaveStruct.end[0] = 0x0D;
	FILE_SaveStruct.end[1] = 0x0A;
}

/*******************************************************************************
 * @brief �ж�ģ����ֵ�Ƿ���Ч����Ч����䡰 NULL������Ч��ת����ASCII
 */
static void Analog2ASCII(FILE_SaveInfoAnalogTypedef* buffer, float value)
{
	if (value == ANALOG_CHANNLE_INVALID_VALUE)
		memcpy(buffer->value, ANALOG_INVALID_VALUE, 5);
	else
	{
		/* %5.1��ʾ��Ч���ݳ���Ϊ5��С��1λ */
		sprintf(buffer->value, "%5.1f", value);
		buffer->str = ',';
	}
}






















