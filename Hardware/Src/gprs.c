#include "gprs.h"

#include "GPRSProcess.h"

/******************************************************************************/
uint8_t  GPRS_RecvData[GPRS_UART_RX_DATA_SIZE_MAX];
uint8_t  GPRS_signalQuality;						/* GPRS�ź����� */
uint16_t GPRS_SendPackSize = 0;						/* GPRS���Ͱ���С */
char     ICCID[20];									/* ICCID */
char	 IMSI[15];									/* IMSI */
char     IMEI[15];									/* IMEI */

GPRS_RecvBufferTypedef GPRS_RecvBuffer;

GPRS_SendbufferTyepdef GPRS_NewSendbuffer;

extern osThreadId gprsprocessTaskHandle;

const char Message[] = {0x67, 0x6D, 0x5D, 0xDE, 0x8D, 0xEF, 0x68, 0x3C,
		0x79, 0xD1, 0x62, 0x80};

/******************************************************************************/
static uint8_t GPRS_VerifyCalculate(uint8_t* pBuffer, uint16_t size);
static void ProtocolFormat_LocationFloat(float value, uint8_t* pBuffer);
static void ProtocolFormat_LocationASCII(char* value, uint8_t* pBuffer);
static void ProtocolFormat_AnalogFloat(float value, uint8_t* pBuffer, DataFormatEnum format);
static void ProtocolFormat_AnalogASCII(char* value, uint8_t* pBuffer, DataFormatEnum format);

/*******************************************************************************
 * function��GPRS��ʼ�����������ͽṹ���ʼ��������idle���ճ�ʼ��
 */
