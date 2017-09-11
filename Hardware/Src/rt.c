#include "rt.h"

#include "common.h"

#include "RealTime.h"
#include "osConfig.h"

/******************************************************************************/
RT_TimeTypedef RT_RealTime;

/******************************************************************************/
static void RT_SetRealTime(RT_TimeTypedef* time);
static void RT_BKUP_ReadDate(void);

/*******************************************************************************
 * function��ʵʱʱ�ӳ�ʼ��
 */
void RT_Init(void)
{
	if (HAL_RTCEx_BKUPRead(&RT_RTC, RTC_BKUP_REG_DATA) != RTC_BKUP_DATA)
	{
		RT_RealTime.date.Year = 17;
		RT_RealTime.date.Month = RTC_MONTH_AUGUST;
		RT_RealTime.date.Date = 17;
		RT_RealTime.date.WeekDay = RTC_WEEKDAY_THURSDAY;
		RT_RealTime.time.Hours = 10;
		RT_RealTime.time.Minutes = 55;
		RT_RealTime.time.Seconds = 0x00;
		RT_SetRealTime(&RT_RealTime);

		HAL_RTCEx_BKUPWrite(&RT_RTC, RTC_BKUP_REG_DATA, RTC_BKUP_DATA);
	}
	else
	{
		RT_BKUP_ReadDate();
		HAL_RTC_GetDate(&RT_RTC, &RT_RealTime.date, RTC_FORMAT_BIN);
	}
}

/*******************************************************************************
 * function������ǰ���ڸ��µ��������򣬲���¼�µ�ǰ����ֵ�����ڸ�������
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
 * ����ƽ̨ʱ��У׼
 * @pBuffer�����ջ������ݣ�ƽ̨���ģ�
 * @pStruct�����ͽṹ��
 */
void RT_TimeAdjustWithCloud(uint8_t* pBuffer)
{
	uint8_t str[10] = {0};

	RT_TimeTypedef   eTime;

	/* ���ַ�ת�������� */
	str2numb((pBuffer + RT_OFFSET_CLOUD_TIME), str, sizeof(str));

	eTime.date.Year    = (str[0]  * 10)  + str[1];
	eTime.date.Month   = (str[2]  * 10)  + str[3];
	eTime.date.Date    = (str[4]  * 10)  + str[5];
	eTime.time.Hours   = (str[6]  * 10)  + str[7];
	eTime.time.Minutes = (str[8]  * 10)  + str[9];

	/* ���յ�ƽ̨���ģ��뷢��ʱ��Ƚϣ������������ʱ����ƫ���У׼�����Ӳ��ƣ�У׼�ֽڳ���Ϊ5 */
	if ((0 != memcmp(&eTime.date, &RT_RealTime.date, 3)
			|| (0 != memcmp(&eTime.time, &RT_RealTime.time, 2))))
	{
		RT_SetRealTime(&eTime);
	}
}

/*******************************************************************************
 * function�����жϻص�����
 */
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	osSignalSet(realtimeTaskHandle, REALTIME_TASK_SIGNAL_UPDATE);
}



/*******************************************************************************
 * function���ӱ������������ǰ����
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
 * function:���õ�ǰʱ�䣬������ǰʱ������ڸ��µ���������
 * time����ǰʱ��ֵ
 */
static void RT_SetRealTime(RT_TimeTypedef* time)
{
	HAL_RTC_SetDate(&RT_RTC, &time->date, RTC_FORMAT_BIN);
	HAL_RTC_SetTime(&RT_RTC, &time->time, RTC_FORMAT_BIN);

	/* �����µ����ڱ��� */
	RT_BKUP_UpdateDate(time);
	HAL_RTC_GetDate(&hrtc, &RT_RealTime.date, RTC_FORMAT_BIN);
}
