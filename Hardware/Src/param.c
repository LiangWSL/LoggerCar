#include "param.h"

/******************************************************************************/
PARAM_DeviceParamTypedef PARAM_DeviceParam;

/*******************************************************************************
 * function:�豸�����ļ���ʼ��
 */
ErrorStatus PARAM_ParamFileInit(void)
{
	/* �����ļ�ϵͳ */
//	if (ERROR == FATFS_FileLink())
//		return ERROR;

	/* ����ļ������� */
//	if (FATFS_FileOpen(PARAM_FILE_NAME, FATFS_MODE_OPEN_EXISTING_READ) == ERROR)
	{
		memcpy(PARAM_DeviceParam.deviceSN, "1708151515", sizeof(PARAM_DeviceParam.deviceSN));
		PARAM_DeviceParam.locationType    = LOCATION_GPS;
		PARAM_DeviceParam.firmwareVersion = 10;
		PARAM_DeviceParam.recordInterval  = 2;
		PARAM_DeviceParam.overLimitRecordInterval = 2;
		PARAM_DeviceParam.exitAnalogChannelNumb = ANALOG_CHANNEL_NUMB;
		PARAM_DeviceParam.param[0].channelType = TYPE_TEMP;
		PARAM_DeviceParam.param[0].channelUnit = UNIT_TEMP;
		PARAM_DeviceParam.param[0].dataFormat  = FORMAT_ONE_DECIMAL;
		PARAM_DeviceParam.param[1].channelType = TYPE_HUMI;
		PARAM_DeviceParam.param[1].channelUnit = UNIT_HUMI;
		PARAM_DeviceParam.param[1].dataFormat  = FORMAT_ONE_DECIMAL;
		PARAM_DeviceParam.param[2].channelType = TYPE_TEMP;
		PARAM_DeviceParam.param[2].channelUnit = UNIT_TEMP;
		PARAM_DeviceParam.param[2].dataFormat  = FORMAT_ONE_DECIMAL;
		PARAM_DeviceParam.param[3].channelType = TYPE_HUMI;
		PARAM_DeviceParam.param[3].channelUnit = UNIT_HUMI;
		PARAM_DeviceParam.param[3].dataFormat  = FORMAT_ONE_DECIMAL;
		PARAM_DeviceParam.param[4].channelType = TYPE_TEMP;
		PARAM_DeviceParam.param[4].channelUnit = UNIT_TEMP;
		PARAM_DeviceParam.param[4].dataFormat  = FORMAT_ONE_DECIMAL;
		PARAM_DeviceParam.param[5].channelType = TYPE_HUMI;
		PARAM_DeviceParam.param[5].channelUnit = UNIT_HUMI;
		PARAM_DeviceParam.param[5].dataFormat  = FORMAT_ONE_DECIMAL;
		PARAM_DeviceParam.param[6].channelType = TYPE_TEMP;
		PARAM_DeviceParam.param[6].channelUnit = UNIT_TEMP;
		PARAM_DeviceParam.param[6].dataFormat  = FORMAT_ONE_DECIMAL;
		PARAM_DeviceParam.param[7].channelType = TYPE_HUMI;
		PARAM_DeviceParam.param[7].channelUnit = UNIT_HUMI;
		PARAM_DeviceParam.param[7].dataFormat  = FORMAT_ONE_DECIMAL;

		PARAM_DeviceParam.channel[0].alarmValueUp       = 40;
		PARAM_DeviceParam.channel[0].alarmValueLow      = -10;
		PARAM_DeviceParam.channel[0].perwarningValueUp  = 30;
		PARAM_DeviceParam.channel[0].perwarningValueLow = -5;
		PARAM_DeviceParam.channel[1].alarmValueUp       = 40;
		PARAM_DeviceParam.channel[1].alarmValueLow      = -10;
		PARAM_DeviceParam.channel[1].perwarningValueUp  = 30;
		PARAM_DeviceParam.channel[1].perwarningValueLow = -5;
		PARAM_DeviceParam.channel[2].alarmValueUp       = 40;
		PARAM_DeviceParam.channel[2].alarmValueLow      = -10;
		PARAM_DeviceParam.channel[2].perwarningValueUp  = 30;
		PARAM_DeviceParam.channel[2].perwarningValueLow = -5;
		PARAM_DeviceParam.channel[3].alarmValueUp       = 40;
		PARAM_DeviceParam.channel[3].alarmValueLow      = -10;
		PARAM_DeviceParam.channel[3].perwarningValueUp  = 30;
		PARAM_DeviceParam.channel[3].perwarningValueLow = -5;

		memcpy(PARAM_DeviceParam.password, "0000", 4);

//		FATFS_FileOpen(FILE_NAME_PARAM, FATFS_MODE_OPEN_ALWAYS_WRITE);
//		FATFS_FileWrite((uint8_t*)&PARAM_DeviceParam, sizeof(PARAM_DeviceParam));
	}
	/* �򿪳ɹ���˵���ļ����� */
//	else
//	{
//		FATFS_FileRead((uint8_t*)&PARAM_DeviceParam, sizeof(PARAM_DeviceParam));
//	}

//	if (FATFS_FileClose() == ERROR)
//		return ERROR;

//	FATFS_FileUnlink();

	return SUCCESS;
}





