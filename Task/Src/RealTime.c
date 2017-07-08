#include "RealTime.h"
#include "osConfig.h"

/******************************************************************************/
void REALTIME_Task(void)
{
	osEvent signal;
	RT_TimeTypedef realTime;

	RT_Init(&realTime);

	while(1)
	{
		signal = osSignalWait(REALTIME_TASK_SIGNAL_UPDATE, 2000);

		/* �յ�ʹ���¼����־λ */
		if ((signal.value.signals & REALTIME_TASK_SIGNAL_UPDATE)
				== REALTIME_TASK_SIGNAL_UPDATE)
		{
			HAL_RTC_GetTime(&hrtc, &realTime.time, RTC_FORMAT_BCD);
			if (realTime.oldWeekDay != hrtc.DateToUpdate.WeekDay)
			{
				/* �������� */
				HAL_RTC_GetDate(&hrtc, &realTime.date, RTC_FORMAT_BCD);

				/* ���ڸ��µ����� */
				RT_BKUP_UpdateDate(&realTime);
			}

			if ((realTime.time.Seconds == 0)
				&& (realTime.time.Minutes % REALTIME_SAVE_INTERVAL == 0))
			{
				osMessagePut(realtimeMessageQId, (uint32_t)&realTime, 100);

				/* ����MainProcess���� */
				osThreadResume(mainprocessTaskHandle);
			}
		}
		else
		{
			printf("���жϳ�ʱ\r\n");
		}

	}
}




