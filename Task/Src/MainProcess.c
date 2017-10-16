#include "../Inc/MainProcess.h"
#include "public.h"
#include "osConfig.h"

#include "GPRSProcess.h"

#include "rt.h"
#include "gps.h"
#include "file.h"
#include "input.h"

/******************************************************************************/
extern FILE_SaveStructTypedef FILE_SaveStruct;
extern FILE_SaveStructTypedef FILE_ReadStruct[GPRS_PATCH_PACK_NUMB_MAX];
extern FILE_PatchPackTypedef FILE_PatchPack;		/* �����ļ���Ϣ */

/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;
	uint8_t curPatchPack;

	RT_TimeTypedef time;
	GPS_LocateTypedef* location;

	while(1)
	{		
		/* ��ȡʱ�� */
		signal = osMessageGet(realtimeMessageQId, 1000);
		memcpy(&time, (uint32_t*)signal.value.v, sizeof(RT_TimeTypedef));

		printf("ʱ�䣺%d.%d.%d %d:%d:%d\r\n", RT_RecordTime.date.Year, RT_RecordTime.date.Month,
				RT_RecordTime.date.Date, RT_RecordTime.time.Hours, RT_RecordTime.time.Minutes, RT_RecordTime.time.Seconds);

		/* ���� */
//		osThreadResume(gprsprocessTaskHandle);
		/* ��ȡ��λ���� */
		osMessagePut(gprsTaskMessageQid, START_TASK_GPS, 1000);
		/* �ȴ�GPS���,��Ϊ������̿���Ҫ����GSMģ�飬���Եȴ����ڱ��볤�㣬20s */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, 20000);
		if ((signal.value.signals & MAINPROCESS_GPS_CONVERT_FINISH)
						== MAINPROCESS_GPS_CONVERT_FINISH)
		{
			/* ��ȡ��λֵ */
			signal = osMessageGet(infoMessageQId, 100);
			location = (GPS_LocateTypedef*)signal.value.v;
		}

		/* ʱ��ת����ASCII */
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.year[0],  &RT_RecordTime.date.Year,    1);
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.month[0], &RT_RecordTime.date.Month,   1);
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.day[0],   &RT_RecordTime.date.Date,    1);
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.hour[0],  &RT_RecordTime.time.Hours,   1);
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.min[0],   &RT_RecordTime.time.Minutes, 1);
		HEX2ASCII((uint8_t*)&FILE_SaveStruct.sec[0],   &RT_RecordTime.time.Seconds, 1);
		/* ��ȡ�ⲿ��Դ״̬ */
		FILE_SaveStruct.exPwrStatus = INPUT_CheckPwrOnStatus() + '0';
		/* ģ����ת��ΪASCII */
		sprintf((char*)&FILE_SaveStruct.analogValue[0].value, "%5.1f", ANALOG_value.temp1);
		sprintf((char*)&FILE_SaveStruct.analogValue[1].value, "%5.1f", ANALOG_value.humi1);
		sprintf((char*)&FILE_SaveStruct.analogValue[2].value, "%5.1f", ANALOG_value.temp2);
		sprintf((char*)&FILE_SaveStruct.analogValue[3].value, "%5.1f", ANALOG_value.humi2);
		sprintf((char*)&FILE_SaveStruct.analogValue[4].value, "%5.1f", ANALOG_value.temp3);
		sprintf((char*)&FILE_SaveStruct.analogValue[5].value, "%5.1f", ANALOG_value.humi3);
		sprintf((char*)&FILE_SaveStruct.analogValue[6].value, "%5.1f", ANALOG_value.temp4);
		sprintf((char*)&FILE_SaveStruct.analogValue[7].value, "%5.1f", ANALOG_value.humi4);
//		ANALOG_Float2ASCII((char*)&FILE_SaveStruct.analogValue[0].value, ANALOG_value.temp1);
		sprintf((char*)&FILE_SaveStruct.batQuality[0],        "%3d",   ANALOG_value.batVoltage);
		/* ��λֵת����ASCII */
		sprintf((char*)&FILE_SaveStruct.longitude[0], "%10.5f", location->longitude);
		sprintf((char*)&FILE_SaveStruct.latitude[0],  "%10.5f",  location->latitude);
		/* CVS�ļ���ʽ */
		FILE_SaveStruct.batQuality[3]	   = '%';		/* ��ص����ٷֺ� */
		FILE_SaveStruct.str7   			   = ',';
		FILE_SaveStruct.str8   			   = ',';
		FILE_SaveStruct.analogValue[0].str = ',';
		FILE_SaveStruct.analogValue[1].str = ',';
		FILE_SaveStruct.analogValue[2].str = ',';
		FILE_SaveStruct.analogValue[3].str = ',';
		FILE_SaveStruct.analogValue[4].str = ',';
		FILE_SaveStruct.analogValue[5].str = ',';
		FILE_SaveStruct.analogValue[6].str = ',';
		FILE_SaveStruct.analogValue[7].str = ',';

		/* �������� */
		FILE_SaveInfo();

		/* ��ȡ������������ */
		/* ��ȡ�ɹ�������������в������ݼ�¼ */
		FILE_ReadFile(FILE_NAME_PATCH_PACK, 0,
				(uint8_t*)&FILE_PatchPack, sizeof(FILE_PatchPackTypedef));
		curPatchPack = FILE_ReadInfo(&FILE_PatchPack);

		FILE_SendInfoFormatConvert((uint8_t*)GPRS_SendBuffer.dataPack, 
								   (uint8_t*)FILE_ReadStruct, curPatchPack);
		GPRS_SendBuffer.dataPackNumbL = curPatchPack;

		/* ʹ��MainProcess���������� */
		osMessagePut(gprsTaskMessageQid, START_TASK_GPRS, 1000);

		/* �ȴ�GPRSProcess��� */
		signal = osSignalWait(MAINPROCESS_GPRS_SEND_FINISHED, 20000);
		if ((signal.value.signals & MAINPROCESS_GPRS_SEND_FINISHED)
						!= MAINPROCESS_GPRS_SEND_FINISHED)
		{
			printf("�������ݳ�ʱ��˵�����ݷ���ʧ�ܣ���¼���ݵȴ�����\r\n");

			/* ����Ǳ��η����Ĳ��������¼ʱ�䣬���������ڲ����Ĺ��̣����ֲ����ļ����ݲ��� */
			if (curPatchPack == 1)
			{
				/* ��¼��ǰ�ļ�ǰһ��λ�� */
				FILE_PatchPack.patchStructOffset = FILE_DataSaveStructCnt - 1;

				FILE_WriteFile(FILE_NAME_PATCH_PACK, 0,
						(uint8_t*)&FILE_PatchPack, sizeof(FILE_PatchPackTypedef));
			}
		}
		else
		{
			/* ���ݷ��ͳɹ� */
			printf("���ݷ��͵�ƽ̨�ɹ�����\r\n");

			/* �в������� */
			if (curPatchPack > 1)
				FILE_WriteFile(FILE_NAME_PATCH_PACK, 0,
						(uint8_t*)&FILE_PatchPack, sizeof(FILE_PatchPackTypedef));
		}
		osThreadSuspend(NULL);
	}
}


