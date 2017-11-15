#ifndef __FILE_H
#define __FILE_H


/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "TFTLCDProcess.h"
#include "public.h"

#include "rt.h"
#include "gps.h"
#include "analog.h"
#include "fatfs.h"
#include "param.h"
#include "input.h"

/******************************************************************************/
#define ANALOG_VALUE_FORMAT						(FORMAT_ONE_DECIMAL)
#define FILE_NAME_SAVE_DATA						("date.csv")
#define FILE_NAME_PATCH_PACK					("patch.txt")
#define FILE_NAME_PARAM							("param.txt")

#define FILE_PRINT_TO_END						(0xFFFF)



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
} FILE_RealTimeTypedef;

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
	char value[5];
	char str;
} FILE_SaveInfoAnalogTypedef;

typedef struct
{
	char year[2];								/* ���ڣ������� ʱ���룬��16�� */
	char month[2];
	char day[2];
	char str1;
	char hour[2];
	char str2;
	char min[2];
	char str3;
	char sec[2];
	char str4;
	char batQuality[4];							/* ��ص�������5�� */
	char str5;
	char exPwrStatus;							/* ���״̬����2�� */
	char str6;
	char locationStatus;						/* ��λ��־�� ��2�� */
	char str7;
	char longitude[10];							/* ���ȣ���11�� */
	char str8;
	char latitude[10];							/* γ�ȣ���11�� */
	char str9;
	FILE_SaveInfoAnalogTypedef analogValue[8];		/* ģ��������6 x 8�� */
	char end[2];								/* ���У�2�� */
} FILE_SaveStructTypedef;							/* �ܹ���97�ֽڣ� */

typedef struct
{
	uint32_t patchStructOffset;					/* ��ʼ�����Ľṹ�����ļ��е�ƫ�� */
	uint16_t patchPackOver_5;					/* ��������5�� */
	uint16_t patchPackOver_10;					/* ��������10�� */
	uint16_t patchPackOver_20;					/* ��������20�� */
	uint16_t patchPackOver_30;					/* ��������30�� */
} FILE_PatchPackTypedef;

#pragma pack(pop)

/******************************************************************************/
extern uint64_t FILE_DataSaveStructCnt;			/* ��ǰ�ļ��ṹ������ */

/******************************************************************************/
void FILE_Init(void);
void FILE_ReadFile(char* fileName, DWORD offset, BYTE* pBuffer, UINT size);
void FILE_WriteFile(char* fileName, DWORD offset, BYTE* pBuffer, UINT size);
uint8_t FILE_ReadInfo(FILE_PatchPackTypedef*  patch);
void FILE_SendInfoFormatConvert(uint8_t* sendInfo, uint8_t* readInfo,
							    uint8_t  sendPackNumb);
float FILE_Analog2Float(FILE_SaveInfoAnalogTypedef* value);

void FILE_SaveSendInfo(FILE_SaveStructTypedef* saveInfo, RT_TimeTypedef* curtime,
		GPS_LocateTypedef* location, ANALOG_ValueTypedef* analog);
uint8_t FILE_ReadSaveInfo(FILE_SaveStructTypedef* readInfo, uint8_t structoffset);
#endif
