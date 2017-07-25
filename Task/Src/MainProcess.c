#include "../Inc/MainProcess.h"

#include "exFlash.h"
#include "analog.h"
#include "input.h"

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
//	exFLASH_InfoTypedef sendInfo;


	while(1)
	{		
		signal = osMessageGet(realtimeMessageQId, 100);
		time = (RT_TimeTypedef*)signal.value.v;

		/* ����ADC���� */
		ANALOG_ConvertEnable();

		/* �ȴ�ADC������� */
		signal = osSignalWait(MAINPROCESS_GET_SENSOR_ENABLE, 1000);
		if ((signal.value.signals & MAINPROCESS_GET_SENSOR_ENABLE)
						!= MAINPROCESS_GET_SENSOR_ENABLE)
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

#if 0
		/* ��ȡ�ⲿ��Դ״̬ */
		/* todo */


		/* ��ȡ��λ���� */
		/* todo */

		/* ��¼��ֵ */
		exFLASH_SaveStructInfo(&sendInfo, time, &AnalogValue, FORMAT_ONE_DECIMAL);

		/* ��ȡ��ֵ */
//		exFLASH_ReadStructInfo(&flashInfo);

		/* ͨ��GPRS�ϴ���ƽ̨ */
		/* ���ݷ��ͽṹ�� */
		osMessagePut(infoMessageQId, (uint32_t)&sendInfo, 100);
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

#endif
		/* ����������ϣ�һ��Ҫ���Լ����� */
		osThreadSuspend(NULL);
	}
}


