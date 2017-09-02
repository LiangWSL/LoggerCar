#ifndef __FILE_H
#define __FILE_H


/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "common.h"
#include "TFTLCDProcess.h"


#include "rt.h"
#include "analog.h"
#include "gprs.h"
#include "gps.h"
#include "fatfs.h"

/******************************************************************************/
#define ANALOG_VALUE_FORMAT						(FORMAT_ONE_DECIMAL)
#define FILE_NAME_SAVE_DATA						("date1.csv")
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
} SaveInfoAnalogTypedef;

typedef struct
{
	char year[2];								/* ���ڣ������� ʱ���룬��14�� */
	char month[2];
	char day[2];
	char str1;
	char hour[2];
	char min[2];
	char sec[2];
	char str2;
	char batQuality[3];							/* ��ص�������4�� */
	char str3;
	FunctionalState exPwrStatus;				/* ���״̬����2�� */
	char str4;
	char longitude[10];							/* ���ȣ���11�� */
	char str5;
	char latitude[10];							/* γ�ȣ���11�� */
	char str6;
	SaveInfoAnalogTypedef analogValue[8];		/* ģ��������6 x 8�� */
	char end[2];								/* ���У�2�� */
} FILE_SaveInfoTypedef;							/* �ܹ���92�ֽڣ� */

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
extern uint64_t dataFileStructCnt;					/* ��ǰ�ļ��ṹ������ */

/******************************************************************************/
void FILE_Init(void);
void FILE_SaveInfoSymbolInit(FILE_SaveInfoTypedef* info);
ErrorStatus FILE_ReadFile(char* fileName, uint32_t offset, BYTE* pBuffer, uint32_t size);
ErrorStatus FILE_WriteFile(char* fileName, uint32_t offset, BYTE* pBuffer, uint32_t size);


ErrorStatus FILE_SaveInfo(FILE_SaveInfoTypedef* saveInfo, uint64_t* fileStructCount);
uint16_t FILE_ReadInfo(FILE_SaveInfoTypedef* readInfo, FILE_PatchPackTypedef* patch);
void FILE_SendInfoFormatConvert(uint8_t* saveInfo, uint8_t* sendInfo, uint8_t sendPackNumb);

ErrorStatus FILE_ParamFileInit(void);
//ErrorStatus FILE_PrintDependOnTime(FILE_RealTime* startTime, FILE_RealTime* stopTime,
//		PRINT_ChannelSelectTypedef* select);

#endif
