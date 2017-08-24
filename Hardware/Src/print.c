#include "print.h"

/******************************************************************************/
uint8_t PRINT_SendBuffer[PRINT_SEND_BYTES_MAX];

/******************************************************************************/
static void PRINT_SendData(uint16_t size);
static BOOL PRINT_GetAnalogAndAdjust(uint16_t analog, uint8_t* buf, float alarmUp, float alarmLow);

/*******************************************************************************
 * function:��ӡ����
 */
void PRINT_Date(char* fileName)
{
	memcpy(&PRINT_SendBuffer[0], "*************", 13);
	memcpy(&PRINT_SendBuffer[13], fileName, 6);
	memcpy(&PRINT_SendBuffer[19], "*************", 13);
	PRINT_SendBuffer[33] = '\n';
	PRINT_SendData(33);
}

/*******************************************************************************
 *
 */
void PRINT_SetMode(void)
{
	PRINT_SendBuffer[0] = 0x1B;
	PRINT_SendBuffer[1] = 0x7B;
	PRINT_SendBuffer[2] = 0;
	PRINT_SendData(3);
}

/*******************************************************************************
 * function:��ӡ���
 */
BOOL PRINT_DataOut(FILE_InfoTypedef* info, PRINT_ChannelSelectTypedef* select)
{
	uint8_t index = 0;
	BOOL    status = FALSE;

	/* ʱ��ת�� */
	BCD2ASCII((char*)&PRINT_SendBuffer[0], &info->realTime.hour, 1);
	PRINT_SendBuffer[2] = ':';
	BCD2ASCII((char*)&PRINT_SendBuffer[3], &info->realTime.min, 1);
	PRINT_SendBuffer[5] = ':';
	BCD2ASCII((char*)&PRINT_SendBuffer[6], &info->realTime.sec, 1);
	index += 8;

	/* ���ݴ�ӡѡ��������� */
	if (select->status.bit.ch1)
	{
		status = PRINT_GetAnalogAndAdjust(info->analogValue.temp1, &PRINT_SendBuffer[index],
				FILE_DeviceParam.temp1.alarmValueUp, FILE_DeviceParam.temp1.alarmValueLow);
		index += 6;
	}
	if (select->status.bit.ch2)
	{
		PRINT_GetAnalogAndAdjust(info->analogValue.humi1, &PRINT_SendBuffer[index],
				PRINT_ALARM_INVALID, PRINT_ALARM_INVALID);
		index += 6;
	}
	if (select->status.bit.ch3)
	{
		status = PRINT_GetAnalogAndAdjust(info->analogValue.temp2, &PRINT_SendBuffer[index],
				FILE_DeviceParam.temp1.alarmValueUp, FILE_DeviceParam.temp1.alarmValueLow);
		index += 6;
	}
	if (select->status.bit.ch4)
	{
		PRINT_GetAnalogAndAdjust(info->analogValue.humi2, &PRINT_SendBuffer[index],
				PRINT_ALARM_INVALID, PRINT_ALARM_INVALID);
		index += 6;
	}
	if (select->status.bit.ch5)
	{
		status = PRINT_GetAnalogAndAdjust(info->analogValue.temp3, &PRINT_SendBuffer[index],
				FILE_DeviceParam.temp1.alarmValueUp, FILE_DeviceParam.temp1.alarmValueLow);
		index += 6;
	}
	if (select->status.bit.ch6)
	{
		PRINT_GetAnalogAndAdjust(info->analogValue.humi3, &PRINT_SendBuffer[index],
				PRINT_ALARM_INVALID, PRINT_ALARM_INVALID);
		index += 6;
	}
	if (select->status.bit.ch7)
	{
		status = PRINT_GetAnalogAndAdjust(info->analogValue.humi4, &PRINT_SendBuffer[index],
				FILE_DeviceParam.temp1.alarmValueUp, FILE_DeviceParam.temp1.alarmValueLow);
		index += 6;
	}
	if (select->status.bit.ch8)
	{
		PRINT_GetAnalogAndAdjust(info->analogValue.temp1, &PRINT_SendBuffer[index],
				PRINT_ALARM_INVALID, PRINT_ALARM_INVALID);
		index += 6;
	}

	/* ��ӡ���� */
	PRINT_SendBuffer[index] = '\n';
	index++;

	PRINT_SendData(index);

	return status;
}

