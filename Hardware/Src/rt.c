#include "rt.h"

#include "../Inc/public.h"
#include "RealTime.h"
#include "param.h"

/******************************************************************************/
RT_TimeTypedef RT_RealTime;						/* ʵʱʱ�� */
RT_TimeTypedef RT_RecordTime;					/* ��¼ʱ�� */

/******************************************************************************/
extern PARAM_DeviceParamTypedef PARAM_DeviceParam;
extern osThreadId realtimeTaskHandle;

/******************************************************************************/
static void RT_SetRealTime(RT_TimeTypedef* time);
static void RT_BKUP_ReadDate(void);
static void RT_SetAlarmTimeInterval(uint8_t interval);

/*******************************************************************************
 * function��ʵʱʱ�ӳ�ʼ��
 */
void RT_Init(void)
{
	RT_BKUP_ReadDate();
	HAL_RTC_GetDate(&RT_RTC, &RT_RealTime.date, RTC_FORMAT_BIN);

	RT_SetAlarmTimeInterval(PARAM_DeviceParam.recordInterval);
	/* ��������жϱ�־λ */
	__HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
	/* ���������ж� */
	__HAL_RTC_ALARM_ENABLE_IT(&hrtc,RTC_IT_ALRA);
	/* ����EXTI�ж� */
	__HAL_RTC_ALARM_EXTI_ENABLE_IT();
	/* EXTI�����ز����ж� */
	__HAL_RTC_ALARM_EXTI_ENABLE_RISING_EDGE();

	/* ʹ�ܶ�ʱ�����жϣ��������� */
	__HAL_TIM_ENABLE(&RT_TIM);
	__HAL_TIM_ENABLE_IT(&RT_TIM, TIM_IT_UPDATE);
	/* ��������һ�β��� */
	osSignalSet(realtimeTaskHandle, REALTIME_TASK_TIME_ANALOG_UPDATE);
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
	uint8_t str[12] = {0};

	RT_TimeTypedef   eTime;

	/* ���ַ�ת�������� */
	str2numb(str, (pBuffer + RT_OFFSET_CLOUD_TIME), sizeof(str));

	eTime.date.Year    = (str[0]  * 10)   + str[1];
	eTime.date.Month   = (str[2]  * 10)   + str[3];
	eTime.date.Date    = (str[4]  * 10)   + str[5];
	eTime.time.Hours   = (str[6]  * 10)   + str[7];
	eTime.time.Minutes = (str[8]  * 10)   + str[9];
	eTime.time.Seconds = (str[10]  * 10)  + str[11];

	/* ���յ�ƽ̨���ģ��뷢��ʱ��Ƚϣ������������ʱ����ƫ���У׼�����Ӳ��ƣ�У׼�ֽڳ���Ϊ5 */
	if ((0 != memcmp(&eTime.date, &RT_RealTime.date, 3)
			|| (0 != memcmp(&eTime.time, &RT_RealTime.time, 2))))
	{
		RT_SetRealTime(&eTime);
	}
}

/*******************************************************************************
 * @brief �����жϻص������������´�����ʱ���𣬴�����¼
 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	/* �����´μ�¼ʱ��� */
	RT_SetAlarmTimeInterval(PARAM_DeviceParam.recordInterval);
	osSignalSet(realtimeTaskHandle, REALTIME_TASK_ALRAM_RECORD);
}

/*******************************************************************************
 * @brief ��������ʱ����
 * @param interval����һ�����Ӽ�� ��λ������
 */
static void RT_SetAlarmTimeInterval(uint8_t interval)
{
	uint16_t high = 0U, low = 0U;
	uint32_t alarmCounter = 0U;

	high = READ_REG(hrtc.Instance->CNTH & RTC_CNTH_RTC_CNT);
	low  = READ_REG(hrtc.Instance->CNTL & RTC_CNTL_RTC_CNT);

	alarmCounter = (((uint32_t) high << 16U) | low);

	/* ����ת��Ϊ�� */
	alarmCounter += (interval * 60);

	/* �ȴ��ϴζ�RTCд������� */
	while((hrtc.Instance->CRL & RTC_CRL_RTOFF) == (uint32_t)RESET);
	/* ��������ģʽ */
	__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);

	WRITE_REG(hrtc.Instance->ALRH, (alarmCounter >> 16U));
	WRITE_REG(hrtc.Instance->ALRL, (alarmCounter & RTC_ALRL_RTC_ALR));

	/* �˳�����ģʽ */
	__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
	while((hrtc.Instance->CRL & RTC_CRL_RTOFF) == (uint32_t)RESET);
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