void GPRS_Init(void)
{
	GPRS_NewSendbuffer.head = GPRS_PACK_HEAD;
	GPRS_NewSendbuffer.dataVersion = 2;
	memcpy(GPRS_NewSendbuffer.serialNumber, "1708151515", 10);
	GPRS_NewSendbuffer.deviceTypeCodeH = 10;
	GPRS_NewSendbuffer.deviceTypeCodeL = 10;
	GPRS_NewSendbuffer.firewareVersion = 1;
//	GPRS_NewSendbuffer.PackBuffer.MessageBuffer.packVersion = 1;
//	GPRS_NewSendbuffer.PackBuffer.MessageBuffer.codeCount = 1;
//	memcpy(GPRS_NewSendbuffer.PackBuffer.MessageBuffer.codeNumber[0], "18367053909", 11);

	GPRS_NewSendbuffer.tail = GPRS_PACK_TAIL;

	UART_DMAIdleConfig(&GPRS_UART, GPRS_RecvData, GPRS_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 * @brief:GPRS��������
 */
void GPRS_SendCmd(char* str)
{
	HAL_UART_Transmit_DMA(&GPRS_UART, (uint8_t*)str, strlen(str));
}

/*******************************************************************************
 * @brief:GPRS��������
 */
void GPRS_SendData(uint16_t size)
{
	HAL_UART_Transmit_DMA(&GPRS_UART, (uint8_t*)&GPRS_NewSendbuffer, size);
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

		memcpy(GPRS_RecvBuffer.buffer.recvBuffer, GPRS_RecvData, GPRS_RecvBuffer.bufferSize);
		memset(GPRS_RecvData, 0, GPRS_RecvBuffer.bufferSize);

		osSignalSet(gprsprocessTaskHandle, GPRS_PROCESS_TASK_RECV_ENABLE);

		GPRS_UART.hdmarx->Instance->CNDTR = GPRS_UART.RxXferSize;
		__HAL_DMA_ENABLE(GPRS_UART.hdmarx);
	}
}

/*******************************************************************************
 * @brief ���ͽṹ�幫������
 */
uint16_t GPRS_SendPackPublicPart(GPRS_SendbufferTyepdef* sendBuffer,
		volatile uint16_t packContentSize, RT_TimeTypedef* curtime, GPRS_PackTypeEnum type)
{
	static uint16_t GPRS_PackCount = 0;						/* GPRS����� */

	/* �������� */
	sendBuffer->packType = type;
	/* ����� */
	GPRS_PackCount++;
	sendBuffer->packCountH = HALFWORD_BYTE_H(GPRS_PackCount);
	sendBuffer->packCountL = HALFWORD_BYTE_L(GPRS_PackCount);
	/* �ϴ�ʱ�� */
	/* ʱ���ַ���ת����BCD */
	HEX2BCD(&sendBuffer->year,  &curtime->date.Year,    1);
	HEX2BCD(&sendBuffer->month, &curtime->date.Month,   1);
	HEX2BCD(&sendBuffer->day,   &curtime->date.Date,    1);
	HEX2BCD(&sendBuffer->hour,  &curtime->time.Hours,   1);
	HEX2BCD(&sendBuffer->min,   &curtime->time.Minutes, 1);
	HEX2BCD(&sendBuffer->sec,   &curtime->time.Seconds, 1);

	/* �ֽ��� */
//	size = (size + 3) + 23;
	packContentSize += 26;
	sendBuffer->dataSizeH = HALFWORD_BYTE_H(packContentSize);
	sendBuffer->dataSizeL = HALFWORD_BYTE_L(packContentSize);

	*(&sendBuffer->head + packContentSize + 3) = GPRS_PACK_TAIL;
	*(&sendBuffer->head + packContentSize + 4) =
			GPRS_VerifyCalculate(&sendBuffer->head, packContentSize + 4);

	/* �����������С */
	return (packContentSize + 5);
}

/*******************************************************************************
 * @brief ���Ͷ��Ű�
 */
void GPRS_SendMessagePack(GPRS_SendbufferTyepdef* sendBuffer,
		RT_TimeTypedef curtime,	char* messageContent, uint16_t messageCount)
{
	uint16_t size = 0;
	static uint16_t GPRS_PackCount = 0;

	/* ���������ֽ��� */
	sendBuffer->PackBuffer.MessageBuffer.contentCountH = HALFWORD_BYTE_H(messageCount);
	sendBuffer->PackBuffer.MessageBuffer.contentCountL = HALFWORD_BYTE_L(messageCount);

	/* �������� */
	memcpy(GPRS_NewSendbuffer.PackBuffer.MessageBuffer.content, messageContent,
			messageCount);
	/* ���峤�� = 53 + 11 * ������� + ���������ֽ��� */
	size = messageCount + sendBuffer->PackBuffer.MessageBuffer.codeCount * 11 + 53;
	sendBuffer->PackBuffer.MessageBuffer.packSizeH = HALFWORD_BYTE_H(size);
	sendBuffer->PackBuffer.MessageBuffer.packSizeL = HALFWORD_BYTE_L(size);

	/* �������� */
	sendBuffer->packType = GPRS_PACK_TYPE_MESSAGE;
	/* ����� */
	GPRS_PackCount++;
	sendBuffer->packCountH = HALFWORD_BYTE_H(GPRS_PackCount);
	sendBuffer->packCountL = HALFWORD_BYTE_L(GPRS_PackCount);
	/* �ϴ�ʱ�� */
	/* ʱ���ַ���ת����BCD */
	HEX2BCD(&sendBuffer->year,  &curtime.date.Year,    1);
	HEX2BCD(&sendBuffer->month, &curtime.date.Month,   1);
	HEX2BCD(&sendBuffer->day,   &curtime.date.Date,    1);
	HEX2BCD(&sendBuffer->hour,  &curtime.time.Hours,   1);
	HEX2BCD(&sendBuffer->min,   &curtime.time.Minutes, 1);
	HEX2BCD(&sendBuffer->sec,   &curtime.time.Seconds, 1);


	/* �ֽ��� */
	size = (size + 3) + 23;
	sendBuffer->dataSizeH = HALFWORD_BYTE_H(size);
	sendBuffer->dataSizeL = HALFWORD_BYTE_L(size);

	if (messageCount < GPRS_MESSAGE_BYTES_MAX)
	{
		sendBuffer->PackBuffer.MessageBuffer.content[messageCount]
													 = GPRS_PACK_TAIL;
	}

	sendBuffer->verify =
			GPRS_VerifyCalculate(&sendBuffer->head, size + 4);

	if (messageCount < GPRS_MESSAGE_BYTES_MAX)
	{
		sendBuffer->PackBuffer.MessageBuffer.content[messageCount + 1]
													 = sendBuffer->verify;
	}



//	memcpy(&sendBuffer->head, MessageModule, sizeof(MessageModule));

	HAL_UART_Transmit_DMA(&GPRS_UART, &sendBuffer->head, size + 5);
}

/*******************************************************************************
 * @brief������ʵʱ���ݰ�
 */
uint16_t GPRS_SendDataPackFromCurrent(GPRS_SendbufferTyepdef* sendBuffer,
		RT_TimeTypedef* curtime, ANALOG_ValueTypedef* analog,
		GPS_LocateTypedef* location)
{
	uint16_t size = 0;

	/* ����汾 */
	sendBuffer->PackBuffer.DataBuffer.packVersion = GPRS_PARAM_DATA_PACK_VERSION;
	/* ��¼����Ϊʵʱ���� */
	sendBuffer->PackBuffer.DataBuffer.recordStatus = GPRS_RECORD_STATUS_CURRENT;
	/* ����ͨ���� */
	sendBuffer->PackBuffer.DataBuffer.channelCount = ANALOG_CHANNEL_NUMB;
	/* N��ͨ������ */
	memcpy(sendBuffer->PackBuffer.DataBuffer.param, PARAM_DeviceParam.chParam,
			sizeof(ParamTypeTypedef) * ANALOG_CHANNEL_NUMB);
	/* ��������Ϊ1 */
	sendBuffer->PackBuffer.DataBuffer.dataPackCountH = 0;
	sendBuffer->PackBuffer.DataBuffer.dataPackCountL = 1;

	/* ʱ���ַ���ת����BCD */
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].year,  &curtime->date.Year,    1);
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].month, &curtime->date.Month,   1);
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].day,   &curtime->date.Date,    1);
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].hour,  &curtime->time.Hours,   1);
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].min,   &curtime->time.Minutes, 1);
	HEX2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[0].sec,   &curtime->time.Seconds, 1);
	/* ��ص��� */
	sendBuffer->PackBuffer.DataBuffer.SendData[0].batteryLevel = (uint8_t)analog->batVoltage;
	/* �ⲿ��Դ״̬ */
	sendBuffer->PackBuffer.DataBuffer.SendData[0].externalPowerStatus =
			(GPRS_ExternalPowerStatusEnum)INPUT_CheckPwrOnStatus();

	/* ��λΪGPS��λ */
	sendBuffer->PackBuffer.DataBuffer.SendData[0].locationStatus = GPS_LOCATION_TYPE_GPS;
	/* ת����γ��ֵ */
	ProtocolFormat_LocationFloat(location->latitude,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].latitude);
	ProtocolFormat_LocationFloat(location->longitude,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].longitude);

	/* ת��ģ����ֵ */
	ProtocolFormat_AnalogFloat(analog->channel1,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[0],
			sendBuffer->PackBuffer.DataBuffer.param[0].dataFormat);
	ProtocolFormat_AnalogFloat(analog->channel2,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[1],
			sendBuffer->PackBuffer.DataBuffer.param[1].dataFormat);
	ProtocolFormat_AnalogFloat(analog->channel3,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[2],
			sendBuffer->PackBuffer.DataBuffer.param[2].dataFormat);
	ProtocolFormat_AnalogFloat(analog->channel4,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[3],
			sendBuffer->PackBuffer.DataBuffer.param[3].dataFormat);
	ProtocolFormat_AnalogFloat(analog->channel5,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[4],
			sendBuffer->PackBuffer.DataBuffer.param[4].dataFormat);
	ProtocolFormat_AnalogFloat(analog->channel6,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[5],
			sendBuffer->PackBuffer.DataBuffer.param[5].dataFormat);
	ProtocolFormat_AnalogFloat(analog->channel7,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[6],
			sendBuffer->PackBuffer.DataBuffer.param[6].dataFormat);
	ProtocolFormat_AnalogFloat(analog->channel8,
			(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[0].analogValue[7],
			sendBuffer->PackBuffer.DataBuffer.param[7].dataFormat);

	/* ��ǰ������ʽ�̶�Ϊ61�ֽ� */
	size = 61;
	sendBuffer->PackBuffer.DataBuffer.packSizeH = 0;
	sendBuffer->PackBuffer.DataBuffer.packSizeL = 61;

	/* ���Э�鹫���������� */
	size = GPRS_SendPackPublicPart(sendBuffer, size, curtime, GPRS_PACK_TYPE_DATA);

	/* �����������С */
	return size;
}


