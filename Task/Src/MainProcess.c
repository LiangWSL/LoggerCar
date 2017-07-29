#include "../Inc/MainProcess.h"

#include "exFlash.h"
#include "analog.h"
#include "input.h"
#include "gps.h"

#include "osConfig.h"
#include "RealTime.h"



/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;
	ANALOG_ValueTypedef AnalogValue;
	RT_TimeTypedef *time;
	exFLASH_InfoTypedef sendInfo;
	GPS_LocateTypedef* location;

	while(1)
	{		
		signal = osMessageGet(realtimeMessageQId, 100);
		time = (RT_TimeTypedef*)signal.value.v;

		/* ����ADC���� */
		ANALOG_ConvertEnable();

		/* �ȴ�ADC������� */
		signal = osSignalWait(MAINPROCESS_SENSOR_CONVERT_FINISH, MAINPROCESS_TICKS_TO_TIMEOUT);
		if ((signal.value.signals & MAINPROCESS_SENSOR_CONVERT_FINISH)
						!= MAINPROCESS_SENSOR_CONVERT_FINISH)
		{
			printf("MainProcess�ȴ�ADC��������źŵȴ�ʧ��,��ʱ\r\n");
			/* ���Լ����� */
			osThreadSuspend(NULL);
		}
		
		printf("��ǰʱ����%02X.%02X.%02X %02X:%02X:%02X\r\n", time->date.Year,
				time->date.Month, time->date.Date,
				time->time.Hours,time->time.Minutes,
				time->time.Seconds);

		/* ��ȡ��������ֵ */
		ANALOG_GetSensorValue(&AnalogValue);
		printf("�¶�1 = %f\r\n", AnalogValue.temp1);
		printf("�¶�2 = %f\r\n", AnalogValue.temp2);
		printf("�¶�3 = %f\r\n", AnalogValue.temp3);
		printf("�¶�4 = %f\r\n", AnalogValue.temp4);
		printf("ʪ��1 = %f\r\n", AnalogValue.humi1);
		printf("ʪ��2 = %f\r\n", AnalogValue.humi2);
		printf("ʪ��3 = %f\r\n", AnalogValue.humi3);
		printf("ʪ��4 = %f\r\n", AnalogValue.humi4);
		printf("��ص��� = %d\r\n", AnalogValue.batVoltage);

		/* ��ȡ�ⲿ��Դ״̬ */
		/* todo */

		/* ��ȡ��λ���� */
		/* ����GPRSProcess��������GPSת�� */
		osThreadResume(gprsprocessTaskHandle);
		/* �ȴ�GPS��� */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, osWaitForever);
		if ((signal.value.signals & MAINPROCESS_GPS_CONVERT_FINISH)
						!= MAINPROCESS_GPS_CONVERT_FINISH)
		{
			printf("GPS��λʧ��\r\n");
			/* ���Լ����� */
			osThreadSuspend(NULL);
		}

		/* ��ȡ��λֵ */
		signal = osMessageGet(infoMessageQId, 100);
		location = (GPS_LocateTypedef*)signal.value.v;

		/* ��¼��ֵ */
		exFLASH_SaveStructInfo(&sendInfo, time, &AnalogValue, location);

		/* ��ȡ��ֵ */
//		exFLASH_ReadStructInfo(&flashInfo);

		/* ͨ��GPRS�ϴ���ƽ̨ */
		/* ���ݷ��ͽṹ�� */
		osMessagePut(infoMessageQId, (uint32_t)&sendInfo, 100);

		/* �ѵ�ǰʱ�䴫�ݵ�GPRS���̣����ݻ���У׼ʱ�� */
		osMessagePut(realtimeMessageQId, (uint32_t)time, 100);

		/* ����MainProcess���� */
		osThreadResume(gprsprocessTaskHandle);

		/* �ȴ�GPRSProcess��� */
		signal = osSignalWait(MAINPROCESS_GPRS_SEND_FINISHED, osWaitForever);
		if ((signal.value.signals & MAINPROCESS_GPRS_SEND_FINISHED)
						!= MAINPROCESS_GPRS_SEND_FINISHED)
		{
			printf("MainProcess�ȴ�GPRSProcess��� �źŵȴ�ʧ��,��ʱ\r\n");
			/* ���Լ����� */
			osThreadSuspend(NULL);
		}

		/* ����������ϣ�һ��Ҫ���Լ����� */
		osThreadSuspend(NULL);
	}
}


