#include "RealTime.h"

#include "osConfig.h"
#include "Mainprocess.h"

#include "analog.h"
#include "file.h"
#include "analog.h"
#include "tftlcd.h"

#include "rt.h"

/******************************************************************************/
void REALTIME_Task(void)
{
	osEvent signal;
	RT_TimeTypedef sTime;

	while(1)
	{
		signal = osSignalWait(REALTIME_TASK_SIGNAL_UPDATE, 2000);

		/* �յ�ʹ���¼����־λ */
		if ((signal.value.signals & REALTIME_TASK_SIGNAL_UPDATE)
				== REALTIME_TASK_SIGNAL_UPDATE)
		{
			HAL_RTC_GetTime(&hrtc, &RT_RealTime.time, RTC_FORMAT_BIN);
			if (RT_RealTime.oldWeekDay != hrtc.DateToUpdate.WeekDay)
			{
				/* �������� */
				HAL_RTC_GetDate(&hrtc, &RT_RealTime.date, RTC_FORMAT_BIN);

				/* ���ڸ��µ����� */
				RT_BKUP_UpdateDate(&RT_RealTime);
			}

			/* ÿ������ʪ�Ȳ���һ�� */
			if (RT_RealTime.time.Seconds % 30 == 0)
			{
				/* ����ADC���� */
				ANALOG_ConvertEnable();

				/* �ȴ�ADC������� */
				signal = osSignalWait(REALTIME_SENSOR_CONVERT_FINISH, 2000);
				if ((signal.value.signals & REALTIME_SENSOR_CONVERT_FINISH)
								== REALTIME_SENSOR_CONVERT_FINISH)
				{
					/* ��ȡ��������ֵ */
					ANALOG_GetSensorValue();
					/* ����Һ������ʾ */
					if (TFTLCD_status.curScreenID == SCREEN_ID_CUR_DATA_8CH)
					{
						osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_ANALOG_UPDATE);
					}

					/* �����¼���ʱ�䵽���򴥷���¼ */
					if (RT_RealTime.time.Seconds % 60 == 0)
					{
						/* ��ʱ�䴫�ݵ�GPRS���̣����ڸ���ƽ̨����У׼ʱ�� */
						osMessagePut(adjustTimeMessageQId, (uint32_t)&RT_RealTime, 1000);

						/* ���ͼ�¼ʱ������,�Ȱ�ʱ��ת����BCDģʽ */
						HEX2BCD((uint8_t*)&RT_RealTime, (uint8_t*)&sTime, sizeof(RT_TimeTypedef));
						osMessagePut(realtimeMessageQId, (uint32_t)&sTime, 100);

						/* ����ģ�������� */
						osMessagePut(analogMessageQId, (uint32_t)&ANALOG_value, 100);

						/* ����MainProcess���� */
						osThreadResume(mainprocessTaskHandle);

						/* ����״̬�� */
						osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_STATUS_BAR_UPDATE);
					}
				}



			}
		}
	}
}




