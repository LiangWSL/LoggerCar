#include "tftlcd.h"

#include "file.h"

#include "osConfig.h"
#include "TFTLCDProcess.h"

/******************************************************************************/
TFTLCD_SendBufferTypedef TFTLCD_SendBuffer;
TFTLCD_RecvBufferTypedef TFTLCD_RecvBuffer;
uint8_t TFTLCD_RecvBuf[TFTLCD_UART_RX_DATA_SIZE_MAX];

TFTLCD_StatusTypedef TFTLCD_status;

/******************************************************************************/
static void TFTLCD_StructInit(void);
static void TFTLCD_UartInit(void);
static void TFTLCD_Analog2ASCII(uint16_t typeID, float analog, AnalogTypedef* batch);
static void TFTLCD_SendBuf(uint8_t size);
static void TFTLCD_ScreenStart(void);

/*******************************************************************************
 * function:��������ʼ��
 */
void TFTLCD_Init(void)
{
	/* ���ͽṹ��ʼ�� */
	TFTLCD_StructInit();

	/* ���������մ��ڳ�ʼ�� */
	TFTLCD_UartInit();

	/* ����������ת */
	TFTLCD_ScreenStart();
}

/*******************************************************************************
 * function:���ý���ID����ת���棩
 */
void TFTLCD_SetScreenId(TFTLCD_ScreenIDEnum screen)
{
	/* �л����� */
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_SET_SCREEN;
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(screen);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(screen);

	memcpy(&TFTLCD_SendBuffer.buffer.data, &TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(9);

	/* ������ת,���Ҹ���״̬�� */
	TFTLCD_status.curScreenID = screen;
	osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_STATUS_BAR_UPDATE);
}

/*******************************************************************************
 * function��ģ��������
 */
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog)
{
	/* ������������ */
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_BATCH_UPDATE;

	/* ����ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(SCREEN_ID_CUR_DATA_8CH);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(SCREEN_ID_CUR_DATA_8CH);

	/* ģ�������� */
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH1, analog->temp1,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[0]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH2, analog->temp2,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[1]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH3, analog->temp3,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[2]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH4, analog->temp4,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[3]);

	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH5, analog->humi1,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[4]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH6, analog->humi2,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[5]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH7, analog->humi3,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[6]);
	TFTLCD_Analog2ASCII(CTL_ID_DATA_CH8, analog->humi4,
			&TFTLCD_SendBuffer.buffer.batch.analogValue[7]);

	/* ��֪��Ϊʲô������ֽ����Ǳ����� */
	/* ǿ��תΪFF */
	/* todo */
	TFTLCD_SendBuffer.tail[0] = 0xFF;

	TFTLCD_SendBuf( sizeof(TFTLCD_SendBufferTypedef));
}

/*******************************************************************************
 * function��״̬�����£��������ݣ�ʵʱʱ�䡢��ص���
 * screenID������ID
 * RT_TimeTypedef:ʱ��ָ��
 * batQuantity����ص���
 */
void TFTLCD_StatusBarTextRefresh(uint16_t screenID, RT_TimeTypedef* rt, uint8_t batQuantity)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_BATCH_UPDATE;

	/* ����ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(screenID);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(screenID);

	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.timeCtlIdH =
			HalfWord_GetHighByte(CTL_ID_REALTIME);
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.timeCtlIdL =
			HalfWord_GetLowByte(CTL_ID_REALTIME);

	/* ʱ�䳤����16���ַ� */
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.timeSizeH = 0;
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.timeSizeL = 16;

