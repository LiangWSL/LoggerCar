#include "TFTLCDProcess.h"

#include "tftlcd.h"
#include "print.h"
#include "gprs.h"
#include "display.h"

/******************************************************************************/
//static void ScreenPrint(uint16_t cmd, CtrlID_PrintEnum ctrl, TFTTASK_StatusEnum* status,
//		PRINT_ChannelSelectTypedef* select, FILE_RealTime* startTime, FILE_RealTime* stopTime);
static void ScreenTimeSelect(FILE_RealTime* pTime, uint16_t cmd,
		CtrlID_TimeSelectEnum ctrl, uint8_t value, TFTTASK_StatusEnum status);

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

//	FILE_RealTime startPrintTime;		/* ��ӡ��ʼʱ�� */
//	FILE_RealTime endPrintTime;			/* ��ӡ��ʼʱ�� */

//	TFTTASK_StatusEnum status;
//	PRINT_ChannelSelectTypedef PrintChannelSelect;

	TFTLCD_Init();

	while(1)
	{
		/* ��ȡ�����ź� */
		signal = osSignalWait(0xFFFFFFFF, 1);

		/* ״̬��ˢ�� */
		if ((signal.value.signals & TFTLCD_TASK_STATUS_BAR_UPDATE) ==
				TFTLCD_TASK_STATUS_BAR_UPDATE)
		{
			TFTLCD_StatusBarTextRefresh(TFTLCD_status.curScreenID, &RT_RealTime,
					ANALOG_value.batVoltage);
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

					/* ���ս��������� */
//					switch (screenID)
//					{
//					case SCREEN_ID_PRINT:
//						ScreenPrint(TFTLCD_RecvBuffer.date.recvBuf.cmd, (CtrlID_PrintEnum)ctrlID,
//								&status, &PrintChannelSelect, &startPrintTime, &endPrintTime);
//						break;
//
//					case SCREEN_ID_PRINT_TIME_SELECT:
//	//					/* �̶��ֽڣ���ʾѡ��ؼ� */
//	//					if (TFTLCD_RecvBuffer.date.recvBuf.buf[0] != 0x1B)
//	//						break;
//						if (status == TFT_PRINT_START_TIME)
//							ScreenTimeSelect(&startPrintTime, TFTLCD_RecvBuffer.date.recvBuf.cmd,
//									(CtrlID_TimeSelectEnum)ctrlID, TFTLCD_RecvBuffer.date.recvBuf.buf[1],
//									TFT_PRINT_START_TIME);
//						else if (status == TFT_PRINT_END_TIME)
//							ScreenTimeSelect(&endPrintTime, TFTLCD_RecvBuffer.date.recvBuf.cmd,
//									(CtrlID_TimeSelectEnum)ctrlID, TFTLCD_RecvBuffer.date.recvBuf.buf[1],
//									TFT_PRINT_END_TIME);
//						break;
//					default:
//						break;
//					}
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
		/* ��ʾ���µ�һ������ */
		DISPLAY_Status.hisDataDispStructOffset =
				dataFileStructCnt - DISPLAY_HIS_DATA_ONE_SCREEN_CNT;
		DISPLAY_HistoryData(DISPLAY_Status.hisDataDispStructOffset);
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

	default:
		break;
	}
}

#if 0
/*******************************************************************************
 *
 */
