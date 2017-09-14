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
	uint32_t hisDataDispStructOffset;			/* ��ʷ������ʾ�ṹ��ƫ�� */
	ChannelSelectTypedef printChannelStatus;	/* ��ӡ����ͨ��״̬ */
	TimeSelectEnum timeSelectStatus;			/* ʱ��ѡ��״̬����������ĸ��������ѡ�� */
	FILE_RealTimeTypedef* selectTime;			/* �ѱ�ѡ���ʱ�� */
	FILE_RealTimeTypedef printTimeStart;		/* ��ʼ��ӡʱ�� */
	FILE_RealTimeTypedef printTimeEnd;			/* ������ӡʱ�� */

	char    passwordBuffer[4];					/* ���뻺�� */
	uint8_t passwordBufferIndex;				/* ����λָʾ */
} DISPLAY_StatusTypedef;


#pragma pack(pop)

/******************************************************************************/
extern DISPLAY_StatusTypedef DISPLAY_Status;

/******************************************************************************/
void DISPLAY_HistoryData(uint32_t startStructOffset, uint8_t structCnt);
void DISPLAY_HistoryTouch(uint16_t typeID);
void DISPLAY_HistoryDataCurve(uint32_t startStructOffset);
void DISPLAY_PrintTouch(uint16_t typeID);
void DISPLAY_TimeSelectTouch(uint16_t typeID, uint8_t value);
void DISPLAY_SetPassword(uint16_t typeID);


#endif
