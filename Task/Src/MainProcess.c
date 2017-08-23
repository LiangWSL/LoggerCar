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
	uint16_t curPatchPack;				/* ���β������� */
	uint16_t curFileStructCount;		/* ��ǰ�ļ��ṹ������ */



	while(1)
	{		
		/* ��ȡʱ�� */
		signal = osMessageGet(realtimeMessageQId, 1000);
		memcpy(&time, (uint32_t*)signal.value.v, sizeof(RT_TimeTypedef));

		/* ��ȡģ�������� */
		signal = osMessageGet(analogMessageQId, 1000);
		AnalogValue = (ANALOG_ValueTypedef*)signal.value.v;

		/* ��ȡ��λ���� */
		/* ����GPRSProcess��������GPSת�� */
//		osThreadResume(gprsprocessTaskHandle);
//		osSignalSet(gprsprocessTaskHandle, GPRSPROCESS_GPS_ENABLE);

		osMessagePut(gprsTaskMessageQid, START_TASK_GPS, 1000);

		/* �ȴ�GPS���,��Ϊ������̿���Ҫ����GSMģ�飬���Եȴ����ڱ��볤�㣬30s */
		/* ������ģ��ĵ�һ�ζ�λ��ʱ����Ҳ���ܵõ���λ���ݣ����Է��� */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, 30000);
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

		printf("��ǰ��¼ʱ����%02x.%02x.%02x %02x:%02x:%02x \r\n",
				time.date.Year,  time.date.Month,   time.date.Date,
				time.time.Hours, time.time.Minutes, time.time.Seconds);

		printf("ģ���������ǣ�%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\r\n,",
				AnalogValue->temp1, AnalogValue->temp2, AnalogValue->temp3, AnalogValue->temp4,
				AnalogValue->humi1, AnalogValue->humi2, AnalogValue->humi3, AnalogValue->humi4);

		/* ��¼���� */
		FILE_SaveInfo(&saveInfo, &curFileStructCount);

		/* ��ȡ������������ */
		/* ��ȡ�ɹ�������������в������ݼ�¼ */
		if (SUCCESS == FILE_ReadPatchPackFile(&patchPack))
		{
			/* ��������ȫ���ϴ���� */
			if (memcmp(patchPack.patchFileName, "\0\0\0\0\0\0", 6) != 0)
			{
				printf("��ȡ�����ļ�����%11s,������ʼ�ṹ��ƫ��=%d \r\n",
						patchPack.patchFileName, patchPack.patchStructOffset);
				curPatchPack = FILE_ReadPatchInfo(&patchPack, readInfo);
			}
			else
			{
				FILE_ReadInfo(readInfo);
				curPatchPack = 1;
			}
		}
		/* ��ȡ�ļ����ɹ�����˵�����ļ���δ���� */
		else
		{
			FILE_ReadInfo(readInfo);
			curPatchPack = 1;
		}
		printf("�����ϴ���������=%d\r\n", curPatchPack);

		/* ͨ��GPRS�ϴ���ƽ̨ */
		/* ���ݷ��ͽṹ�� */
		osMessagePut(infoMessageQId, (uint32_t)&readInfo, 1000);

		/* ���ݱ��η��͵�����������ע�⣺curPatchPack����������ʽ���ݣ����Ǵ���ָ�� */
		osMessagePut(infoCntMessageQId, (uint16_t)curPatchPack, 1000);

		/* ʹ��MainProcess���������� */
//		osSignalSet(gprsprocessTaskHandle, GPRSPROCESS_SEND_DATA_ENABLE);
		osMessagePut(gprsTaskMessageQid, START_TASK_GPRS, 1000);

		/* �ȴ�GPRSProcess��� */
		signal = osSignalWait(MAINPROCESS_GPRS_SEND_FINISHED, 30000);
		if ((signal.value.signals & MAINPROCESS_GPRS_SEND_FINISHED)
						!= MAINPROCESS_GPRS_SEND_FINISHED)
		{
			printf("�������ݳ�ʱ��˵�����ݷ���ʧ�ܣ���¼���ݵȴ�����\r\n");

			/* ����Ǳ��η����Ĳ��������¼ʱ�䣬���������ڲ����Ĺ��̣����ֲ����ļ����ݲ��� */
			if (curPatchPack == 1)
			{
				/* ����ʱ�������ļ��� */
				BCD2ASCII(&patchPack.patchFileName[0], &time.date.Year,  1);
				BCD2ASCII(&patchPack.patchFileName[2], &time.date.Month, 1);
				BCD2ASCII(&patchPack.patchFileName[4], &time.date.Date,  1);
				/* ������Ϣ�ļ�����׺ */
				memcpy(&patchPack.patchFileName[6], ".txt\0", 5);

				/* ��¼��ǰ�ļ�ǰһ��λ�� */
				patchPack.patchStructOffset = curFileStructCount - 1;

				FILE_WritePatchPackFile(&patchPack);
			}
		}
		else
		{
			/* ���ݷ��ͳɹ� */
			printf("���ݷ��͵�ƽ̨�ɹ�����\r\n");

			/* �в������� */
			if (curPatchPack > 1)
				FILE_WritePatchPackFile(&patchPack);
		}

		osThreadSuspend(NULL);
	}
}


