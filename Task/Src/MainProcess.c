#include "../Inc/MainProcess.h"
#include "public.h"

#include "GPRSProcess.h"

#include "rt.h"
#include "gps.h"
#include "file.h"
#include "input.h"
#include "analog.h"

/******************************************************************************/
extern FILE_SaveStructTypedef FILE_SaveStruct;
extern FILE_SaveStructTypedef FILE_ReadStruct[GPRS_PATCH_PACK_NUMB_MAX];
extern FILE_PatchPackTypedef FILE_PatchPack;		/* �����ļ���Ϣ */
extern osMessageQId gprsTaskMessageQid;
extern GPS_LocateTypedef  GPS_Locate;

FunctionalState messageEnable = ENABLE;


extern uint16_t GPRS_SendPackSize;		/* GPRS���Ͱ���С */
extern GPRS_NewSendbufferTyepdef GPRS_NewSendbuffer;

/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;
	uint8_t curPatchPack;

	while(1)
	{		
		/* ��ȡʱ�� */
//		signal = osMessageGet(realtimeMessageQId, 1000);
//		memcpy(&time, (uint32_t*)signal.value.v, sizeof(RT_TimeTypedef));

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
			/* ��λֵת����ASCII */
			sprintf((char*)&FILE_SaveStruct.longitude[0], "%10.5f", GPS_Locate.longitude);
			sprintf((char*)&FILE_SaveStruct.latitude[0],  "%10.5f", GPS_Locate.latitude);
		}
		else
		{
			memset((char*)&FILE_SaveStruct.longitude[0], 0, 10);
			memset((char*)&FILE_SaveStruct.latitude[0],  0, 10);
		}

		/* ʱ��ת����ASCII */
//		HEX2ASCII(&FILE_SaveStruct.year[0],  &RT_RecordTime.date.Year,    1);
//		HEX2ASCII(&FILE_SaveStruct.month[0], &RT_RecordTime.date.Month,   1);
//		HEX2ASCII(&FILE_SaveStruct.day[0],   &RT_RecordTime.date.Date,    1);
//		HEX2ASCII(&FILE_SaveStruct.hour[0],  &RT_RecordTime.time.Hours,   1);
//		HEX2ASCII(&FILE_SaveStruct.min[0],   &RT_RecordTime.time.Minutes, 1);
//		HEX2ASCII(&FILE_SaveStruct.sec[0],   &RT_RecordTime.time.Seconds, 1);
//		/* ��ȡ�ⲿ��Դ״̬ */
//		FILE_SaveStruct.exPwrStatus = INPUT_CheckPwrOnStatus() + '0';

		/* ģ����ת��ΪASCII */
//		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[0], ANALOG_value.temp1);
//		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[1], ANALOG_value.humi1);
//		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[2], ANALOG_value.temp2);
//		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[3], ANALOG_value.humi2);
//		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[4], ANALOG_value.temp3);
//		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[5], ANALOG_value.humi3);
//		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[6], ANALOG_value.temp4);
//		ANALOG_Float2ASCII(&FILE_SaveStruct.analogValue[7], ANALOG_value.humi4);
//		sprintf((char*)&FILE_SaveStruct.batQuality[0], "%3d", ANALOG_value.batVoltage);

		/* CVS�ļ���ʽ */
//		FILE_SaveStruct.batQuality[3]	   = '%';		/* ��ص����ٷֺ� */
//		FILE_SaveStruct.str7   			   = ',';
//		FILE_SaveStruct.str8   			   = ',';
//
//		/* �������� */
//		FILE_SaveInfo();
//
//		/* ��ȡ������������ */
//		/* ��ȡ�ɹ�������������в������ݼ�¼ */
//		FILE_ReadFile(FILE_NAME_PATCH_PACK, 0,
//				(uint8_t*)&FILE_PatchPack, sizeof(FILE_PatchPackTypedef));
//		curPatchPack = FILE_ReadInfo(&FILE_PatchPack);
//
//		FILE_SendInfoFormatConvert((uint8_t*)GPRS_SendBuffer.dataPack,
//								   (uint8_t*)FILE_ReadStruct, curPatchPack);
//		GPRS_SendBuffer.dataPackNumbL = curPatchPack;

		GPRS_SendPackSize = GPRS_SendDataPackFromCurrent(&GPRS_NewSendbuffer,
				&RT_RealTime, &ANALOG_value, &GPS_Locate);

		/* ʹ��MainProcess���������� */
		osMessagePut(gprsTaskMessageQid, START_TASK_DATA, 1000);

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

//		if (messageEnable)
		{
			/* ʹ��MainProcess���������� */
//			osMessagePut(gprsTaskMessageQid, START_TASK_MESSAGE, 1000);
		}


		osThreadSuspend(NULL);
	}
}


