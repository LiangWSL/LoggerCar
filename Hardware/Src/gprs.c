#include "gprs.h"

#include "GPRSProcess.h"

/******************************************************************************/
uint8_t GPRS_RecvData[GPRS_UART_RX_DATA_SIZE_MAX];
uint8_t GPRS_signalQuality;			/* GPRS�ź����� */

GPRS_SendBufferTypedef GPRS_SendBuffer;
GPRS_RecvBufferTypedef GPRS_RecvBuffer;

GPRS_NewSendbufferTyepdef GPRS_NewSendbuffer;

extern osThreadId gprsprocessTaskHandle;

const char Message[] = {0x67, 0x6D, 0x5D, 0xDE, 0x8D, 0xEF, 0x68, 0x3C,
		0x79, 0xD1, 0x62, 0x80};

/******************************************************************************/
static void GPRS_StructInit(GPRS_SendBufferTypedef* sendBuf);
static void GPRS_SendData(uint8_t *pData, uint16_t Size);
static uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint16_t size);

/*******************************************************************************
 * function��GPRS��ʼ�����������ͽṹ���ʼ��������idle���ճ�ʼ��
 */
void GPRS_Init(void)
{
	GPRS_NewSendbuffer.head = GPRS_PACK_HEAD_NEW;
	GPRS_NewSendbuffer.dataVersion = 1;
	memcpy(GPRS_NewSendbuffer.serialNumber, "1708151515", 10);
	GPRS_NewSendbuffer.firewareVersion = 1;
	GPRS_NewSendbuffer.PackBuffer.MessageBuffer.packVersion = 1;
	GPRS_NewSendbuffer.PackBuffer.MessageBuffer.codeCount = 1;
	memcpy(GPRS_NewSendbuffer.PackBuffer.MessageBuffer.codeNumber[0], "18367053909", 11);

	GPRS_NewSendbuffer.tail = GPRS_PACK_TAIL_NEW;

	GPRS_StructInit(&GPRS_SendBuffer);
	UART_DMAIdleConfig(&GPRS_UART, GPRS_RecvData, GPRS_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 * function:GPRS��������
 */
void GPRS_SendCmd(char* str)
{
	GPRS_SendData((uint8_t*)str, strlen(str));
}

/*******************************************************************************
 * function��GPRSģ�鸴λ
 */
void GPRS_RstModule(void)
{
	GPRS_RST_CTRL_ENABLE();
	osDelay(100);
	GPRS_RST_CTRL_DISABLE();
}

/*******************************************************************************
 * function��GPRS����Э�鵽ƽ̨
 * sendBuf����������ָ��
 * patch���û����а�����������������
 */
void GPRS_SendProtocol(GPRS_SendBufferTypedef* sendBuf)
{
	uint16_t dataSize = 0;

	/* ���ݳ��ȼ��㷽����m=���ݰ����� n=ͨ����
	 * ���ݳ��� = m(17+2n)+3n+23 */
//	dataSize = patchPack * (17 + 2 * ANALOG_CHANNEL_NUMB)
//				+ sizeof(GPRS_ParamTypedef) * ANALOG_CHANNEL_NUMB
//				+ 23;
	dataSize = GPRS_SendBuffer.dataPackNumbL * sizeof(GPRS_SendInfoTypedef) + 47;

	/* ��ȡ���ݳ��� */
	sendBuf->dateSizeH = HALFWORD_BYTE_H(dataSize);
	sendBuf->dateSizeL = HALFWORD_BYTE_L(dataSize);

	/* �������δ��װ������������һ�����м���֡β */
	if (GPRS_SendBuffer.dataPackNumbL < GPRS_PATCH_PACK_NUMB_MAX)
	{
		/* �����ݵĽ�β�������֡β */
		memcpy(&sendBuf->dataPack[GPRS_SendBuffer.dataPackNumbL], &sendBuf->tail, 1);
	}

	/* ����У�� */
	sendBuf->verifyData =
			GPRS_VerifyCalculate(&sendBuf->head, dataSize + 4);
	if (GPRS_SendBuffer.dataPackNumbL < GPRS_PATCH_PACK_NUMB_MAX)
	{
		/* ������������У�� */
		memcpy((void*)(&(sendBuf->dataPack[GPRS_SendBuffer.dataPackNumbL].month)),
					&sendBuf->verifyData, 1);
	}

	/* �������ݣ����͵����ֽ��� = 5+���ݳ��� */
	GPRS_SendData(&sendBuf->head, dataSize + 5);
}

/*******************************************************************************
 * function����ȡ�ź�����
 */
uint8_t GPRS_GetSignalQuality(uint8_t* buf)
{
	uint8_t temp[2];

	str2numb(temp, &buf[GPRS_SIGNAL_QUALITY_OFFSET], 2);
	return (uint8_t)(temp[0] * 10 + temp[1]);
}

/*******************************************************************************
 * function:uart����idle���մ���
 */
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

		GPRS_RecvBuffer.bufferSize = GPRS_UART_RX_DATA_SIZE_MAX
						- __HAL_DMA_GET_COUNTER(GPRS_UART.hdmarx);

		memcpy(GPRS_RecvBuffer.recvBuffer, GPRS_RecvData, GPRS_RecvBuffer.bufferSize);
		memset(GPRS_RecvData, 0, GPRS_RecvBuffer.bufferSize);

		osSignalSet(gprsprocessTaskHandle, GPRS_PROCESS_TASK_RECV_ENABLE);

		GPRS_UART.hdmarx->Instance->CNDTR = GPRS_UART.RxXferSize;
		__HAL_DMA_ENABLE(GPRS_UART.hdmarx);
	}
}