//	sprintf(&(TFTLCD_SendBuffer.buf.data.value.time.year), "%4d", rt->date.Year + 2000);

	/* �����ǰ������ϡ�20�� */
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.year[0] = '2';
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.year[1] = '0';
	HEX2ASCII(&rt->date.Year, (uint8_t*)&TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.year[2], 1);
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.str1 = '.';
	HEX2ASCII(&rt->date.Month, (uint8_t*)&TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.month[0], 1);
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.str2 = '.';
	HEX2ASCII(&rt->date.Date, (uint8_t*)&TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.day[0], 1);
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.str3 = ' ';
	HEX2ASCII(&rt->time.Hours, (uint8_t*)&TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.hour[0], 1);
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.str4 = ':';
	HEX2ASCII(&rt->time.Minutes, (uint8_t*)&TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.min[0], 1);

	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.signalCtlIdH =
			HalfWord_GetHighByte(CTL_ID_BAT_QUANTITY_PERCENT);
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.signalCtlIdL =
			HalfWord_GetLowByte(CTL_ID_BAT_QUANTITY_PERCENT);

	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.signalSizeH = 0;
	TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.signalSizeL = 3;

	sprintf(TFTLCD_SendBuffer.buffer.batch.statusBarUpdate.signalQuality,
			"%3d", batQuantity);

	memcpy(&TFTLCD_SendBuffer.buffer.data[sizeof(StatusBarUpdateTypedef)],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(sizeof(StatusBarUpdateTypedef) + 11);
}

/*******************************************************************************
 * function:��������������ת����ʽ���������ʷ����
 * @saveInfo������������
 * @typeID�����µĿؼ�
 */
void TFTLCD_HistoryDataFormat(FILE_SaveInfoTypedef* saveInfo, TFTLCD_HisDataCtlIdEnum typeID)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(SCREEN_ID_HIS_DATA);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(SCREEN_ID_HIS_DATA);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HalfWord_GetHighByte(typeID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HalfWord_GetLowByte(typeID);

	/* ��ʷ������ʾ�ĸ�ʽΪ��
	 * [0][1][2][3][4][5][6][7][8][9][10][11][12][13][14][15][16][17][18]
	 * |     |     |     |  |     |  |       |   |                  |   |
	 *    ��           ��          ��       �ո�     ʱ         ��       ��          �ո�                     ͨ��1                     �ո�
	 *
	 *
	 * [19][20][21][22][23][24][25][26][27][28][29][30][31][32][33][34][35]
	 * |                   |   |                   |   |                  |
	 *       ͨ��2                           �ո�                 ͨ��3                           �ո�                   ͨ��4
	 *
	 * [36][37][38][39][40][41][42][43][44][45][46][47][48][49][50][51][52]
	 * |                   |   |                   |   |                  |
	 *         ͨ��5                      �ո�                         ͨ��6                   �ո�                     ͨ��7
	 *
	 * [53][54][55][56][57][58]
	 * |   |                  |
	 *  �ո�          ͨ��8
	 */

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[0], saveInfo->year,  9);
	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[10], saveInfo->min,  2);
	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[13], saveInfo->analogValue[0].value, 23);
	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[36], saveInfo->analogValue[4].value, 23);
	TFTLCD_SendBuffer.buffer.update.value.date[9]  = ':';
	TFTLCD_SendBuffer.buffer.update.value.date[12] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[18] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[24] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[30] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[41] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[47] = ' ';
	TFTLCD_SendBuffer.buffer.update.value.date[53] = ' ';

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[59],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(70);
}

/*******************************************************************************
 * function:��ӡͨ��ѡ��ͼ����ʾ
 * @ctrl��ͨ��ѡ��ؼ����
 * @status����ǰ��ͨ����ֵ����ѡ�����ɲ�ѡ����֮��Ȼ��
 */
void TFTLCD_ChannelSelectICON(TFTLCD_ScreenIDEnum screen, uint16_t typeID, uint8_t status)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_ICON_DISP;

	/* ����ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(screen);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(screen);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HalfWord_GetHighByte(typeID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HalfWord_GetLowByte(typeID);

	TFTLCD_SendBuffer.buffer.update.value.date[0] = status;

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[1],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(12);
}

/*******************************************************************************
 * function:��ʱ��ѡ�����ѡ�õ���ֵ���µ�ָ����ʱ��ؼ�
 * @time��ѡ�õ�ʱ��
 */
