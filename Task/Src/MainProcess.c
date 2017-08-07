#include "../Inc/MainProcess.h"

#include "exFlash.h"
#include "analog.h"
#include "input.h"
#include "gps.h"
#include "file.h"

#include "osConfig.h"
#include "RealTime.h"
#include "GPRSProcess.h"

/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;

	RT_TimeTypedef time;
	GPS_LocateTypedef* location;
	ANALOG_ValueTypedef* AnalogValue;

	FILE_InfoTypedef saveInfo;
	FILE_InfoTypedef readInfo[GPRS_PATCH_PACK_NUMB_MAX];

	FILE_PatchPackTypedef patchPack;
	uint16_t curPatchPack;			/* ���β������� */

	

	while(1)
	{		
		/* ��ȡʱ�� */
		signal = osMessageGet(realtimeMessageQId, 100);
		memcpy(&time, (uint32_t*)signal.value.v, sizeof(RT_TimeTypedef));

		/* ��ȡģ�������� */
		signal = osMessageGet(analogMessageQId, 100);
		AnalogValue = (ANALOG_ValueTypedef*)signal.value.v;

		printf("��ǰʱ����%02X.%02X.%02X %02X:%02X:%02X\r\n", time.date.Year,
				time.date.Month, time.date.Date,
				time.time.Hours,time.time.Minutes,
				time.time.Seconds);

		printf("�¶�1 = %f\r\n", AnalogValue->temp1);
		printf("�¶�2 = %f\r\n", AnalogValue->temp2);
		printf("�¶�3 = %f\r\n", AnalogValue->temp3);
		printf("�¶�4 = %f\r\n", AnalogValue->temp4);
		printf("ʪ��1 = %f\r\n", AnalogValue->humi1);
		printf("ʪ��2 = %f\r\n", AnalogValue->humi2);
		printf("ʪ��3 = %f\r\n", AnalogValue->humi3);
		printf("ʪ��4 = %f\r\n", AnalogValue->humi4);
		printf("��ص��� = %d\r\n", AnalogValue->batVoltage);

		/* ��ȡ��λ���� */
		/* ����GPRSProcess��������GPSת�� */
		osThreadResume(gprsprocessTaskHandle);
 		osSignalSet(gprsprocessTaskHandle, GPRSPROCESS_GPS_ENABLE);
		
		/* �ȴ�GPS���,��Ϊ������̿���Ҫ����GSMģ�飬���Եȴ����ڱ��볤�㣬60s */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, 60000);
		if ((signal.value.signals & MAINPROCESS_GPS_CONVERT_FINISH)
						!= MAINPROCESS_GPS_CONVERT_FINISH)
		{
			printf("GPS��λʧ��\r\n");
		}
		else
		{
			/* ��ȡ��λֵ */
			signal = osMessageGet(infoMessageQId, 100);
			location = (GPS_LocateTypedef*)signal.value.v;
		}

		/* �����ݸ�ʽת����Э���ʽ */
		FILE_InfoFormatConvert(&saveInfo, &time, location, AnalogValue);

		/* ��ȡ������������ */
		FILE_ReadPatchPackFile(&patchPack);
		/* �������ݲ�Ϊ0��Ҳ���Ǵ��̸�ʽ����ȫFFFF */
		if ((patchPack.patchPackNumb != 0) && (patchPack.patchPackNumb != 0xFFFF))
		{
			/* ���������ݴ��ڵ�����󲹴����� */
			if (patchPack.patchPackNumb >= GPRS_PATCH_PACK_NUMB_MAX)
				curPatchPack = GPRS_PATCH_PACK_NUMB_MAX;
			else
				/* С����󲹴���������ѵ�ǰ��������Ҳ�ϴ���ȥ */
				curPatchPack = patchPack.patchPackNumb + 1;
		}
		else
		{
			/* û����Ҫ���������ݣ����͵�����Ϊ1 */
			curPatchPack = 1;

			/* ����patchPack.patchPackNumb��ֵΪ0xFFFF */
			patchPack.patchPackNumb = 0;
		}
		printf("������Ҫ����������Ϊ%d��", patchPack.patchPackNumb);

		/* ���沢��ȡ���� */
		FILE_SaveReadInfo(&saveInfo, readInfo, curPatchPack);

		/* ͨ��GPRS�ϴ���ƽ̨ */
		/* ���ݷ��ͽṹ�� */
		osMessagePut(infoMessageQId, (uint32_t)&readInfo, 100);

		/* ���ݱ��η��͵�����������ע�⣺curPatchPack����������ʽ���ݣ����Ǵ���ָ�� */
		osMessagePut(infoCntMessageQId, (uint16_t)curPatchPack, 100);

		/* �ѵ�ǰʱ�䴫�ݵ�GPRS���̣����ݻ���У׼ʱ�� */
		osMessagePut(realtimeMessageQId, (uint32_t)&time, 100);

		/* ʹ��MainProcess���������� */
		osSignalSet(gprsprocessTaskHandle, GPRSPROCESS_SEND_DATA_ENABLE);

		/* �ȴ�GPRSProcess��� */
		signal = osSignalWait(MAINPROCESS_GPRS_SEND_FINISHED, 10000);
		if ((signal.value.signals & MAINPROCESS_GPRS_SEND_FINISHED)
						!= MAINPROCESS_GPRS_SEND_FINISHED)
		{
			printf("�������ݳ�ʱ��˵�����ݷ���ʧ�ܣ���¼���ݵȴ�����\r\n");

			/* �����ϴ�ʧ�ܣ���־λҪ��գ������´�ֱ�Ӵ��� */
//			osSignalClear(gprsprocessTaskHandle, GPRSPROCESS_SEND_DATA_ENABLE);

			/* ��¼������������ */
			/* ����ʧ�ܣ��򲹴�����+���μ�¼��һ������ */
			patchPack.patchPackNumb++;

			FILE_WritePatchPackFile(&patchPack);
		}
		else
		{
			/* ���ݷ��ͳɹ� */
			printf("���ݷ��͵�ƽ̨�ɹ�����\r\n");

			/* �в������� */
			if (curPatchPack > 1)
			{
				if (curPatchPack >= GPRS_PATCH_PACK_NUMB_MAX)
				{
					/* ����30�����ݴ��� */
					patchPack.patchPackOver_30++;
					patchPack.patchPackNumb -= curPatchPack;
				}
				else
				{
					if (curPatchPack >= 20)
						patchPack.patchPackOver_20++;
					else if (curPatchPack >= 10)
						patchPack.patchPackOver_10++;
					else if (curPatchPack >= 5)
						patchPack.patchPackOver_5++;

					/* �Ѿ�����ȫ������ */
					patchPack.patchPackNumb = 0;
				}

				FILE_WritePatchPackFile(&patchPack);
			}
		}

		/* ����������ϣ�һ��Ҫ���Լ����� */
		osThreadSuspend(NULL);
	}
}


