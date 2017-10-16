#include "TFTLCDProcess.h"

#include "tftlcd.h"
#include "print.h"
#include "gprs.h"
#include "display.h"
#include "ble.h"

/******************************************************************************/
void ScreenDefaultDisplay(uint16_t screen);
void ScreenTouchDisplay(uint16_t screenID, uint16_t typeID);

/*******************************************************************************
 *
 */
void TFTLCD_Task(void)
{
	osEvent signal;
	uint16_t screenID;
	uint16_t ctrlID;

	TFTLCD_Init();

	while(1)
	{
//		BLE_LinkPrint();

		/* ��ȡ�����ź� */
		signal = osSignalWait(TFTLCD_TASK_STATUS_BAR_UPDATE
				| TFTLCD_TASK_ANALOG_UPDATE | TFTLCD_TASK_RECV_ENABLE, 1000);

		/* ״̬��ˢ�� */
		if ((signal.value.signals & TFTLCD_TASK_STATUS_BAR_UPDATE) ==
				TFTLCD_TASK_STATUS_BAR_UPDATE)
		{
			/* ����״̬���ı� */
			TFTLCD_StatusBarTextRefresh(TFTLCD_status.curScreenID, &RT_RealTime,
					ANALOG_value.batVoltage);

			/* ����״̬��ͼ�� */
			TFTLCD_StatusBarIconRefresh(TFTLCD_status.curScreenID);
		}

		/* ˢ��ʵʱ���� */
		if ((signal.value.signals & TFTLCD_TASK_ANALOG_UPDATE) ==
				TFTLCD_TASK_ANALOG_UPDATE)
		{
			TFTLCD_AnalogDataRefresh(&ANALOG_value);
		}

		/* ���ڽ��մ��� */
		if ((signal.value.signals & TFTLCD_TASK_RECV_ENABLE) == TFTLCD_TASK_RECV_ENABLE)
		{
			/* ���ͷ��β */
			if (ERROR != TFTLCD_CheckHeadTail())
			{
				/* ʶ����ID */
				screenID = ((TFTLCD_RecvBuffer.date.recvBuf.screenIdH << 8)
						| (TFTLCD_RecvBuffer.date.recvBuf.screenIdL));

				/* ������µ�ָ�� */
				if (TFTLCD_RecvBuffer.date.recvBuf.cmd == TFTLCD_CMD_SCREEN_ID_GET)
				{
					TFTLCD_status.curScreenID = (TFTLCD_ScreenIDEnum)((TFTLCD_RecvBuffer.date.recvBuf.screenIdH << 8)
							| (TFTLCD_RecvBuffer.date.recvBuf.screenIdL));

					/* ������ת����Ҫˢ��һ��״̬�� */
					osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_STATUS_BAR_UPDATE);

					/* ������ת��Ĭ�ϳ��ֵĻ��� */
					ScreenDefaultDisplay(screenID);
				}
				else
				{
					/* �ؼ�ID */
					ctrlID = ((TFTLCD_RecvBuffer.date.recvBuf.ctrlIDH << 8)
							| (TFTLCD_RecvBuffer.date.recvBuf.ctrlIDL));

					/* �����û��Դ������Ĳ�����Ϣ */
					ScreenTouchDisplay(screenID, ctrlID);
				}
			}
		}
	}
}

/*******************************************************************************
 * function:������ת��Ĭ�ϳ��ֵ���Ϣ
 */
void ScreenDefaultDisplay(uint16_t screen)
{
	switch(screen)
	{
	case SCREEN_ID_HIS_DATA:
		if (FILE_DataSaveStructCnt >= DISPLAY_HIS_DATA_ONE_SCREEN_CNT)
		{
			/* ��ʾ���µ�һ������ */
			DISPLAY_Status.hisDataDispStructOffset =
					FILE_DataSaveStructCnt - DISPLAY_HIS_DATA_ONE_SCREEN_CNT;

			DISPLAY_HistoryData(DISPLAY_Status.hisDataDispStructOffset,DISPLAY_HIS_DATA_ONE_SCREEN_CNT);
		}
		break;

	/* ��ʷ���߽��� */
	case SCREEN_ID_HIS_DATA_CURVE:
		if (FILE_DataSaveStructCnt >= DISPLAY_HIS_DATA_CURVE_CNT)
		{
			DISPLAY_Status.hisDataDispStructOffset =
					FILE_DataSaveStructCnt - DISPLAY_HIS_DATA_CURVE_CNT;

			DISPLAY_HistoryDataCurve(DISPLAY_Status.hisDataDispStructOffset);
		}
		break;

	/* ������� */
	case SCREEN_ID_SET_PASSWORD:
		/* ������뻺���λָʾ */
		memcpy(DISPLAY_Status.passwordBuffer, "    ", 4);
		DISPLAY_Status.passwordBufferIndex = 0;
		TFTLCD_SetPasswordUpdate(DISPLAY_Status.passwordBufferIndex);
		break;
	default:
		break;
	}
}

/*******************************************************************************
 * funtion:���յ��û�����ʱ����ʾ
 */
void ScreenTouchDisplay(uint16_t screenID, uint16_t typeID)
{
	switch (screenID)
	{
	case SCREEN_ID_HIS_DATA:
		DISPLAY_HistoryTouch(typeID);
		break;

	case SCREEN_ID_PRINT:
		DISPLAY_PrintTouch(typeID);
		break;

	case SCREEN_ID_TIME_SELECT:
		DISPLAY_TimeSelectTouch(typeID, TFTLCD_RecvBuffer.date.recvBuf.buf[1]);
		break;

	case SCREEN_ID_SET_PASSWORD:
		DISPLAY_SetPasswordTouch(typeID);
		break;

	case SCREEN_ID_SET_ALARM_LIMIT:
		DISPLAY_SetAlarmLimitTouch(typeID);
		break;

	case SCREEN_ID_SET_ALARM_LIMIT_2:
		DISPLAY_SetAlarmLimit2Touch(typeID);
		break;

	case SCREEN_ID_SET_ALARM_CODE:
		DISPLAY_SetMessageTouch(typeID);
		break;

	case SCREEN_ID_SET_CHANGE_PASSWORD:
		DISPLAY_SetPasswordChangeTouch(typeID);
		break;

	default:
		break;
	}
}

