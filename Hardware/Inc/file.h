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
#include "fatfs.h"

/******************************************************************************/
#define ANALOG_VALUE_FORMAT						(FORMAT_ONE_DECIMAL)
#define PATCH_PACK_FILE_NAME					("patch.txt")
#define PARAM_FILE_NAME							("param.txt")

#define FILE_PRINT_TO_END						(0xFFFF)

/******************************************************************************/
typedef enum
{
	LOCATION_BASE_SATTION,
	LOCATION_GPS
} LocationTypdEnum;

typedef enum
{
	TYPE_TEMP = 1,									/* �¶� */
	TYPE_HUMI,										/* ʪ�� */
	TYPE_RAINFALL,									/* ���� */
	TYPE_SOIL_MOISTURE,								/* ����ˮ�� */
	TYPE_VOLTAGE,									/* ��ѹ */
	TYPE_ELECTRIC_CURRENT,							/* ���� */
	TYPE_POWER,										/* ���� */
	TYPE_ILLUMINATION_INTENSITY,					/* ����ǿ�� */
	TYPE_WIND_SPEED,								/* ���� */
	TYPE_WIND_DIR,									/* ���� */
	TYPE_CO2,										/* CO2 */
	TYPE_BAROMETRIC_PRESSURE,						/* ����ѹ */
	TYPE_O2,										/* O2 */
	TYPE_NUMB_OF_TIMES,								/* ���� */
	TYPE_WATER_LEVEL,								/* ˮλ */
	TYPE_PRESSURE,									/* ѹ�� */
	TYPE_PH_VALUE,									/* PHֵ */
} ChannelTypeEnum;

typedef enum
{
	UNIT_TEMP = 1,									/* �¶� */
	UNIT_HUMI,										/* ʪ�� */
	UNIT_RAINFALL,									/* ���� */
	UNIT_SOIL_MOISTURE,								/* ����ˮ�� */
	UNIT_VOLTAGE,									/* ��ѹ */
	UNIT_ELECTRIC_CURRENT,							/* ���� */
	UNIT_POWER,										/* ���� */
	UNIT_ILLUMINATION_INTENSITY,					/* ����ǿ�� */
	UNIT_WIND_SPEED,								/* ���� */
	UNIT_WIND_DIR,									/* ���� */
	UNIT_CO2,										/* CO2 */
	UNIT_BAROMETRIC_PRESSURE,						/* ����ѹ */
	UNIT_O2,										/* O2 */
	UNIT_NUMB_OF_TIMES,								/* ���� */
	UNIT_WATER_LEVEL,								/* ˮλ */
	UNIT_PRESSURE,									/* ѹ�� */
	UNIT_PH_VALUE,									/* PHֵ */
} ChannelUnitEnum;

typedef enum
{
	FORMAT_INT = 1,									/* ���ݸ�ʽ���� */
	FORMAT_ONE_DECIMAL,								/* ���ݸ�ʽ1λС�� */
	FORMAT_TWO_DECIMAL,								/* ���ݸ�ʽ2λС�� */
} DataFormatEnum;

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
	char patchFileName[11];						/* ��ʼ�������ļ��� */
	uint16_t patchStructOffset;					/* ��ʼ�����Ľṹ�����ļ��е�ƫ�� */
	uint16_t patchPackOver_5;					/* ��������5�� */
	uint16_t patchPackOver_10;					/* ��������10�� */
	uint16_t patchPackOver_20;					/* ��������20�� */
	uint16_t patchPackOver_30;					/* ��������30�� */
} FILE_PatchPackTypedef;

/**************************������������*******************************************/
typedef struct
{
	ChannelTypeEnum channelType;					/* ͨ������ */
	ChannelUnitEnum channelUnit;					/* ͨ����λ */
	DataFormatEnum  dataFormat;					/* ������ʽ */
} ParamTypeTypedef;

typedef struct
{
	char    deviceSN[10];									/* �豸SN�� */
	LocationTypdEnum locationType;							/* ��λ��� */
	uint8_t firmwareVersion;								/* �̼��汾�� */
	uint8_t recordInterval;									/* ��¼��� */
	uint8_t overLimitRecordInterval;						/* �����¼��� */
	uint8_t exitAnalogChannelNumb;							/* �ⲿģ����ͨ���� */
	ParamTypeTypedef param[ANALOG_CHANNEL_NUMB];			/* ģ�������� */
} FILE_DeviceParamTypedef;

#pragma pack(pop)

/******************************************************************************/
extern FILE_DeviceParamTypedef FILE_DeviceParam;	/* �豸������Ϣ */

/******************************************************************************/
void FILE_Init(void);
ErrorStatus FILE_SaveInfo(FILE_InfoTypedef* saveInfo, uint16_t* fileStructCount);
ErrorStatus FILE_ReadInfo(FILE_InfoTypedef* readInfo);
uint16_t FILE_ReadPatchInfo(FILE_PatchPackTypedef* patch, FILE_InfoTypedef* readInfo);
void FILE_InfoFormatConvert(FILE_InfoTypedef*    saveInfo,
							RT_TimeTypedef*      realTime,
							GPS_LocateTypedef*   location,
							ANALOG_ValueTypedef* analogValue);
ErrorStatus FILE_ReadPatchPackFile(FILE_PatchPackTypedef* pBuffer);
ErrorStatus FILE_WritePatchPackFile(FILE_PatchPackTypedef* pBuffer);
ErrorStatus FILE_ParamFileInit(void);
ErrorStatus FILE_PrintDependOnTime(FILE_RealTime* startTime, FILE_RealTime* stopTime,
		PRINT_ChannelSelectTypedef* select);

#endif
