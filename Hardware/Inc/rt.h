#ifndef __RT_H
#define __RT_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "rtc.h"

/******************************************************************************/
#define RT_RTC						(hrtc)

/******************************************************************************/
#define RTC_BKUP_DATA			(0xA0A0)

#define RTC_BKUP_REG_DATA		(RTC_BKP_DR2)
#define RTC_BKUP_REG_YEAR		(RTC_BKP_DR3)
#define RTC_BKUP_REG_MONTH		(RTC_BKP_DR4)
#define RTC_BKUP_REG_DAY		(RTC_BKP_DR5)
#define RTC_BKUP_REG_WEEK		(RTC_BKP_DR6)

/******************************************************************************/
typedef struct
{
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;
	uint8_t oldWeekDay;				/* �ϴμ�¼����,���ڸ������� */
} RT_TimeTypedef;

/******************************************************************************/
void RT_BKUP_UpdateDate(RT_TimeTypedef* time);
static void RT_BKUP_ReadDate(void);
void RT_SetRealTime(RT_TimeTypedef* time);
void RT_Init(RT_TimeTypedef* time);

#endif