/*******************************************************************************
 * @brief ���Ͷ��Ű�
 */
void GPRS_SendMessagePack(GPRS_NewSendbufferTyepdef* sendBuffer,
		RT_TimeTypedef curtime,	char* messageContent, uint16_t messageCount)
{
	/* ���������ֽ��� */
	sendBuffer->PackBuffer.MessageBuffer.contentCount = messageCount;
	/* �������� */
	memcpy(GPRS_NewSendbuffer.PackBuffer.MessageBuffer.content, messageContent,
			messageCount);
	/* ���峤�� = 53 + 11 * ������� + ���������ֽ��� */
	sendBuffer->PackBuffer.MessageBuffer.packSize = messageCount
			+ sendBuffer->PackBuffer.MessageBuffer.codeCount * 11 + 53;

	/* �������� */
	sendBuffer->packType = GPRS_PACK_TYPE_MESSAGE;
	/* ����� */
	sendBuffer->packCount++;
	/* �ϴ�ʱ�� */
	/* ʱ���ַ���ת����BCD */
	HEX2BCD(&sendBuffer->year,  &curtime.date.Year,    1);
	HEX2BCD(&sendBuffer->month, &curtime.date.Month,   1);
	HEX2BCD(&sendBuffer->day,   &curtime.date.Date,    1);
	HEX2BCD(&sendBuffer->hour,  &curtime.time.Hours,   1);
	HEX2BCD(&sendBuffer->min,   &curtime.time.Minutes, 1);
	HEX2BCD(&sendBuffer->sec,   &curtime.time.Seconds, 1);


	/* �ֽ��� */
	sendBuffer->dataSize = (sendBuffer->PackBuffer.MessageBuffer.packSize + 3) + 23;

	if (messageCount < GPRS_MESSAGE_BYTES_MAX)
	{
		sendBuffer->PackBuffer.MessageBuffer.content[messageCount] = GPRS_PACK_TAIL_NEW;
	}

	sendBuffer->verify =
			GPRS_VerifyCalculate(&sendBuffer->head, sendBuffer->dataSize + 4);

	if (messageCount < GPRS_MESSAGE_BYTES_MAX)
	{
		sendBuffer->PackBuffer.MessageBuffer.content[messageCount + 1]
													 = sendBuffer->verify;
	}

	HAL_UART_Transmit_DMA(&GPRS_UART, &sendBuffer->head, sendBuffer->dataSize + 5);
}

/*******************************************************************************
 * function:���͵�ƽ̨���ݽṹ��ʼ��
 */
static void GPRS_StructInit(GPRS_SendBufferTypedef* sendBuf)
{
	sendBuf->head = GPRS_PACK_HEAD;

	sendBuf->locationType = PARAM_DeviceParam.locationType;

	/* eeprom�ж������� */
	memcpy(&sendBuf->seriaNumber, PARAM_DeviceParam.deviceSN, sizeof(sendBuf->seriaNumber));
	sendBuf->firmwareVersion = 	  PARAM_DeviceParam.firmwareVersion;
	sendBuf->recordInterval = 	  PARAM_DeviceParam.recordInterval;
	sendBuf->overLimitInterval =  PARAM_DeviceParam.overLimitRecordInterval;

	/* ����ͨ������ */
	sendBuf->exitAnalogChannelNumb = PARAM_DeviceParam.exAnalogChannelNumb;
	memcpy(&sendBuf->param[0], &PARAM_DeviceParam.chParam[0], sizeof(ParamTypeTypedef) * 8);

	/* ���ݰ�������󲻿��ܳ���255��������Hֻ��Ϊ0 */
	sendBuf->dataPackNumbH = 0x00;

	sendBuf->tail = GPRS_PACK_TAIL;
}

/*******************************************************************************
 * function:����dma��������
 */
static void GPRS_SendData(uint8_t *pData, uint16_t Size)
{
	HAL_UART_Transmit_DMA(&GPRS_UART, pData, Size);
}

/*******************************************************************************
 * function��gprs��������У��
 */
static uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint16_t size)
{
	uint8_t cal = 0;

	while(size--)
	{
		cal += (*pBuffer++);
	}

	return cal;
}