static void ScreenPrint(uint16_t cmd, CtrlID_PrintEnum ctrl, TFTTASK_StatusEnum* status,
		PRINT_ChannelSelectTypedef* select, FILE_RealTime* startTime, FILE_RealTime* stopTime)
{

	switch (cmd)
	{
	case TFTLCD_CMD_BUTTON:
		switch (ctrl)
		{
		case PRINT_CTRL_ID_START_TIME_BUTTON:
			*status = TFT_PRINT_START_TIME;
			break;
		case PRINT_CTRL_ID_END_TIME_BUTTON:
			*status = TFT_PRINT_END_TIME;
			break;
		case PRINT_CTRL_ID_START_PRINT:
//			FILE_PrintDependOnTime(startTime, stopTime, select);
			break;
		case PRINT_CTRL_ID_CHANNEL_1_BUTTON:
			TFTLCD_printChannelSelectICON(PRINT_CTRL_ID_CHANNEL_1_ICON,
					select->status.bit.ch1);
			/* ͨ��ѡ���״̬ȡ�� */
			select->status.bit.ch1 = !select->status.bit.ch1;
			break;
		case PRINT_CTRL_ID_CHANNEL_2_BUTTON:
			TFTLCD_printChannelSelectICON(PRINT_CTRL_ID_CHANNEL_2_ICON,
					select->status.bit.ch2);
			/* ͨ��ѡ���״̬ȡ�� */
			select->status.bit.ch2 = !select->status.bit.ch2;
			break;
		case PRINT_CTRL_ID_CHANNEL_3_BUTTON:
			TFTLCD_printChannelSelectICON(PRINT_CTRL_ID_CHANNEL_3_ICON,
					select->status.bit.ch3);
			/* ͨ��ѡ���״̬ȡ�� */
			select->status.bit.ch3 = !select->status.bit.ch3;
			break;
		case PRINT_CTRL_ID_CHANNEL_4_BUTTON:
			TFTLCD_printChannelSelectICON(PRINT_CTRL_ID_CHANNEL_4_ICON,
					select->status.bit.ch4);
			/* ͨ��ѡ���״̬ȡ�� */
			select->status.bit.ch4 = !select->status.bit.ch4;
			break;
		case PRINT_CTRL_ID_CHANNEL_5_BUTTON:
			TFTLCD_printChannelSelectICON(PRINT_CTRL_ID_CHANNEL_5_ICON,
					select->status.bit.ch5);
			/* ͨ��ѡ���״̬ȡ�� */
			select->status.bit.ch5 = !select->status.bit.ch5;
			break;
		case PRINT_CTRL_ID_CHANNEL_6_BUTTON:
			TFTLCD_printChannelSelectICON(PRINT_CTRL_ID_CHANNEL_6_ICON,
					select->status.bit.ch6);
			/* ͨ��ѡ���״̬ȡ�� */
			select->status.bit.ch6 = !select->status.bit.ch6;
			break;
		case PRINT_CTRL_ID_CHANNEL_7_BUTTON:
			TFTLCD_printChannelSelectICON(PRINT_CTRL_ID_CHANNEL_7_ICON,
					select->status.bit.ch7);
			/* ͨ��ѡ���״̬ȡ�� */
			select->status.bit.ch7 = !select->status.bit.ch7;
			break;
		case PRINT_CTRL_ID_CHANNEL_8_BUTTON:
			TFTLCD_printChannelSelectICON(PRINT_CTRL_ID_CHANNEL_8_ICON,
					select->status.bit.ch8);
			/* ͨ��ѡ���״̬ȡ�� */
			select->status.bit.ch8 = !select->status.bit.ch8;
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}
#endif

/*******************************************************************************
 * function:ʱ��ѡ��ؼ���ֵ�ϴ�
 * @pTime:����ʱ��ṹ��
 * @cmd����������
 * @ctrl���ؼ�ö��
 * @value���ؼ�ֵ
 * @timeType��ʱ�����ͣ�1����ӡ��ʼʱ�䣬0����ӡ����ʱ�䣩
 */
static void ScreenTimeSelect(FILE_RealTime* pTime, uint16_t cmd,
		CtrlID_TimeSelectEnum ctrl, uint8_t value, TFTTASK_StatusEnum status)
{
	/* ���տؼ����ͻ��� */
	switch (cmd)
	{
	/* ѡ��ؼ����� */
	case TFTLCD_CMD_BUTTON_SELECT:
		switch (ctrl)
		{
		case TIME_SELECT_CTRL_ID_YEAR:
			pTime->year = 17 + value;
			break;
		case TIME_SELECT_CTRL_ID_MONTH:
			pTime->month = 1 + value;
			break;
		case TIME_SELECT_CTRL_ID_DAY:
			pTime->day = 1 + value;
			break;
		case TIME_SELECT_CTRL_ID_HOUR:
			pTime->hour = value;
			break;
		case TIME_SELECT_CTRL_ID_MIN:
			pTime->min = 5 * value;
			break;
		case TIME_SELECT_CTRL_ID_CANCEL:
			/* ȡ�����ʱ����� */
			memset(pTime, 0, sizeof(FILE_RealTime));
			break;
		case TIME_SELECT_CTRL_ID_OK:
			switch (status)
			{
			case TFT_PRINT_START_TIME:
//				TFTLCD_printTimeUpdate(pTime, PRINT_CTRL_ID_START_TIME);
				break;
			case TFT_PRINT_END_TIME:
//				TFTLCD_printTimeUpdate(pTime, PRINT_CTRL_ID_END_TIME);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

/*******************************************************************************
 *
 */
void HistoryDataDisplay(void)
{

}
















