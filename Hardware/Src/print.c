#include "print.h"
#include "ble.h"
#include "iwdg.h"

/******************************************************************************/
uint8_t PRINT_SendBuffer[PRINT_SEND_BYTES_MAX];
FILE_SaveStructTypedef PRINT_DataFileReadStruct;

/******************************************************************************/
static void PRINT_SendData(uint16_t size);
//static BOOL PRINT_GetAnalogAndAdjust(uint16_t analog, uint8_t* buf, float alarmUp, float alarmLow);
static PRINT_DataStatusEnum PRINT_DataPrint(uint64_t offset,
		DISPLAY_CompareTimeTypedef* endTimePoint,
		DISPLAY_CompareTimeTypedef* printDate,
		ChannelSelectTypedef* select);

static void PRINT_PrintTitle(void);
static void PRINT_PrintTail(void);
static void PRINT_SetMode(void);

/*******************************************************************************
 * @brief ��ӡ����
 * @param startTime����ʼ��ӡʱ��
 * @param endTime��������ӡʱ��
 * @select����ӡͨ��ѡ��
 */
void PRINT_PrintProcess(DISPLAY_CompareTimeTypedef* startTime,
						DISPLAY_CompareTimeTypedef* endTime,
						ChannelSelectTypedef* select)
{
	uint32_t offsetStruct;
	PRINT_DataStatusEnum status;
	DISPLAY_CompareTimeTypedef printDate;	/* ��ǰ��ӡ���� */

	/* ��յ�ǰ��ӡ���� */
	memset((uint8_t*)&printDate, 0, sizeof(DISPLAY_CompareTimeTypedef));

	/* ���ݿ�ʼ��ӡʱ��Ѱ����ʼ����ƫ���� */
	offsetStruct = PRINT_SearchStartTime(startTime);

	/* ���ô�ӡģʽ */
	PRINT_SetMode();
	/* �ȴ�ӡ���� */
	PRINT_PrintTitle();

	/* ��ӡ��� */
	while (1)
	{
		status = PRINT_DataPrint(offsetStruct, endTime, &printDate, select);

		if (PRINT_DATA_END == status)
			break;
		else if (PRINT_DATA_NORMAL == status)
			offsetStruct += 5;
		else if (PRINT_DATA_OVERlIMITED == status)
			offsetStruct += 2;

		osDelay(100);

#if IWDG_ENABLE
		/* ���Ź���� */
		HAL_IWDG_Refresh(&hiwdg);
#endif
	}

	/* ��ӡǩ�� */
	PRINT_PrintTail();
}

/*******************************************************************************
 * @brief ���ݿ�ʼʱ����ҽṹ��ƫ�ƣ����ؽṹ��ƫ��
 * @param destTime��Ŀ��ʱ���
 */
uint32_t PRINT_SearchStartTime(DISPLAY_CompareTimeTypedef* destTime)
{
	uint32_t fileStructStart, fileStructEnd, searchPoint;
	int compareStatus;

	fileStructStart = 0;
	fileStructEnd   = FILE_DataSaveStructCnt;

	while(1)
	{
		/* ȡ�м�ֵ */
		searchPoint = (fileStructStart + fileStructEnd) / 2;

		/* ������������ṹ��֮��ģ��Ա��ζ���������Ϊ׼ */
		if (searchPoint == fileStructStart)
			break;

		FILE_ReadFile(FILE_NAME_SAVE_DATA, searchPoint * sizeof(FILE_SaveStructTypedef),
				(BYTE*)&PRINT_DataFileReadStruct, sizeof(FILE_SaveStructTypedef));
		/* �Ƚ����ݵ�������ʱ�� */
		/* buf1<buf2 return -1
		 * buf1>buf2 return 1
		 * buf1=buf2 return 0 */
		compareStatus = memcmp(destTime, &PRINT_DataFileReadStruct,
				sizeof(DISPLAY_CompareTimeTypedef));

		/* �ҵ�Ŀ��ʱ�� */
		if (compareStatus == 0)
		{
			break;
		}
		else if (compareStatus < 0) /* Ŀ��ʱ�� < ��ȡʱ�� */
		{
			fileStructEnd = searchPoint;
		}
		else /* Ŀ��ʱ�� > ��ȡʱ�� */
		{
			fileStructStart = searchPoint;
		}
	}

	return searchPoint;
}

/*******************************************************************************
 * function:���ݽṹ��ƫ�ƣ��������ݣ������ݴ�ӡͨ��ѡ�񣬴�ӡ���ݣ������ظ������Ƿ񳬱�
 */
