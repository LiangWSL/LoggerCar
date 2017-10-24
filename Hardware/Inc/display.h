#ifndef __DISPLAY_H
#define __DISPLAY_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "file.h"

/******************************************************************************/
#define DISPLAY_HIS_DATA_ONE_SCREEN_CNT				(4)		/* ��ʷ����һ��������ʾ4�� */
#define DISPLAY_HIS_DATA_READ_ONCE_CNT				(5)		/* ��ʷ���߽��棬һ�ζ�ȡ5������ */
#define DISPLAY_HIS_DATA_READ_CNT					(18)	/* һ������90�����ݣ���ȡ16�ο���� */
#define DISPLAY_HIS_DATA_CURVE_CNT					(DISPLAY_HIS_DATA_READ_ONCE_CNT * DISPLAY_HIS_DATA_READ_CNT)

/******************************************************************************/
typedef enum
{
	TIME_SELECT_HIS_DATA_START,
	TIME_SELECT_HIS_DATA_END,
	TIME_SELECT_START_PRINT_TIME,
	TIME_SELECT_END_PRINT_TIME,
} TimeSelectEnum;

/******************************************************************************/
typedef enum
{
	PRINT_MODE_INTEGRATED,						/* һ��ʽ */
	PRINT_MODE_BLE_UNLINK,						/* ������ӡ��δ���� */
	PRINT_MODE_BLE_LINK,						/* ������ӡ�������� */
} PrintModeEnum;

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)

/******************************************************************************/
typedef struct
{
	union
	{
		struct
		{
			uint8_t ch1:1;
			uint8_t ch2:1;
			uint8_t ch3:1;
			uint8_t ch4:1;
			uint8_t ch5:1;
			uint8_t ch6:1;
			uint8_t ch7:1;
			uint8_t ch8:1;
		} bit;
		uint8_t all;
	} status;
} ChannelSelectTypedef;

typedef struct
{
	char year[2];								/* ���ڣ������� ʱ���룬��16�� */
	char month[2];
	char day[2];
	char str1;
	char hour[2];
	char str2;
	char min[2];
} DISPLAY_CompareTimeTypedef;

typedef struct
{
	uint32_t hisDataDispStructOffset;			/* ��ʷ������ʾ�ṹ��ƫ�� */
	FunctionalState      displayModeBusy;		/* ��ʾ״̬æ���ظ�������Ч */
	ChannelSelectTypedef printChannelStatus;	/* ��ӡ����ͨ��״̬ */
//	TimeSelectEnum timeSelectStatus;			/* ʱ��ѡ��״̬����������ĸ��������ѡ�� */

	DISPLAY_CompareTimeTypedef* selectTime;				/* �ѱ�ѡ���ʱ�� */
	DISPLAY_CompareTimeTypedef  hisDataTimeStart;		/* ��ʷ���ݿ�ʼʱ�� */
	DISPLAY_CompareTimeTypedef  hisDataTimeStop;		/* ��ʷ����ֹͣʱ�� */
	DISPLAY_CompareTimeTypedef  printTimeStart;			/* ��ʼ��ӡʱ�� */
	DISPLAY_CompareTimeTypedef  printTimeEnd;			/* ������ӡʱ�� */

	uint32_t hisDataTimePointStart;				/* ��ʷ���ݿ�ʼʱ��� */
	uint32_t hisDataTimePointStop;				/* ��ʷ���ݽ���ʱ��� */

	char    passwordBuffer[4];					/* ���뻺�� */
	char    passwordBufferNew[4];
	char    passwordBufferNewAgain[4];
	uint8_t passwordBufferIndex;				/* ����λָʾ */

	PrintModeEnum printMode;					/* ��ӡģʽ */
} DISPLAY_StatusTypedef;


#pragma pack(pop)

/******************************************************************************/
extern DISPLAY_StatusTypedef DISPLAY_Status;

/******************************************************************************/
void DISPLAY_Init(void);
void DISPLAY_HistoryData(uint32_t startStructOffset, uint8_t structCnt);
void DISPLAY_HistoryTouch(uint16_t typeID);
void DISPLAY_HistoryDataCurve(uint32_t startStructOffset);
void DISPLAY_TimeSelect(RT_TimeTypedef* time);
void DISPLAY_PrintTouch(uint16_t typeID);
void DISPLAY_TimeSelectTouch(uint16_t typeID, uint8_t value);
void DISPLAY_SetPasswordTouch(uint16_t typeID);
void DISPLAY_SetAlarmLimitTouch(uint16_t typeID);
void DISPLAY_SetAlarmLimit2Touch(uint16_t typeID);
void DISPLAY_SetMessageTouch(uint16_t typeID);
void DISPLAY_SetPasswordChangeTouch(uint16_t typeID);


#endif
