#include "TFTLCDProcess.h"

#include "tftlcd.h"

/******************************************************************************/
static void ScreenPrint(uint16_t cmd, CtrlID_PrintEnum ctrl, TFTTASK_StatusEnum* status,
		PrintChannelSelectTypedef* select);
static void ScreenTimeSelect(FILE_RealTime* pTime, uint16_t cmd,
		CtrlID_TimeSelectEnum ctrl, uint8_t value, TFTTASK_StatusEnum status);

/*******************************************************************************
 *
 */
void TFTLCD_Task(void)
{
	osEvent signal;
	uint16_t screenID;
	uint16_t ctrlID;

	FILE_RealTime startPrintTime;		/* ��ӡ��ʼʱ�� */
	FILE_RealTime endPrintTime;			/* ��ӡ��ʼʱ�� */

	TFTTASK_StatusEnum status;
	PrintChannelSelectTypedef PrintChannelSelect;

	TFTLCD_Init();

	while(1)
	{
		signal = osSignalWait(TFTLCD_TASK_RECV_ENABLE, osWaitForever);
		if ((signal.value.signals & TFTLCD_TASK_RECV_ENABLE) == TFTLCD_TASK_RECV_ENABLE)
		{
			/* ���ͷ��β */
			if (ERROR != TFTLCD_CheckHeadTail())
			{
				/* ʶ����ID�Ϳؼ�ID */
				screenID = ((TFTLCD_RecvBuffer.date.recvBuf.screenIdH << 8)
						| (TFTLCD_RecvBuffer.date.recvBuf.screenIdL));
				ctrlID = ((TFTLCD_RecvBuffer.date.recvBuf.ctrlIDH << 8)
						| (TFTLCD_RecvBuffer.date.recvBuf.ctrlIDL));

				/* ���ս��������� */
				switch (screenID)
				{
				case SCREEN_ID_PRINT:
					ScreenPrint(TFTLCD_RecvBuffer.date.recvBuf.cmd, (CtrlID_PrintEnum)ctrlID,
							&status, &PrintChannelSelect);
					break;

				case SCREEN_ID_PRINT_TIME_SELECT:
//					/* �̶��ֽڣ���ʾѡ��ؼ� */
//					if (TFTLCD_RecvBuffer.date.recvBuf.buf[0] != 0x1B)
//						break;
					if (status == TFT_PRINT_START_TIME)
						ScreenTimeSelect(&startPrintTime, TFTLCD_RecvBuffer.date.recvBuf.cmd,
								(CtrlID_TimeSelectEnum)ctrlID, TFTLCD_RecvBuffer.date.recvBuf.buf[1],
								TFT_PRINT_START_TIME);
					else if (status == TFT_PRINT_END_TIME)
						ScreenTimeSelect(&endPrintTime, TFTLCD_RecvBuffer.date.recvBuf.cmd,
								(CtrlID_TimeSelectEnum)ctrlID, TFTLCD_RecvBuffer.date.recvBuf.buf[1],
								TFT_PRINT_END_TIME);
					break;
				default:
					break;
				}
			}
		}
	}

}

/*******************************************************************************
 *
 */
static void ScreenPrint(uint16_t cmd, CtrlID_PrintEnum ctrl, TFTTASK_StatusEnum* status,
		PrintChannelSelectTypedef* select)
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
				TFTLCD_printTimeUpdate(pTime, PRINT_CTRL_ID_START_TIME);
				break;
			case TFT_PRINT_END_TIME:
				TFTLCD_printTimeUpdate(pTime, PRINT_CTRL_ID_END_TIME);
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


