void TFTLCD_SelectTimeUpdate(TFTLCD_ScreenIDEnum screen, uint16_t ctlID, FILE_RealTimeTypedef* time)
{
	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(screen);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(screen);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HalfWord_GetHighByte(ctlID);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HalfWord_GetLowByte(ctlID);

	HEX2ASCII(&time->year, (uint8_t*)TFTLCD_SendBuffer.buffer.update.value.date, 3);
	TFTLCD_SendBuffer.buffer.update.value.date[6] = ' ';
	HEX2ASCII(&time->hour, (uint8_t*)&TFTLCD_SendBuffer.buffer.update.value.date[7], 1);
	TFTLCD_SendBuffer.buffer.update.value.date[9] = ':';
	HEX2ASCII(&time->min, (uint8_t*)&TFTLCD_SendBuffer.buffer.update.value.date[10], 1);

	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[12],
				TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(23);
}

#if 0
/*******************************************************************************
 * function:��ӡʱ�����
 */
void TFTLCD_printTimeUpdate(FILE_RealTime* rt, CtrlID_PrintEnum ctrl)
{
	if ((ctrl != PRINT_CTRL_ID_START_TIME) && (ctrl != PRINT_CTRL_ID_END_TIME))
		return;

	TFTLCD_SendBuffer.cmd = TFTLCD_CMD_TEXT_UPDATE;

	/* ����ID */
	TFTLCD_SendBuffer.screenIdH = HalfWord_GetHighByte(SCREEN_ID_PRINT);
	TFTLCD_SendBuffer.screenIdL = HalfWord_GetLowByte(SCREEN_ID_PRINT);

	TFTLCD_SendBuffer.buffer.update.ctrlIdH = HalfWord_GetHighByte(ctrl);
	TFTLCD_SendBuffer.buffer.update.ctrlIdL = HalfWord_GetLowByte(ctrl);

	/* �����ǰ������ϡ�20�� */
	TFTLCD_SendBuffer.buffer.update.value.time.year[0] = '2';
	TFTLCD_SendBuffer.buffer.update.value.time.year[1] = '0';
	sprintf(&TFTLCD_SendBuffer.buffer.update.value.time.year[2], "%2d", rt->year);
	TFTLCD_SendBuffer.buffer.update.value.time.str1 = '.';
	sprintf(&TFTLCD_SendBuffer.buffer.update.value.time.month[0], "%2d", rt->month);
	TFTLCD_SendBuffer.buffer.update.value.time.str2 = '.';
	sprintf(&TFTLCD_SendBuffer.buffer.update.value.time.day[0], "%2d", rt->day);
	TFTLCD_SendBuffer.buffer.update.value.time.str3 = ' ';
	sprintf(&TFTLCD_SendBuffer.buffer.update.value.time.hour[0], "%2d", rt->hour);
	TFTLCD_SendBuffer.buffer.update.value.time.str4 = ':';
	sprintf(&TFTLCD_SendBuffer.buffer.update.value.time.min[0], "%2d", rt->min);

	/* �������ˡ���05������λ */
	memcpy(&TFTLCD_SendBuffer.buffer.update.value.date[sizeof(TFTLCD_TimeUpdateTypedef) - 3],
			TFTLCD_SendBuffer.tail, 4);

	TFTLCD_SendBuf(sizeof(TFTLCD_TimeUpdateTypedef) + 11);
}
#endif



/*******************************************************************************
 * Uart�����жϺ���
 */
