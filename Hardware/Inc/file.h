#ifndef __FILE_H
#define __FILE_H


/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "common.h"
#include "TFTLCDProcess.h"


#include "rt.h"
#include "analog.h"
#include "gps.h"
#include "eeprom.h"
#include "fatfs.h"

/******************************************************************************/
#define ANALOG_VALUE_FORMAT						(FORMAT_ONE_DECIMAL)
#define PATCH_PACK_FILE_NAME					("patch.txt")

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)									/* ���ֽڶ��� */

typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
} FILE_RealTime;

typedef struct
{
	uint16_t temp1;
	uint16_t humi1;

	uint16_t temp2;
	uint16_t humi2;

	uint16_t temp3;
	uint16_t humi3;

	uint16_t temp4;
	uint16_t humi4;
} FILE_AnalogValueTypedef;

typedef struct
{
	FILE_RealTime realTime;						/* ʱ�� */
	uint8_t  batteryLevel;						/* ��ص��� */
	uint8_t  externalPowerStatus;				/* �ⲿ���״̬ */
	uint32_t longitude;							/* ���� */
	uint32_t latitude;							/* γ�� */
	uint8_t  resever;							/* ���� */
	FILE_AnalogValueTypedef analogValue;		/* ģ����ֵ */
} FILE_InfoTypedef;

typedef struct
{
	uint16_t patchPackNumb;						/* ������������ */
	uint16_t patchPackOver_5;					/* ��������5�� */
	uint16_t patchPackOver_10;					/* ��������10�� */
	uint16_t patchPackOver_20;					/* ��������20�� */
	uint16_t patchPackOver_30;					/* ��������30�� */
} FILE_PatchPackTypedef;

#pragma pack(pop)

/******************************************************************************/
void FILE_Init(void);
ErrorStatus FILE_SaveReadInfo(FILE_InfoTypedef* saveInfo,
		FILE_InfoTypedef* readInfo, uint8_t readInfoCount);
void FILE_InfoFormatConvert(FILE_InfoTypedef*    saveInfo,
							RT_TimeTypedef*      realTime,
							GPS_LocateTypedef*   location,
							ANALOG_ValueTypedef* analogValue);
ErrorStatus FILE_ReadPatchPackFile(FILE_PatchPackTypedef* pBuffer);
ErrorStatus FILE_WritePatchPackFile(FILE_PatchPackTypedef* pBuffer);
ErrorStatus FILE_PrintDependOnTime(FILE_RealTime* startTime, FILE_RealTime* stopTime,
		PRINT_ChannelSelectTypedef* select);

#endif
