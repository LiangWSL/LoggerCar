#include "gprs.h"


#include "GPRSProcess.h"
#include "osConfig.h"

/******************************************************************************/
uint8_t GPRS_RecvBuffer[GPRS_UART_RX_DATA_SIZE_MAX];
GPRS_BufferStatusTypedef GPRS_BufferStatus;

/*******************************************************************************
 *
 */
void GPRS_StructInit(GPRS_StructTypedef* sendBuf)
{
	sendBuf->head = GPRS_PACK_HEAD;

	sendBuf->locationType = FILE_DeviceParam.locationType;

	/* eeprom�ж������� */
	memcpy(&sendBuf->seriaNumber, FILE_DeviceParam.deviceSN, sizeof(sendBuf->seriaNumber));
	sendBuf->firmwareVersion = 	  FILE_DeviceParam.firmwareVersion;
	sendBuf->recordInterval = 	  FILE_DeviceParam.recordInterval;
	sendBuf->overLimitInterval =  FILE_DeviceParam.overLimitRecordInterval;

	/* ����ͨ������ */
	sendBuf->exitAnalogChannelNumb = FILE_DeviceParam.exitAnalogChannelNumb;
	memcpy(&sendBuf->param[0], &FILE_DeviceParam.param[0], sizeof(sendBuf->param));

	sendBuf->tail = GPRS_PACK_TAIL;
}

/*******************************************************************************
 *
 */
void GPRS_Init(void)
{
	UART_DMAIdleConfig(&GPRS_UART, GPRS_RecvBuffer, GPRS_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 *
 */
void GPRS_SendData(uint8_t *pData, uint16_t Size)
{
	HAL_UART_Transmit_DMA(&GPRS_UART, pData, Size);
}

/*******************************************************************************
 *
 */
void GPRS_SendCmd(char* str)
{
	GPRS_SendData((uint8_t*)str, strlen(str));
}

/******************************************************************************/
void GPRS_UartIdleDeal(void)
{
	uint32_t tmp_flag = 0, tmp_it_source = 0;

	tmp_flag = __HAL_UART_GET_FLAG(&GPRS_UART, UART_FLAG_IDLE);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(&GPRS_UART, UART_IT_IDLE);
	if((tmp_flag != RESET) && (tmp_it_source != RESET))
	{
		__HAL_DMA_DISABLE(GPRS_UART.hdmarx);
		__HAL_DMA_CLEAR_FLAG(GPRS_UART.hdmarx, DMA_FLAG_GL6);

		/* Clear Uart IDLE Flag */
		__HAL_UART_CLEAR_IDLEFLAG(&GPRS_UART);

		GPRS_BufferStatus.bufferSize = GPRS_UART_RX_DATA_SIZE_MAX
						- __HAL_DMA_GET_COUNTER(GPRS_UART.hdmarx);

		memcpy(GPRS_BufferStatus.recvBuffer, GPRS_RecvBuffer, GPRS_BufferStatus.bufferSize);
		memset(GPRS_RecvBuffer, 0, GPRS_BufferStatus.bufferSize);

		osSignalSet(gprsprocessTaskHandle, GPRS_PROCESS_TASK_RECV_ENABLE);

		GPRS_UART.hdmarx->Instance->CNDTR = GPRS_UART.RxXferSize;
		__HAL_DMA_ENABLE(GPRS_UART.hdmarx);
	}
}

/*******************************************************************************
 *
 */
void GPRS_RstModule(void)
{
	GPRS_RST_CTRL_ENABLE();
	osDelay(100);
	GPRS_RST_CTRL_DISABLE();
}

/*******************************************************************************
 * gprs����У��
 */
uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint16_t size)
{
	uint8_t cal = 0;

	while(size--)
	{
		cal += (*pBuffer++);
	}

	return cal;
}

/*******************************************************************************
 *
 */
void GPRS_SendProtocol(GPRS_StructTypedef* sendBuf, uint16_t patchPack)
{
	uint16_t dataSize = 0;

	/* ���ݳ��ȼ��㷽����m=���ݰ����� n=ͨ����
	 * ���ݳ��� = m(17+2n)+3n+23 */
//	dataSize = patchPack * (17 + 2 * ANALOG_CHANNEL_NUMB)
//				+ sizeof(GPRS_ParamTypedef) * ANALOG_CHANNEL_NUMB
//				+ 23;
	dataSize = patchPack * 33 + 47;

	/* ��ȡ���ݳ��� */
	sendBuf->dateSizeH = HalfWord_GetHighByte(dataSize);
	sendBuf->dateSizeL = HalfWord_GetLowByte(dataSize);

	/* ��ȡ���ݰ��� */
	sendBuf->dataPackNumbH = HalfWord_GetHighByte(patchPack);
	sendBuf->dataPackNumbL = HalfWord_GetLowByte(patchPack);

	if (patchPack < GPRS_PATCH_PACK_NUMB_MAX)
	{
		/* �����ݵĽ�β�������֡β */
		memcpy(&sendBuf->dataPack[patchPack], &sendBuf->tail, 1);
	}

	/* ����У�� */
	sendBuf->verifyData =
			GPRS_VerifyCalculate(&sendBuf->head, dataSize + 4);
	if (patchPack < GPRS_PATCH_PACK_NUMB_MAX)
	{
		/* ������������У�� */
		memcpy((void*)(&(sendBuf->dataPack[patchPack].realTime.month)),
					&sendBuf->verifyData, 1);
	}

	/* �������ݣ����͵����ֽ��� = 5+���ݳ��� */
	GPRS_SendData(&sendBuf->head, dataSize + 5);
}

/*******************************************************************************
 * function����ȡ�ź�����
 *
 */
uint8_t GPRS_GetSignalQuality(uint8_t* buf)
{
	uint8_t temp[2];

	str2numb(&buf[GPRS_SIGNAL_QUALITY_OFFSET], temp, 2);
	return (uint8_t)(temp[0] * 10 + temp[1]);
}


