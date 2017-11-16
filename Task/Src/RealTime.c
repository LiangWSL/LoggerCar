#include "RealTime.h"

#include "iwdg.h"
#include "analog.h"
#include "tftlcd.h"
#include "rt.h"

/******************************************************************************/
extern osThreadId mainprocessTaskHandle;
extern osThreadId tftlcdTaskHandle;

uint8_t sendPackTimeCnt = 0;							/* �������ݰ��������� */
/* ��ʼ��ʱ��ʹ�ܣ��Ϳ��ڵ�һ�δ�����ʱ���¼ */
FunctionalState sendPackRecordEnable = ENABLE;			/* �������ݰ���¼ʹ�� */

/*******************************************************************************
 *
 */
void REALTIME_Task(void)
{
	osEvent signal;
	osEvent signalAnalog;

	while(1)
	{
		/* �ȴ�ʱ���ģ�������±�־�������Ӵ�����¼��־ */
		signal = osSignalWait(REALTIME_TASK_TIME_ANALOG_UPDATE | REALTIME_TASK_ALRAM_RECORD, 2000);
		if (((signal.value.signals & REALTIME_TASK_TIME_ANALOG_UPDATE) == REALTIME_TASK_TIME_ANALOG_UPDATE)
			|| ((signal.value.signals & REALTIME_TASK_ALRAM_RECORD) == REALTIME_TASK_ALRAM_RECORD))
		{
			/* ����ʱ�� */
			HAL_RTC_GetTime(&hrtc, &RT_RealTime.time, RTC_FORMAT_BIN);
			if (RT_RealTime.oldWeekDay != hrtc.DateToUpdate.WeekDay)
			{
				/* �������� */
				HAL_RTC_GetDate(&hrtc, &RT_RealTime.date, RTC_FORMAT_BIN);

				/* ���ڸ��µ����� */
				RT_BKUP_UpdateDate(&RT_RealTime);
			}
			/* ����״̬�� */
			osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_STATUS_BAR_UPDATE);

			/* ����ģ���� */
			ANALOG_ConvertEnable();
			signalAnalog = osSignalWait(REALTIME_TASK_SENSOR_CONVERT_FINISH, 2000);
			if ((signalAnalog.value.signals & REALTIME_TASK_SENSOR_CONVERT_FINISH)
							== REALTIME_TASK_SENSOR_CONVERT_FINISH)
			{
				/* ��ȡ��������ֵ */
				ANALOG_GetSensorValue();
				/* ����Һ������ʾ */
				osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_ANALOG_UPDATE);
			}

			/* ��������Ӵ��������¼���� */
			if ((signal.value.signals & REALTIME_TASK_ALRAM_RECORD) == REALTIME_TASK_ALRAM_RECORD)
			{
				sendPackTimeCnt++;
				if (sendPackTimeCnt >= 5)
				{
					sendPackTimeCnt = 0;
					sendPackRecordEnable = ENABLE;
				}
				/* ���ͼ�¼ʱ������ */
				memcpy(&RT_RecordTime, &RT_RealTime, sizeof(RT_TimeTypedef));
				/* ����MainProcess���� */
				osThreadResume(mainprocessTaskHandle);
			}
		}
#if IWDG_ENABLE
		/* ���Ź���� */
//		HAL_IWDG_Refresh(&hiwdg);
#endif

	}
}




