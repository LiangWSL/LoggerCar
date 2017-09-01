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
//	ANALOG_ValueTypedef* AnalogValue;

//	FILE_InfoTypedef saveInfo;


	FILE_PatchPackTypedef patchPack;
	uint16_t curPatchPack;				/* ���β������� */
	uint64_t curFileStructCount;		/* ��ǰ�ļ��ṹ������ */

	FILE_SaveInfoTypedef saveInfoStruct;		/* ����Ľṹ����� */
	FILE_SaveInfoTypedef readInfoStruct[GPRS_PATCH_PACK_NUMB_MAX];

	while(1)
	{		
		/* ��ȡʱ�� */
		signal = osMessageGet(realtimeMessageQId, 1000);
		memcpy(&time, (uint32_t*)signal.value.v, sizeof(RT_TimeTypedef));

		/* ʱ��ת����ASCII */
		HEX2ASCII(&time.date.Year,  (uint8_t*)&saveInfoStruct.year[0],  1);
		HEX2ASCII(&time.date.Month, (uint8_t*)&saveInfoStruct.month[0], 1);
		HEX2ASCII(&time.date.Date,  (uint8_t*)&saveInfoStruct.day[0],   1);
		HEX2ASCII(&time.time.Hours, (uint8_t*)&saveInfoStruct.hour[0],  3);

		/* ģ����ת��ΪASCII */
		sprintf((char*)&saveInfoStruct.analogValue[0].value, "%5.1f", ANALOG_value.temp1);
		sprintf((char*)&saveInfoStruct.analogValue[1].value, "%5.1f", ANALOG_value.humi1);
		sprintf((char*)&saveInfoStruct.analogValue[2].value, "%5.1f", ANALOG_value.temp2);
		sprintf((char*)&saveInfoStruct.analogValue[3].value, "%5.1f", ANALOG_value.humi2);
		sprintf((char*)&saveInfoStruct.analogValue[4].value, "%5.1f", ANALOG_value.temp3);
		sprintf((char*)&saveInfoStruct.analogValue[5].value, "%5.1f", ANALOG_value.humi3);
		sprintf((char*)&saveInfoStruct.analogValue[6].value, "%5.1f", ANALOG_value.temp4);
		sprintf((char*)&saveInfoStruct.analogValue[7].value, "%5.1f", ANALOG_value.humi4);
		sprintf((char*)&saveInfoStruct.batQuality[0],        "%3d",   ANALOG_value.batVoltage);

		/* ��ȡ��λ���� */
		osMessagePut(gprsTaskMessageQid, START_TASK_GPS, 1000);

		/* �ȴ�GPS���,��Ϊ������̿���Ҫ����GSMģ�飬���Եȴ����ڱ��볤�㣬30s */
		/* ������ģ��ĵ�һ�ζ�λ��ʱ����Ҳ���ܵõ���λ���ݣ����Է��� */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, 30000);
		if ((signal.value.signals & MAINPROCESS_GPS_CONVERT_FINISH)
						== MAINPROCESS_GPS_CONVERT_FINISH)
		{
			/* ��ȡ��λֵ */
			signal = osMessageGet(infoMessageQId, 100);
			location = (GPS_LocateTypedef*)signal.value.v;

			/* ��λֵת����ASCII */
			sprintf((char*)&saveInfoStruct.longitude[0], "%10.5f", location->longitude);
			sprintf((char*)&saveInfoStruct.latitude[0],  "%9.5f",  location->latitude);
		}

		/* ������Ϣ���ų�ʼ�� */
		FILE_SaveInfoSymbolInit(&saveInfoStruct);

		/* ��¼���� */
		FILE_SaveInfo(&saveInfoStruct, &curFileStructCount);

		/* ��ȡ������������ */
		/* ��ȡ�ɹ�������������в������ݼ�¼ */
//		if (SUCCESS == FILE_ReadPatchPackFile(&patchPack))
//		{
//			/* ��������ȫ���ϴ���� */
//			if (memcmp(patchPack.patchFileName, "\0\0\0\0\0\0", 6) != 0)
//			{
//				printf("��ȡ�����ļ�����%11s,������ʼ�ṹ��ƫ��=%d \r\n",
//						patchPack.patchFileName, patchPack.patchStructOffset);
////				curPatchPack = FILE_ReadPatchInfo(&patchPack, readInfoStruct);
//			}
//			else
//			{
//				FILE_ReadInfo(readInfoStruct);
//				curPatchPack = 1;
//			}
//		}
		/* ��ȡ�ļ����ɹ�����˵�����ļ���δ���� */
		else
		{
			FILE_ReadInfo(readInfoStruct);
			curPatchPack = 1;
		}
		printf("�����ϴ���������=%d\r\n", curPatchPack);

		FILE_SendInfoFormatConvert(readInfoStruct, &GPRS_SendBuffer.dataPack[0], curPatchPack);

		/* ͨ��GPRS�ϴ���ƽ̨ */
		/* ���ݷ��ͽṹ�� */
//		osMessagePut(infoMessageQId, (uint32_t)&readInfoStruct, 1000);

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


