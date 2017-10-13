#include "gprs.h"

#include "GPRSProcess.h"
#include "osConfig.h"

/******************************************************************************/
uint8_t GPRS_RecvData[GPRS_UART_RX_DATA_SIZE_MAX];
uint8_t GPRS_signalQuality;			/* GPRS�ź����� */

GPRS_SendBufferTypedef GPRS_SendBuffer;
GPRS_RecvBufferTypedef GPRS_RecvBuffer;

/******************************************************************************/
static void GPRS_StructInit(GPRS_SendBufferTypedef* sendBuf);
static void GPRS_SendData(uint8_t *pData, uint16_t Size);
static uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint16_t size);

/*******************************************************************************
 * function��GPRS��ʼ�����������ͽṹ���ʼ��������idle���ճ�ʼ��
 */
void GPRS_Init(GPRS_SendBufferTypedef* sendBuf)
{
	GPRS_StructInit(sendBuf);
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
void GPRS_SendProtocol(GPRS_SendBufferTypedef* sendBuf, uint8_t patchPack)
{
	uint16_t dataSize = 0;

	/* ���ݳ��ȼ��㷽����m=���ݰ����� n=ͨ����
	 * ���ݳ��� = m(17+2n)+3n+23 */
//	dataSize = patchPack * (17 + 2 * ANALOG_CHANNEL_NUMB)
//				+ sizeof(GPRS_ParamTypedef) * ANALOG_CHANNEL_NUMB
//				+ 23;
	dataSize = patchPack * sizeof(GPRS_SendInfoTypedef) + 47;

	/* ��ȡ���ݳ��� */
	sendBuf->dateSizeH = HALFWORD_BYTE_H(dataSize);
	sendBuf->dateSizeL = HALFWORD_BYTE_L(dataSize);

	/* ��ȡ���ݰ��� */
	sendBuf->dataPackNumbH = HALFWORD_BYTE_H(patchPack);
	sendBuf->dataPackNumbL = HALFWORD_BYTE_L(patchPack);

	/* �������δ��װ������������һ�����м���֡β */
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
		memcpy((void*)(&(sendBuf->dataPack[patchPack].month)),
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

	str2numb(&buf[GPRS_SIGNAL_QUALITY_OFFSET], temp, 2);
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
	memcpy(&sendBuf->param[0], &PARAM_DeviceParam.param[0], sizeof(sendBuf->param));

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