/*******************************************************************************
 * function����ӡ��������
 */
void PRINT_TitleOut(void)
{
	uint8_t index = 0;

	memcpy(&PRINT_SendBuffer[0], "********************************\n", 33);
	index += 33;

	memcpy(&PRINT_SendBuffer[index], "�ջ�����\n", 9);
	index += 9;

	memcpy(&PRINT_SendBuffer[index], "��������\n", 9);
	index += 9;

	memcpy(&PRINT_SendBuffer[index], "���ͳ��ƣ�\n", 11);
	index += 11;

	memcpy(&PRINT_SendBuffer[index], "������ţ�\n", 11);
	index += 11;

	PRINT_SendData(index);
}

/*******************************************************************************
 *
 */
void PRINT_TailOut(void)
{
	uint8_t index = 0;

	memcpy(&PRINT_SendBuffer[0], "********************************\n", 33);
	index += 33;

	memcpy(&PRINT_SendBuffer[index], "ǩ���ˣ�\n", 9);
	index += 9;

	memcpy(&PRINT_SendBuffer[index], "\n\n\n", 3);
	index += 3;

	memcpy(&PRINT_SendBuffer[index], "ǩ�����ڣ�\n", 11);
	index += 11;

	memcpy(&PRINT_SendBuffer[index], "\n\n\n", 3);
	index += 3;

	PRINT_SendData(index);
}

/*******************************************************************************
 *
 */
static void PRINT_SendData(uint16_t size)
{
//	HAL_UART_Transmit_DMA(&PRINT_UART, PRINT_SendBuffer, size);
	HAL_UART_Transmit(&PRINT_UART, PRINT_SendBuffer, size, 1000);
}

/*******************************************************************************
 * function:��ȡģ������ֵ�����������ӡ�������ж��Ƿ񳬱�
 * analog��ģ����ֵ
 * buf��ת�����ַ�����ָ��
 * alarmUp:��������
 * alarmLow����������
 * return: true->���꣬  false->����
 */
static BOOL PRINT_GetAnalogAndAdjust(uint16_t analog, uint8_t* buf, float alarmUp, float alarmLow)
{
	float    temp;
	uint16_t data;
	BOOL     status = FALSE;

	data = (HalfWord_GetLowByte(analog) << 8) | HalfWord_GetHighByte(analog);

	/* �ж����� */
	if ((data & 0x8000) != 0)
		temp = -((float)(data & 0x7FFF) / 10);
	else
		temp = (float)(data & 0x7FFF) / 10;

	/* ����������������豸��������δ�ҵ����⣬���������о� */
//	sprintf((char*)buf, "%6.1f", temp);

	*buf = ' ';

	/* ���Ϊ���� */
	if ((data & 0x8000) != 0)
	{
		*(buf + 1) = '-';
		/* ת��Ϊ���� */
		data &= 0x7FFF;
	}
	else
	{
		/* �����Ļ����п�������λ�� */
		*(buf + 1) = ((data % 10000) / 1000) + '0';
		if (*(buf + 1) == '0')
			*(buf + 1) = ' ';
	}

	*(buf + 2) = ((data % 1000) / 100)   + '0';
	*(buf + 3) = ((data % 100) / 10)     + '0';
	*(buf + 4) = '.';
	*(buf + 5) = (data % 10)             + '0';

	/* ��������ֵ��Ч�����ҳ��� */
	if ((alarmUp != PRINT_ALARM_INVALID) && (temp > alarmUp))
		status = TRUE;

	if ((alarmLow != PRINT_ALARM_INVALID) && (temp < alarmLow))
		status = TRUE;

	return status;
}


























