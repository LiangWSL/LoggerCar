#include "RealTime.h"

#include "osConfig.h"
#include "Mainprocess.h"

#include "analog.h"
#include "file.h"
#include "analog.h"
#include "tftlcd.h"

/******************************************************************************/
void REALTIME_Task(void)
{
	osEvent signal;
	RT_TimeTypedef realTime, sTime;
	ANALOG_ValueTypedef AnalogValue;

	RT_Init(&realTime);


	while(1)
	{
		signal = osSignalWait(REALTIME_TASK_SIGNAL_UPDATE, 2000);

		/* �յ�ʹ���¼����־λ */
		if ((signal.value.signals & REALTIME_TASK_SIGNAL_UPDATE)
				== REALTIME_TASK_SIGNAL_UPDATE)
		{
			HAL_RTC_GetTime(&hrtc, &realTime.time, RTC_FORMAT_BIN);
			if (realTime.oldWeekDay != hrtc.DateToUpdate.WeekDay)
			{
				/* �������� */
				HAL_RTC_GetDate(&hrtc, &realTime.date, RTC_FORMAT_BIN);

				/* ���ڸ��µ����� */
				RT_BKUP_UpdateDate(&realTime);
			}
			TFTLCD_RealtimeRefresh(&realTime);

			/* ÿ������ʪ�Ȳ���һ�� */
			if (realTime.time.Seconds % 30 == 0)
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
					/* ����Һ������ʾ */
					TFTLCD_AnalogDataRefresh(&AnalogValue);
				}
			}

			/* �����¼���ʱ�䵽���򴥷���¼ */
//			if (realTime.time.Minutes % FILE_DeviceParam.recordInterval == 0)
			if ((realTime.time.Minutes % 5 == 0)
					&& (realTime.time.Seconds % 60 == 0))
			{
				/* ��ʱ�䴫�ݵ�GPRS���̣����ڸ���ƽ̨����У׼ʱ�� */
				osMessagePut(adjustTimeMessageQId, (uint32_t)&realTime, 1000);

				/* ���ͼ�¼ʱ������,�Ȱ�ʱ��ת����BCDģʽ */
				HEX2BCD((uint8_t*)&realTime, (uint8_t*)&sTime, sizeof(RT_TimeTypedef));
				osMessagePut(realtimeMessageQId, (uint32_t)&sTime, 100);

				/* ����ģ�������� */
				osMessagePut(analogMessageQId, (uint32_t)&AnalogValue, 100);

				/* ����MainProcess���� */
				osThreadResume(mainprocessTaskHandle);
			}
		}
	}
}




