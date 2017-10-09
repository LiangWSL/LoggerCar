#ifndef __RT_H
#define __RT_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "public.h"

#include "rtc.h"

/******************************************************************************/
#define RT_RTC						(hrtc)

/******************************************************************************/
#define RTC_BKUP_DATA				(0xA0A1)

#define RTC_BKUP_REG_DATA			(RTC_BKP_DR2)
#define RTC_BKUP_REG_YEAR			(RTC_BKP_DR3)
#define RTC_BKUP_REG_MONTH			(RTC_BKP_DR4)
#define RTC_BKUP_REG_DAY			(RTC_BKP_DR5)
#define RTC_BKUP_REG_WEEK			(RTC_BKP_DR6)

/******************************************************************************/
#define RT_OFFSET_CLOUD_TIME		(8)				/* ƽ̨����ʱ��ƫ�� */

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* ���ֽڶ��� */

typedef struct
{
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;
	uint8_t         oldWeekDay;				/* �ϴμ�¼����,���ڸ������� */
} RT_TimeTypedef;

#pragma pack(pop)

/******************************************************************************/
extern RT_TimeTypedef RT_RealTime;
extern BOOL RT_recodeFlag;

/******************************************************************************/
void RT_Init(void);
void RT_BKUP_UpdateDate(RT_TimeTypedef* time);
void RT_TimeAdjustWithCloud(uint8_t* pBuffer);

#endif

