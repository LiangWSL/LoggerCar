#include "../Inc/MainProcess.h"

#include "osConfig.h"
#include "analog.h"
#include "RealTime.h"

#include "exFlash.h"

/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;
	ANALOG_ValueTypedef AnalogValue;
	RT_TimeTypedef *time;
	exFLASH_InfoTypedef flashInfo;

//	exFLASH_ChipErase();

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
			printf("MainProcess�źŵȴ�ʧ��,��ʱ\r\n");
			/* ���Լ����� */
			osThreadSuspend(NULL);
		}
		
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
		/* todo */

		/* ���źŶ����л�ȡʱ��ֵ */
		/* todo */

		/* ��¼��ֵ */
//		exFLASH_SaveStructInfo(time, &AnalogValue, FORMAT_ONE_DECIMAL);

		/* ��ȡ��ֵ */
//		exFLASH_ReadStructInfo(&flashInfo);
//		printf("��ǰʱ����%02X.%02X.%02X %02X:%02X:%02X\r\n", time->date.Year,
//				time->date.Month, time->date.Date,
//				time->time.Hours,time->time.Minutes,
//				time->time.Seconds);
//		printf("����ʱ����%02X.%02X.%02X %02X:%02X:%02X\r\n", flashInfo.realTime.year,
//				flashInfo.realTime.month,flashInfo.realTime.day,
//				flashInfo.realTime.hour,flashInfo.realTime.min,
//				flashInfo.realTime.sec);

		/* ͨ��GPRS�ϴ���ƽ̨ */
		/* todo */

		/* ����������ϣ�һ��Ҫ���Լ����� */
		osThreadSuspend(NULL);

	}
}