void TFTLCD_UartIdleDeal(void)
{
	uint32_t tmp_flag = 0, tmp_it_source = 0;

	tmp_flag = __HAL_UART_GET_FLAG(&TFTLCD_UART, UART_FLAG_IDLE);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(&TFTLCD_UART, UART_IT_IDLE);
	if((tmp_flag != RESET) && (tmp_it_source != RESET))
	{
		__HAL_DMA_DISABLE(TFTLCD_UART.hdmarx);
		__HAL_DMA_CLEAR_FLAG(TFTLCD_UART.hdmarx, TFTLCD_UART_DMA_FLAG_GL);

		/* Clear Uart IDLE Flag */
		__HAL_UART_CLEAR_IDLEFLAG(&TFTLCD_UART);

		TFTLCD_RecvBuffer.bufferSize = TFTLCD_UART_RX_DATA_SIZE_MAX
						- __HAL_DMA_GET_COUNTER(TFTLCD_UART.hdmarx);

		memcpy(&TFTLCD_RecvBuffer.date.buf[0], TFTLCD_RecvBuf, TFTLCD_RecvBuffer.bufferSize);
		memset(TFTLCD_RecvBuf, 0, TFTLCD_RecvBuffer.bufferSize);

		osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_RECV_ENABLE);

		TFTLCD_UART.hdmarx->Instance->CNDTR = TFTLCD_UART.RxXferSize;
		__HAL_DMA_ENABLE(TFTLCD_UART.hdmarx);
	}
}

/*******************************************************************************
 *
 */
ErrorStatus TFTLCD_CheckHeadTail(void)
{
	if (TFTLCD_RecvBuffer.date.recvBuf.head == TFTLCD_CMD_HEAD)
	{
		if ((TFTLCD_RecvBuffer.date.buf[TFTLCD_RecvBuffer.bufferSize - 4] == TFTLCD_CMD_TAIL1)
			&& (TFTLCD_RecvBuffer.date.buf[TFTLCD_RecvBuffer.bufferSize - 3] == TFTLCD_CMD_TAIL2)
			&& (TFTLCD_RecvBuffer.date.buf[TFTLCD_RecvBuffer.bufferSize - 2] == TFTLCD_CMD_TAIL3)
			&& (TFTLCD_RecvBuffer.date.buf[TFTLCD_RecvBuffer.bufferSize - 1] == TFTLCD_CMD_TAIL4))
		{
			return SUCCESS;
		}
		else
			return ERROR;
	}
	else
		return ERROR;
}



/*******************************************************************************
 *
 */
static void TFTLCD_SendBuf(uint8_t size)
{
//	HAL_UART_Transmit_DMA(&TFTLCD_UART, (uint8_t*)&TFTLCD_SendBuffer.head, size);
	HAL_UART_Transmit(&TFTLCD_UART, (uint8_t*)&TFTLCD_SendBuffer.head, size, 1000);
}

/*******************************************************************************
 *
 */
static void TFTLCD_StructInit(void)
{
	TFTLCD_SendBuffer.head = TFTLCD_CMD_HEAD;

	TFTLCD_SendBuffer.tail[0] = TFTLCD_CMD_TAIL1;
	TFTLCD_SendBuffer.tail[1] = TFTLCD_CMD_TAIL2;
	TFTLCD_SendBuffer.tail[2] = TFTLCD_CMD_TAIL3;
	TFTLCD_SendBuffer.tail[3] = TFTLCD_CMD_TAIL4;
}

/*******************************************************************************
 *
 */
static void TFTLCD_UartInit(void)
{
	UART_DMAIdleConfig(&TFTLCD_UART, TFTLCD_RecvBuf, TFTLCD_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 *
 */
static void TFTLCD_Analog2ASCII(uint16_t typeID, float analog, AnalogTypedef* batch)
{
	uint16_t size;

	batch->ctrlIdH = HalfWord_GetHighByte(typeID);
	batch->ctrlIdL = HalfWord_GetLowByte(typeID);
	/* %5.1��ʾ��Ч���ݳ���Ϊ5��С��1λ */
	size = sprintf((char*)&batch->value[0], "%5.1f", analog);
	batch->sizeH = HalfWord_GetHighByte(size);
	batch->sizeL = HalfWord_GetLowByte(size);
}

/*******************************************************************************
 *
 */
static void TFTLCD_ScreenStart(void)
{
	osDelay(3000);

	TFTLCD_SetScreenId(SCREEN_ID_CUR_DATA_8CH);
}