static PRINT_DataStatusEnum PRINT_DataPrint(uint64_t offset,
		DISPLAY_CompareTimeTypedef* endTimePoint,
		DISPLAY_CompareTimeTypedef* printDate,
		ChannelSelectTypedef* select)
{
	uint8_t index = 0;
	PRINT_DataStatusEnum status = PRINT_DATA_OVERlIMITED;

	/* ��ȡ���� */
	FILE_ReadFile(FILE_NAME_SAVE_DATA, offset * sizeof(FILE_SaveStructTypedef),
			(uint8_t*)&PRINT_DataFileReadStruct, sizeof(FILE_SaveStructTypedef));

	/* �������ݵ�ʱ�� >= ����ʱ��� */
	/* Ϊ�˷�ֹ�û�ѡ��Ľ���ʱ���ǵ�ǰ��ʱ�䣬���ǵ�ǰʱ�䲢δ��¼���ݣ������������޴�ӡ������ */
	if (memcmp(PRINT_DataFileReadStruct.year, endTimePoint, sizeof(DISPLAY_CompareTimeTypedef)) >= 0)
	{
		return PRINT_DATA_END;
	}

	/* ��ӡĳ�����ݣ��ڿ�ʼ��ӡ���ڣ������ӡֻ���ӡʱ�伴�� */
	if (memcmp(printDate, PRINT_DataFileReadStruct.year, 6) != 0)
	{
		/* ��¼��ǰ��ӡʱ�� */
		memcpy(printDate, PRINT_DataFileReadStruct.year, 6);
		PRINT_Date(PRINT_DataFileReadStruct.year);
	}

	/* ʱ���� */
	memcpy((char*)&PRINT_SendBuffer[0],  &PRINT_DataFileReadStruct.hour, 9);
	index += 9;

	/* ���ݴ�ӡѡ��������� */
	if (select->status.bit.ch1)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[0], 6);
		/* �ж��Ƿ񳬱� */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[0], &PARAM_DeviceParam.chAlarmValue[0]);
		index += 6;
	}
	if (select->status.bit.ch2)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[1], 6);
		/* �ж��Ƿ񳬱� */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[1], &PARAM_DeviceParam.chAlarmValue[1]);
		index += 6;
	}
	if (select->status.bit.ch3)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[2], 6);
		/* �ж��Ƿ񳬱� */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[2], &PARAM_DeviceParam.chAlarmValue[2]);
		index += 6;
	}
	if (select->status.bit.ch4)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[3], 6);
		/* �ж��Ƿ񳬱� */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[3], &PARAM_DeviceParam.chAlarmValue[3]);
		index += 6;
	}
	if (select->status.bit.ch5)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[4], 6);
		/* �ж��Ƿ񳬱� */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[4], &PARAM_DeviceParam.chAlarmValue[4]);
		index += 6;
	}
	if (select->status.bit.ch6)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[5], 6);
		/* �ж��Ƿ񳬱� */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[5], &PARAM_DeviceParam.chAlarmValue[5]);
		index += 6;
	}
	if (select->status.bit.ch7)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[6], 6);
		/* �ж��Ƿ񳬱� */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[6], &PARAM_DeviceParam.chAlarmValue[6]);
		index += 6;
	}
	if (select->status.bit.ch8)
	{
		memcpy((char*)&PRINT_SendBuffer[index],  &PRINT_DataFileReadStruct.analogValue[7], 6);
		/* �ж��Ƿ񳬱� */
//		status = PRINT_AdjustOverLimited(&saveInfo.analogValue[7], &PARAM_DeviceParam.chAlarmValue[7]);
		index += 6;
	}

	/* ��ӡ���У����һ����ֵ����Ҫ���������û��и��� */
	PRINT_SendBuffer[index - 1] = '\n';

	PRINT_SendData(index);

	return status;
}

/*******************************************************************************
 *
 */
static void PRINT_SetMode(void)
{
	PRINT_SendBuffer[0] = 0x1B;
	PRINT_SendBuffer[1] = 0x7B;
	PRINT_SendBuffer[2] = 0;
	PRINT_SendData(3);
}

/*******************************************************************************
 *
 */
static void PRINT_SendData(uint16_t size)
{
//	HAL_UART_Transmit_DMA(&PRINT_UART, PRINT_SendBuffer, size);
	if (PRINT_MODE_INTEGRATED == DISPLAY_Status.printMode)
	{
		HAL_UART_Transmit(&PRINT_UART, PRINT_SendBuffer, size, 1000);
	}
	else if (PRINT_MODE_BLE_LINK == DISPLAY_Status.printMode)
	{
		HAL_UART_Transmit(&BLE_UART, PRINT_SendBuffer, size, 1000);
	}
}

/*******************************************************************************
 * function����ӡ��������
 */
static void PRINT_PrintTitle(void)
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
static void PRINT_PrintTail(void)
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

	memcpy(&PRINT_SendBuffer[index], "\n\n\n\n\n\n\n\n\n", 9);
	index += 9;

	PRINT_SendData(index);
}

/*******************************************************************************
 * function���ж���ֵ�Ƿ񳬱�
 */
static PRINT_DataStatusEnum PRINT_AdjustOverLimited(FILE_SaveInfoAnalogTypedef* analog,
													ParamAlarmTypedef* param)
{
	float value;

	/* ת����float */
	value = FILE_Analog2Float(analog);

	/* �Ƚ������� */
	if ((value > param->alarmValueUp) || (value < param->alarmValueLow))
		return PRINT_DATA_OVERlIMITED;
	else
		return PRINT_DATA_NORMAL;
}

/*******************************************************************************
 * @brief ��ӡ����
 */
static void PRINT_Date(char* date)
{
	memcpy(&PRINT_SendBuffer[0], "*************", 13);
	memcpy(&PRINT_SendBuffer[13], date, 6);
	memcpy(&PRINT_SendBuffer[19], "*************", 13);
	PRINT_SendBuffer[33] = '\n';
	PRINT_SendData(33);
}














