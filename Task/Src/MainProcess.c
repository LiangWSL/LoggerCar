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

extern FunctionalState sendPackRecordEnable;			/* �������ݰ���¼ʹ�� */

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

		/* �������Ҫ����ģ��ȱ������� */
		if (sendPackRecordEnable == ENABLE)
		{
			FILE_SaveSendInfo(&FILE_SaveStruct, &RT_RecordTime, &GPS_Locate, &ANALOG_value);
		}

		/* ��ȡ�����ļ� */
		FILE_ReadFile(FILE_NAME_PATCH_PACK, 0,
				(uint8_t*)&FILE_PatchPack, sizeof(FILE_PatchPackTypedef));
		/* ����жϵ����� */
//		if (FILE_PatchPack.patchStructOffset != 0)
//		{
//			/* ��ȡ�Ӷϵ㿪ʼ�����ݣ����ص�ǰ�����İ��� */
//			curPatchPack =
//					FILE_ReadSaveInfo(FILE_ReadStruct, FILE_PatchPack.patchStructOffset);
//			GPRS_SendPackSize = GPRS_SendDataPackFromRecord(&GPRS_NewSendbuffer,
//					FILE_ReadStruct, curPatchPack, &RT_RealTime);
//			FILE_PatchPack.patchStructOffset += curPatchPack;
//		}
//		else /* û�жϵ����� */
		{
			/* ��¼���� */
			if (sendPackRecordEnable == ENABLE)
			{
				curPatchPack = FILE_ReadSaveInfo(FILE_ReadStruct, 0);
				GPRS_SendPackSize = GPRS_SendDataPackFromRecord(&GPRS_NewSendbuffer,
						FILE_ReadStruct, 1, &RT_RealTime);
			}
			else	/* ʵʱ���� */
			{
				GPRS_SendPackSize = GPRS_SendDataPackFromCurrent(&GPRS_NewSendbuffer,
						&RT_RealTime, &ANALOG_value, &GPS_Locate);
			}
		}

		/* ����ʲô�������ȡ����¼ */
		sendPackRecordEnable = DISABLE;

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

		osThreadSuspend(NULL);
	}
}


