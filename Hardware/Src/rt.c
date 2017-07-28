#include "rt.h"

#include "common.h"

#include "RealTime.h"
#include "osConfig.h"

/*******************************************************************************
 *
 */
void RT_BKUP_UpdateDate(RT_TimeTypedef* time)
{
	/* ���±������� */
	HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_YEAR,  RT_RTC.DateToUpdate.Year);
	HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_MONTH, RT_RTC.DateToUpdate.Month);
	HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_DAY,   RT_RTC.DateToUpdate.Date);
	HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_WEEK,  RT_RTC.DateToUpdate.WeekDay);

	/* �����ϴμ�¼���� */
	time->oldWeekDay = RT_RTC.DateToUpdate.WeekDay;
}

/*******************************************************************************
 *
 */
static void RT_BKUP_ReadDate(void)
{
	/* ��ȡ�������� */
	RT_RTC.DateToUpdate.Year =    HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_YEAR);
	RT_RTC.DateToUpdate.Month =   HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_MONTH);
	RT_RTC.DateToUpdate.Date =    HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_DAY);
	RT_RTC.DateToUpdate.WeekDay = HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_WEEK);
}

/*******************************************************************************
 *
 */
void RT_SetRealTime(RT_TimeTypedef* time)
{
	HAL_RTC_SetDate(&RT_RTC, &time->date, RTC_FORMAT_BCD);
	HAL_RTC_SetTime(&RT_RTC, &time->time, RTC_FORMAT_BCD);

	/* �����µ����ڱ��� */
	RT_BKUP_UpdateDate(time);
}

/*******************************************************************************
 *
 */
void RT_Init(RT_TimeTypedef* time)
{
	if (HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_DATA) != RTC_BKUP_DATA)
	{
		time->date.Year = 0X17;
		time->date.Month = RTC_MONTH_JULY;
		time->date.Date = 0x28;
		time->date.WeekDay = RTC_WEEKDAY_FRIDAY;
		time->time.Hours = 0x09;
		time->time.Minutes = 0x50;
		time->time.Seconds = 0x00;
		RT_SetRealTime(time);

		HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_DATA, RTC_BKUP_DATA);
	}
	else
	{
		RT_BKUP_ReadDate();
		HAL_RTC_GetDate(&RT_RTC, &time->date, RTC_FORMAT_BCD);
	}
}

/******************************************************************************/
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	osSignalSet(realtimeTaskHandle, REALTIME_TASK_SIGNAL_UPDATE);
}

/*******************************************************************************
 * ����ƽ̨ʱ��У׼
 * @pBuffer�����ջ������ݣ�ƽ̨���ģ�
 * @pStruct�����ͽṹ��
 */
void RT_TimeAdjustWithCloud(uint8_t* pBuffer, RT_TimeTypedef* time)
{
	uint8_t str[12] = {0};

	RT_TimeTypedef   eTime;

	/* ���ַ�ת�������� */
	str2numb((pBuffer + RT_OFFSET_CLOUD_TIME), str, sizeof(str));

	eTime.date.Year    = (str[0] << 4)  + (str[1]);
	eTime.date.Month   = (str[2] << 4)  + (str[3]);
	eTime.date.Date    = (str[4] << 4)  + (str[5]);
	eTime.time.Hours   = (str[6] << 4)  + (str[7]);
	eTime.time.Minutes = (str[8] << 4)  + (str[9]);
	eTime.time.Seconds = (str[10] << 4) + (str[11]);

	/* ���յ�ƽ̨���ģ��뷢��ʱ��Ƚϣ������������ʱ����ƫ���У׼�����Ӳ��ƣ�У׼�ֽڳ���Ϊ5 */
	if ((0 != memcmp(&eTime.date, &time->date, 3)
			|| (0 != memcmp(&eTime.time, &time->time, 2))))
	{
		RT_SetRealTime(&eTime);
	}
}
