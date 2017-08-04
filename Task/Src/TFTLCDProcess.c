#include "TFTLCDProcess.h"

#include "tftlcd.h"

/******************************************************************************/
static void ScreenPrint(uint16_t cmd, CtrlID_PrintEnum ctrl, TFTTASK_StatusEnum* status);
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

	FILE_RealTime printTime;		/* ��ӡ��ʼʱ�� */

	TFTTASK_StatusEnum status;

	TFTLCD_Init();

	while(1)
	{
		signal = osSignalWait(TFTLCD_TASK_RECV_ENABLE, osWaitForever);
		if ((signal.value.signals & TFTLCD_TASK_RECV_ENABLE) != TFTLCD_TASK_RECV_ENABLE)
			break;

		/* ���ͷ��β */
		if (ERROR == TFTLCD_CheckHeadTail())
			break;

		/* ʶ����ID�Ϳؼ�ID */
		screenID = ((TFTLCD_RecvBuffer.date.recvBuf.screenIdH << 8)
				| (TFTLCD_RecvBuffer.date.recvBuf.screenIdL));
		ctrlID = ((TFTLCD_RecvBuffer.date.recvBuf.ctrlIDH << 8)
				| (TFTLCD_RecvBuffer.date.recvBuf.ctrlIDL));

		/* ���ս��������� */
		switch (screenID)
		{
		case SCREEN_ID_PRINT:
			ScreenPrint(TFTLCD_RecvBuffer.date.recvBuf.cmd, (CtrlID_PrintEnum)ctrlID, &status);
			break;

		case SCREEN_ID_PRINT_TIME_SELECT:
//			/* �̶��ֽڣ���ʾѡ��ؼ� */
//			if (TFTLCD_RecvBuffer.date.recvBuf.buf[0] != 0x1B)
//				break;
			ScreenTimeSelect(&printTime, TFTLCD_RecvBuffer.date.recvBuf.cmd,
					(CtrlID_TimeSelectEnum)ctrlID, TFTLCD_RecvBuffer.date.recvBuf.buf[1], status);
			break;
		default:
			break;
		}
	}

}

/*******************************************************************************
 *
 */
static void ScreenPrint(uint16_t cmd, CtrlID_PrintEnum ctrl, TFTTASK_StatusEnum* status)
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


