/*******************************************************************************
 * @brief�����ͼ�¼���ݰ�
 */
uint16_t GPRS_SendDataPackFromRecord(GPRS_SendbufferTyepdef* sendBuffer,
		FILE_SaveStructTypedef* saveInfo, uint16_t sendPackCount, RT_TimeTypedef* curtime)
{
	uint8_t i = 0;
	uint16_t size = 0;
	__IO uint8_t* saveInfoAddr = 0;

	/* ����汾 */
	sendBuffer->PackBuffer.DataBuffer.packVersion = GPRS_PARAM_DATA_PACK_VERSION;
	/* ��¼����Ϊ��¼���� */
	sendBuffer->PackBuffer.DataBuffer.recordStatus = GPRS_RECORD_STATUS_RECORD;
	/* ����ͨ���� */
	sendBuffer->PackBuffer.DataBuffer.channelCount = ANALOG_CHANNEL_NUMB;
	/* N��ͨ������ */
	memcpy(sendBuffer->PackBuffer.DataBuffer.param, PARAM_DeviceParam.chParam,
			sizeof(ParamTypeTypedef) * ANALOG_CHANNEL_NUMB);
	/* �������� */
	sendBuffer->PackBuffer.DataBuffer.dataPackCountH = HALFWORD_BYTE_H(sendPackCount);
	sendBuffer->PackBuffer.DataBuffer.dataPackCountL = HALFWORD_BYTE_L(sendPackCount);

	for (i = 0; i < sendPackCount; i++)
	{
		/* ʱ��ת����BCD */
		ASCII2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[i].year,  saveInfo->year,  2);
		ASCII2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[i].month, saveInfo->month, 2);
		ASCII2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[i].day,   saveInfo->day,   2);
		ASCII2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[i].hour,  saveInfo->hour,  2);
		ASCII2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[i].min,   saveInfo->min,   2);
		ASCII2BCD(&sendBuffer->PackBuffer.DataBuffer.SendData[i].sec,   saveInfo->sec,   2);
		/* ��ص��� */
		ASCII2HEX(&sendBuffer->PackBuffer.DataBuffer.SendData[i].batteryLevel, saveInfo->batQuality, 3);
		/* �ⲿ��Դ״̬ */
		ASCII2HEX(&sendBuffer->PackBuffer.DataBuffer.SendData[i].externalPowerStatus, &saveInfo->exPwrStatus, 1);

		/* ��λΪGPS��λ */
		sendBuffer->PackBuffer.DataBuffer.SendData[i].locationStatus = GPS_LOCATION_TYPE_GPS;
		/* ת����γ��ֵ */
		ProtocolFormat_LocationASCII(saveInfo->latitude,
				(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[i].latitude);
		ProtocolFormat_LocationASCII(saveInfo->longitude,
				(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[i].longitude);

		/* ת��ģ����ֵ */
		ProtocolFormat_AnalogASCII(saveInfo->analogValue[0].value,
				(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[i].analogValue[0],
				sendBuffer->PackBuffer.DataBuffer.param[0].dataFormat);
		ProtocolFormat_AnalogASCII(saveInfo->analogValue[1].value,
				(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[i].analogValue[1],
				sendBuffer->PackBuffer.DataBuffer.param[1].dataFormat);
		ProtocolFormat_AnalogASCII(saveInfo->analogValue[2].value,
				(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[i].analogValue[2],
				sendBuffer->PackBuffer.DataBuffer.param[2].dataFormat);
		ProtocolFormat_AnalogASCII(saveInfo->analogValue[3].value,
				(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[i].analogValue[3],
				sendBuffer->PackBuffer.DataBuffer.param[3].dataFormat);
		ProtocolFormat_AnalogASCII(saveInfo->analogValue[4].value,
				(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[i].analogValue[4],
				sendBuffer->PackBuffer.DataBuffer.param[4].dataFormat);
		ProtocolFormat_AnalogASCII(saveInfo->analogValue[5].value,
				(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[i].analogValue[5],
				sendBuffer->PackBuffer.DataBuffer.param[5].dataFormat);
		ProtocolFormat_AnalogASCII(saveInfo->analogValue[6].value,
				(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[i].analogValue[6],
				sendBuffer->PackBuffer.DataBuffer.param[6].dataFormat);
		ProtocolFormat_AnalogASCII(saveInfo->analogValue[7].value,
				(uint8_t*)&sendBuffer->PackBuffer.DataBuffer.SendData[i].analogValue[7],
				sendBuffer->PackBuffer.DataBuffer.param[7].dataFormat);

		/* �ṹ��ָ�벻��ֱ����Ӽ� */
		saveInfoAddr = (__IO uint8_t*)saveInfo;
		saveInfoAddr += sizeof(FILE_SaveStructTypedef);
		saveInfo = (FILE_SaveStructTypedef*)saveInfoAddr;
	}

	/* ��ǰ������ʽ�̶�Ϊ61�ֽ� */
//	size = 4 + 3 * ͨ���� + �������� * ��17 + 2 * ͨ������
	size = 28 + sendPackCount * 33;
	sendBuffer->PackBuffer.DataBuffer.packSizeH = HALFWORD_BYTE_H(size);
	sendBuffer->PackBuffer.DataBuffer.packSizeL = HALFWORD_BYTE_L(size);

	/* ���Э�鹫���������� */
	size = GPRS_SendPackPublicPart(sendBuffer, size, curtime, GPRS_PACK_TYPE_DATA);

	/* �����������С */
	return size;
}

/*******************************************************************************
 * ����ƽ̨ʱ��У׼
 * @pBuffer�����ջ������ݣ�ƽ̨���ģ�
 * @pStruct�����ͽṹ��
 */
void GPRS_TimeAdjustWithCloud(GPRS_RecvPackTypedef* recvPack)
{
	RT_TimeTypedef   eTime;

	BCD2HEX(&eTime.date.Year,    &recvPack->serverYear,  1);
	BCD2HEX(&eTime.date.Month,   &recvPack->serverMonth, 1);
	BCD2HEX(&eTime.date.Date,    &recvPack->serverDay,   1);
	BCD2HEX(&eTime.time.Hours,   &recvPack->serverHour,  1);
	BCD2HEX(&eTime.time.Minutes, &recvPack->serverMin,   1);
	BCD2HEX(&eTime.time.Seconds, &recvPack->serverSec,   1);

	if ((eTime.date.Year           != RT_RealTime.date.Year)
			|| (eTime.date.Month   != RT_RealTime.date.Month)
			|| (eTime.date.Date    != RT_RealTime.date.Date)
			|| (eTime.time.Hours   != RT_RealTime.time.Hours)
			|| (eTime.time.Minutes != RT_RealTime.time.Minutes))
	{
		RT_SetRealTime(&eTime);
	}
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

/*******************************************************************************
 * @brief ��λ������ֵЭ���ʽת��
 */
static void ProtocolFormat_LocationFloat(float value, uint8_t* pBuffer)
{
	uint32_t temp;

	/* ��Ч�Ķ�λ��ֵ */
	if (value == 0)
		memset(pBuffer, 0, 4);

	/* ��ȡ�������� */
	*pBuffer = abs((int)value);

	temp = (uint32_t)((value - (*pBuffer)) * 1000000);

	if (value < 0)
		temp |= 0x800000;

	*(pBuffer + 1) = (uint8_t)((temp & 0x00FF0000) >> 16);
	*(pBuffer + 2) = (uint8_t)((temp & 0x0000FF00) >> 8);
	*(pBuffer + 3) = (uint8_t)(temp & 0x000000FF);
}

/*******************************************************************************
 * @brief ��λASCIIֵЭ���ʽת��
 */
static void ProtocolFormat_LocationASCII(char* value, uint8_t* pBuffer)
{
	char str[11];
	double data;
	uint32_t temp;

	/* ���ַ���תΪdouble */
	memcpy(str, value, 10);
	str[10] = '\0';
	data = atof(str);

	/* ��ȡ�������� */
	*pBuffer = abs((int)data);

	temp = (uint32_t)((data - (*pBuffer)) * 1000000);

	if (data < 0)
		temp |= 0x800000;

	*(pBuffer + 1) = (uint8_t)((temp & 0x00FF0000) >> 16);
	*(pBuffer + 2) = (uint8_t)((temp & 0x0000FF00) >> 8);
	*(pBuffer + 3) = (uint8_t)(temp & 0x000000FF);
}

/*******************************************************************************
 * @brief ģ����������ֵЭ���ʽת��
 */
static void ProtocolFormat_AnalogFloat(float value, uint8_t* pBuffer, DataFormatEnum format)
{
	uint16_t temp = 0;
	/* ��Чֵ */
	if (value == ANALOG_CHANNLE_INVALID_VALUE)
	{
		*pBuffer       = 0xFF;
		*(pBuffer + 1) = 0xFE;
		return;
	}

	switch (format)
	{
	case FORMAT_INT:
		temp = (uint16_t)abs((int)(value));
		break;

	case FORMAT_ONE_DECIMAL:
		temp = (uint16_t)abs((int)(value * 10));
		break;

	case FORMAT_TWO_DECIMAL:
		temp = (uint16_t)abs((int)(value * 100));
		break;
	default:
		break;
	}

	*pBuffer       = HALFWORD_BYTE_H(temp);
	*(pBuffer + 1) = HALFWORD_BYTE_L(temp);

	/* ���������λ��һ */
	if (value < 0)
		*pBuffer |= 0x80;
}

/*******************************************************************************
 * @brief ģ����������ֵЭ���ʽת��
 */
static void ProtocolFormat_AnalogASCII(char* value, uint8_t* pBuffer, DataFormatEnum format)
{
	char str[6];
	float data;
	uint16_t temp = 0;

	/* �����ͨ��ֵ��NULL�������Э����дFFFE */
	if (memcmp(value, " NULL", 5) == 0)
	{
		*pBuffer       = 0xFF;
		*(pBuffer + 1) = 0xFE;
		return;
	}

	/* ���ַ���תΪfloat */
	memcpy(str, value, 5);
	str[5] = '\0';
	data = (float)atof(str);

	switch (format)
	{
	case FORMAT_INT:
		temp = (uint16_t)abs((int)(data));
		break;

	case FORMAT_ONE_DECIMAL:
		temp = (uint16_t)abs((int)(data * 10));
		break;

	case FORMAT_TWO_DECIMAL:
		temp = (uint16_t)abs((int)(data * 100));
		break;
	default:
		break;
	}

	*pBuffer =       HALFWORD_BYTE_H(temp);
	*(pBuffer + 1) = HALFWORD_BYTE_L(temp);

	/* ���������λ��һ */
	if (data < 0)
		*pBuffer |= 0x80;
}
