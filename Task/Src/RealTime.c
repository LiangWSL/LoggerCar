#include "RealTime.h"

#include "osConfig.h"
#include "analog.h"
#include "eeprom.h"
#include "analog.h"
#include "tftlcd.h"

/******************************************************************************/
void REALTIME_Task(void)
{
	osEvent signal;
	RT_TimeTypedef realTime;
	ANALOG_ValueTypedef AnalogValue;

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
			TFTLCD_RealtimeRefresh(&realTime);

			/* ÿ������ʪ�Ȳ���һ�� */
			if (realTime.time.Seconds == 0)
			{
				/* ����ADC���� */
				ANALOG_ConvertEnable();

				/* �ȴ�ADC������� */
				signal = osSignalWait(REALTIME_SENSOR_CONVERT_FINISH, 2000);
				if ((signal.value.signals & REALTIME_SENSOR_CONVERT_FINISH)
								!= REALTIME_SENSOR_CONVERT_FINISH)
				{
					printf("ADC�����źŵȴ���ʱ������\r\n");
				}
				else
				{
					/* ��ȡ��������ֵ */
					ANALOG_GetSensorValue(&AnalogValue);
					printf("ģ����ת����ɣ�����\r\n");

					/* �����¼���ʱ�䵽���򴥷���¼ */
					if (realTime.time.Minutes % EE_recordInterval == 0)
					{
						/* ���ͼ�¼ʱ������ */
						osMessagePut(realtimeMessageQId, (uint32_t)&realTime, 100);

						/* ����ģ�������� */
						osMessagePut(analogMessageQId, (uint32_t)&AnalogValue, 100);

						/* ����MainProcess���� */
						osThreadResume(mainprocessTaskHandle);
					}

					/* ����Һ������ʾ */
					/* todo */
					TFTLCD_AnalogDataRefresh(&AnalogValue);
				}
			}
		}
		else
		{
			printf("���жϳ�ʱ\r\n");
		}

	}
}




