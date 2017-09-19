#include "display.h"

#include "file.h"
#include "tftlcd.h"
#include "print.h"
#include "param.h"

/******************************************************************************/
DISPLAY_StatusTypedef DISPLAY_Status;

/******************************************************************************/
static void TimeSelectReturn(void);
static void PasswordSelect(char numb);
static void GetAlarmLimitValue(float* value);
static void GetAlarmCode(char* code);

/*******************************************************************************
 * ��ʷ���ݽ���
 ******************************************************************************/
/*******************************************************************************
 * ��ʷ������ʾ
 */
void DISPLAY_HistoryData(uint32_t startStructOffset, uint8_t structCnt)
{
	uint8_t i;
	FILE_SaveInfoTypedef saveInfo[4];

	FILE_ReadFile(FILE_NAME_SAVE_DATA, startStructOffset * sizeof(FILE_SaveInfoTypedef),
					(uint8_t*)saveInfo, structCnt * sizeof(FILE_SaveInfoTypedef));

	for (i = 0; i < structCnt; i++)
	{
		TFTLCD_HistoryDataFormat(&saveInfo[i], (TFTLCD_HisDataCtlIdEnum)(CTL_ID_DIS_DATA_1 + i));
	}
}

/*******************************************************************************
 * ��ʷ������ʾ
 */
void DISPLAY_HistoryDataCurve(uint32_t startStructOffset)
{
	uint8_t i, j;
	FILE_SaveInfoTypedef info[DISPLAY_HIS_DATA_READ_ONCE_CNT];

	for (j = 0; j < DISPLAY_HIS_DATA_READ_CNT; j++)
	{
		FILE_ReadFile(FILE_NAME_SAVE_DATA,
					  startStructOffset * sizeof(FILE_SaveInfoTypedef),
					  (uint8_t*)info,
					  sizeof(info));
		for (i = 0; i < DISPLAY_HIS_DATA_READ_ONCE_CNT; i++)
		{
			TFTLCD_HistoryDataCurveFormat(&info[i]);
		}

		startStructOffset += DISPLAY_HIS_DATA_READ_ONCE_CNT;
	}

}

/*******************************************************************************
 * function����ʷ���ݽ��棬�����û�����
 */
void DISPLAY_HistoryTouch(uint16_t typeID)
{
	switch (typeID)
	{
	case CTL_ID_PAGE_UP:
		/* ���ⷭ��ͷ */
		if (FILE_DataSaveStructCnt - DISPLAY_Status.hisDataDispStructOffset >
				DISPLAY_HIS_DATA_ONE_SCREEN_CNT * 2)
		{
			DISPLAY_Status.hisDataDispStructOffset += DISPLAY_HIS_DATA_ONE_SCREEN_CNT;
			DISPLAY_HistoryData(DISPLAY_Status.hisDataDispStructOffset,DISPLAY_HIS_DATA_ONE_SCREEN_CNT);
		}
		break;

	case CTL_ID_PAGE_DOWN:
		/* ���ⷭ��ͷ */
		if (DISPLAY_Status.hisDataDispStructOffset >= DISPLAY_HIS_DATA_ONE_SCREEN_CNT)
		{
			DISPLAY_Status.hisDataDispStructOffset -= DISPLAY_HIS_DATA_ONE_SCREEN_CNT;
			DISPLAY_HistoryData(DISPLAY_Status.hisDataDispStructOffset,DISPLAY_HIS_DATA_ONE_SCREEN_CNT);
		}
		break;

	default:
		break;
	}
}

/*******************************************************************************
 * ���ݴ�ӡ����
 ******************************************************************************/
void DISPLAY_PrintTouch(uint16_t typeID)
{
	switch(typeID)
	{
	case CTL_ID_PRINT_TIME_START_TOUCH:
		/* ʱ��ѡ��������ɿ�ʼ��ӡʱ��ѡ����� */
		DISPLAY_Status.timeSelectStatus = TIME_SELECT_START_PRINT_TIME;

		/* ����ʱ��ָ�� */
		DISPLAY_Status.selectTime = &DISPLAY_Status.printTimeStart;

		TFTLCD_SetScreenId(SCREEN_ID_TIME_SELECT);

		/* ��λʱ��ѡ��ؼ�����ǰʱ�� */
		/* todo */
		break;

	case CTL_ID_PRINT_TIME_END_TOUCH:
		/* ʱ��ѡ��������ɽ�����ӡʱ��ѡ����� */
		DISPLAY_Status.timeSelectStatus = TIME_SELECT_END_PRINT_TIME;

		/* ����ʱ��ָ�� */
		DISPLAY_Status.selectTime = &DISPLAY_Status.printTimeEnd;

		TFTLCD_SetScreenId(SCREEN_ID_TIME_SELECT);

		/* ��λʱ��ѡ��ؼ�����ǰʱ�� */
		/* todo */
		break;

	case CTL_ID_CHANNAL_SELECT_CH1_TOUCH:
		DISPLAY_Status.printChannelStatus.status.bit.ch1 =
				!DISPLAY_Status.printChannelStatus.status.bit.ch1;
		TFTLCD_ChannelSelectICON(SCREEN_ID_PRINT, CTL_ID_CHANNAL_SELECT_CH1_ICON,
				DISPLAY_Status.printChannelStatus.status.bit.ch1);
		break;

	case CTL_ID_CHANNAL_SELECT_CH2_TOUCH:
		DISPLAY_Status.printChannelStatus.status.bit.ch2 =
				!DISPLAY_Status.printChannelStatus.status.bit.ch2;
		TFTLCD_ChannelSelectICON(SCREEN_ID_PRINT, CTL_ID_CHANNAL_SELECT_CH2_ICON,
				DISPLAY_Status.printChannelStatus.status.bit.ch2);
		break;

	case CTL_ID_CHANNAL_SELECT_CH3_TOUCH:
		DISPLAY_Status.printChannelStatus.status.bit.ch3 =
				!DISPLAY_Status.printChannelStatus.status.bit.ch3;
		TFTLCD_ChannelSelectICON(SCREEN_ID_PRINT, CTL_ID_CHANNAL_SELECT_CH3_ICON,
				DISPLAY_Status.printChannelStatus.status.bit.ch3);
		break;

	case CTL_ID_CHANNAL_SELECT_CH4_TOUCH:
		DISPLAY_Status.printChannelStatus.status.bit.ch4 =
				!DISPLAY_Status.printChannelStatus.status.bit.ch4;
		TFTLCD_ChannelSelectICON(SCREEN_ID_PRINT, CTL_ID_CHANNAL_SELECT_CH4_ICON,
				DISPLAY_Status.printChannelStatus.status.bit.ch4);
		break;

	case CTL_ID_CHANNAL_SELECT_CH5_TOUCH:
		DISPLAY_Status.printChannelStatus.status.bit.ch5 =
				!DISPLAY_Status.printChannelStatus.status.bit.ch5;
		TFTLCD_ChannelSelectICON(SCREEN_ID_PRINT, CTL_ID_CHANNAL_SELECT_CH5_ICON,
				DISPLAY_Status.printChannelStatus.status.bit.ch5);
		break;

	case CTL_ID_CHANNAL_SELECT_CH6_TOUCH:
		DISPLAY_Status.printChannelStatus.status.bit.ch6 =
				!DISPLAY_Status.printChannelStatus.status.bit.ch6;
		TFTLCD_ChannelSelectICON(SCREEN_ID_PRINT, CTL_ID_CHANNAL_SELECT_CH6_ICON,
				DISPLAY_Status.printChannelStatus.status.bit.ch6);
		break;

	case CTL_ID_CHANNAL_SELECT_CH7_TOUCH:
		DISPLAY_Status.printChannelStatus.status.bit.ch7 =
				!DISPLAY_Status.printChannelStatus.status.bit.ch7;
		TFTLCD_ChannelSelectICON(SCREEN_ID_PRINT, CTL_ID_CHANNAL_SELECT_CH7_ICON,
				DISPLAY_Status.printChannelStatus.status.bit.ch7);
		break;

	case CTL_ID_CHANNAL_SELECT_CH8_TOUCH:
		DISPLAY_Status.printChannelStatus.status.bit.ch8 =
				!DISPLAY_Status.printChannelStatus.status.bit.ch8;
		TFTLCD_ChannelSelectICON(SCREEN_ID_PRINT, CTL_ID_CHANNAL_SELECT_CH8_ICON,
				DISPLAY_Status.printChannelStatus.status.bit.ch8);
		break;

	case CTL_ID_PRINT_DEFAULT:
		PRINT_DataOut(&DISPLAY_Status.printTimeStart, &DISPLAY_Status.printTimeEnd,
				&DISPLAY_Status.printChannelStatus);
		break;

	case CTL_ID_PRINT_CUSTOM:
		break;

	default:
		break;
	}
}

/*******************************************************************************
 * ʱ��ѡ�����
 ******************************************************************************/
void DISPLAY_TimeSelectTouch(uint16_t typeID, uint8_t value)
{
	switch (typeID)
	{
	case CTL_ID_TIME_SELECT_YEAR:
		DISPLAY_Status.selectTime->year = 17 + value;
		break;

	case CTL_ID_TIME_SELECT_MONTH:
		DISPLAY_Status.selectTime->month = 1 + value;
		break;

	case CTL_ID_TIME_SELECT_DAY:
		DISPLAY_Status.selectTime->day = 1 + value;
		break;

	case CTL_ID_TIME_SELECT_HOUR:
		DISPLAY_Status.selectTime->hour = value;
		break;

	case CTL_ID_TIME_SELECT_MIN:
		DISPLAY_Status.selectTime->min = value;
		break;

	case CTL_ID_TIME_SELECT_CANCEL:
		memset(DISPLAY_Status.selectTime, 0, sizeof(FILE_RealTimeTypedef));
		TFTLCD_SetScreenId(SCREEN_ID_PRINT);
		break;

	case CTL_ID_TIME_SELECT_OK:
		/* ����ѡ�����ʱ��ѡ����棬����ʱ�� */
		TimeSelectReturn();
		break;

	default:
		break;
	}
}

/*******************************************************************************
 * function:�������ý��棬�����û���������
 */
void DISPLAY_SetPasswordTouch(uint16_t typeID)
{
	switch (typeID)
	{
	case CTL_ID_SET_PASSWORD_NUMB_1:
		PasswordSelect('1');
		break;

	case CTL_ID_SET_PASSWORD_NUMB_2:
		PasswordSelect('2');
		break;

	case CTL_ID_SET_PASSWORD_NUMB_3:
		PasswordSelect('3');
		break;

	case CTL_ID_SET_PASSWORD_NUMB_4:
		PasswordSelect('4');
		break;

	case CTL_ID_SET_PASSWORD_NUMB_5:
		PasswordSelect('5');
		break;

	case CTL_ID_SET_PASSWORD_NUMB_6:
		PasswordSelect('6');
		break;

	case CTL_ID_SET_PASSWORD_NUMB_7:
		PasswordSelect('7');
		break;

	case CTL_ID_SET_PASSWORD_NUMB_8:
		PasswordSelect('8');
		break;

	case CTL_ID_SET_PASSWORD_NUMB_9:
		PasswordSelect('9');
		break;

	case CTL_ID_SET_PASSWORD_NUMB_0:
		PasswordSelect('0');
		break;

	case CTL_ID_SET_PASSWORD_CLEAR:
		/* ������뻺���λָʾ */
		memcpy(DISPLAY_Status.passwordBuffer, "    ", 4);
		DISPLAY_Status.passwordBufferIndex = 0;
		TFTLCD_SetPasswordUpdate(DISPLAY_Status.passwordBufferIndex);
		break;

	case CTL_ID_SET_PASSWORD_ENTER:
		if ((DISPLAY_Status.passwordBuffer[0] == PARAM_DeviceParam.password[0])
			&& (DISPLAY_Status.passwordBuffer[1] == PARAM_DeviceParam.password[1])
			&& (DISPLAY_Status.passwordBuffer[2] == PARAM_DeviceParam.password[2])
			&& (DISPLAY_Status.passwordBuffer[3] == PARAM_DeviceParam.password[3]))
		{
			TFTLCD_SetScreenId(SCREEN_ID_SET_ALARM_LIMIT);
		}
		else/* ������� */
		{
			/* ������뻺���λָʾ */
			memcpy(DISPLAY_Status.passwordBuffer, "    ", 4);
			DISPLAY_Status.passwordBufferIndex = 0;
			TFTLCD_SetPasswordUpdate(DISPLAY_Status.passwordBufferIndex);
		}
		break;
	}
}

/*******************************************************************************
 * function�����ñ���������ֵ
 */
void DISPLAY_SetAlarmLimitTouch(uint16_t typeID)
{
	switch (typeID)
	{
	case CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH1:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[0].alarmValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH1:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[0].alarmValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH1:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[0].perwarningValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH1:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[0].perwarningValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH2:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[1].alarmValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH2:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[1].alarmValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH2:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[1].perwarningValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH2:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[1].perwarningValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH3:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[2].alarmValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH3:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[2].alarmValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH3:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[2].perwarningValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH3:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[2].perwarningValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH4:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[3].alarmValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH4:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[3].alarmValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH4:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[3].perwarningValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH4:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[3].perwarningValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_SAVE:
		PARAM_SaveStruct(&PARAM_DeviceParam);
		break;

	default:
		break;
	}
}

/*******************************************************************************
 * function�����ñ���������ֵ
 */
void DISPLAY_SetAlarmLimit2Touch(uint16_t typeID)
{
	switch (typeID)
	{
	case CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH5:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[4].alarmValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH5:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[4].alarmValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH5:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[4].perwarningValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH5:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[4].perwarningValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH6:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[5].alarmValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH6:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[5].alarmValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH6:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[5].perwarningValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH6:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[5].perwarningValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH7:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[6].alarmValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH7:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[6].alarmValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH7:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[6].perwarningValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH7:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[6].perwarningValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH8:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[7].alarmValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH8:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[7].alarmValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH8:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[7].perwarningValueUp);
		break;

	case CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH8:
		GetAlarmLimitValue(&PARAM_DeviceParam.channel[7].perwarningValueLow);
		break;

	case CTL_ID_SET_ALARM_LIMIT_2_SAVE:
		PARAM_SaveStruct(&PARAM_DeviceParam);
		break;

	default:
		break;
	}
}

/*******************************************************************************
 *
 */
void DISPLAY_SetMessageTouch(uint16_t typeID)
{
	switch (typeID)
	{
	case CTL_ID_SET_ALARM_CODE_1:
		GetAlarmCode((char*)&PARAM_DeviceParam.alarmCode[0]);
		break;

	case CTL_ID_SET_ALARM_CODE_2:
		GetAlarmCode((char*)&PARAM_DeviceParam.alarmCode[1]);
		break;

	case CTL_ID_SET_ALARM_CODE_3:
		GetAlarmCode((char*)&PARAM_DeviceParam.alarmCode[2]);
		break;

	case CTL_ID_SET_ALARM_CODE_SAVE:
		PARAM_SaveStruct(&PARAM_DeviceParam);
		break;

	default:
		break;
	}
}

/*******************************************************************************
 *
 */
void DISPLAY_SetPasswordChangeTouch(uint16_t typeID)
{
	switch (typeID)
	{
	case CTL_ID_SET_PASSWORD_CHANGE_OLD:
		memcpy(DISPLAY_Status.passwordBuffer,
				(char*)(&TFTLCD_RecvBuffer.date.recvBuf.buf[1]), 4);
		break;
	case CTL_ID_SET_PASSWORD_CHANGE_NEW:
		memcpy(DISPLAY_Status.passwordBufferNew,
				(char*)(&TFTLCD_RecvBuffer.date.recvBuf.buf[1]), 4);
		break;
	case CTL_ID_SET_PASSWORD_CHANGE_NEW_AGAIN:
		memcpy(DISPLAY_Status.passwordBufferNewAgain,
				(char*)(&TFTLCD_RecvBuffer.date.recvBuf.buf[1]), 4);
		break;
	case CTL_ID_SET_PASSWORD_CHANGE_SAVE:
		if ((DISPLAY_Status.passwordBuffer[0] == PARAM_DeviceParam.password[0])
				&& (DISPLAY_Status.passwordBuffer[1] == PARAM_DeviceParam.password[1])
				&& (DISPLAY_Status.passwordBuffer[2] == PARAM_DeviceParam.password[2])
				&& (DISPLAY_Status.passwordBuffer[3] == PARAM_DeviceParam.password[3]))
		{
			if ((DISPLAY_Status.passwordBufferNew[0] == DISPLAY_Status.passwordBufferNewAgain[0])
				&& (DISPLAY_Status.passwordBufferNew[1] == DISPLAY_Status.passwordBufferNewAgain[1])
				&& (DISPLAY_Status.passwordBufferNew[2] == DISPLAY_Status.passwordBufferNewAgain[2])
				&& (DISPLAY_Status.passwordBufferNew[3] == DISPLAY_Status.passwordBufferNewAgain[3]))
			{
				memcpy(PARAM_DeviceParam.password, DISPLAY_Status.passwordBufferNewAgain, 4);
				PARAM_SaveStruct(&PARAM_DeviceParam);
				TFTLCD_TextValueUpdate(SCREEN_ID_SET_CHANGE_PASSWORD, CTL_ID_SET_PASSWORD_CHANGE_NEW_AGAIN,
					"�����޸ĳɹ�", 12);
			}
			else
			{
				TFTLCD_TextValueUpdate(SCREEN_ID_SET_CHANGE_PASSWORD, CTL_ID_SET_PASSWORD_CHANGE_NEW_AGAIN,
					"���������", 10);
			}
		}
		else
		{
			TFTLCD_TextValueUpdate(SCREEN_ID_SET_CHANGE_PASSWORD, CTL_ID_SET_PASSWORD_CHANGE_NEW_AGAIN,
					"���������", 10);
		}
		
		TFTLCD_TextValueUpdate(SCREEN_ID_SET_CHANGE_PASSWORD, CTL_ID_SET_PASSWORD_CHANGE_OLD,
					"    ", 4);
		TFTLCD_TextValueUpdate(SCREEN_ID_SET_CHANGE_PASSWORD, CTL_ID_SET_PASSWORD_CHANGE_NEW,
					"    ", 4);
		break;

	default:
		break;
	}
}

/*******************************************************************************
 * function:ʱ��ѡ�������ȷ���������ı�
 */
static void TimeSelectReturn(void)
{
	switch(DISPLAY_Status.timeSelectStatus)
	{
	case TIME_SELECT_START_PRINT_TIME:
		TFTLCD_SelectTimeUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TIME_START_TEXT,
				DISPLAY_Status.selectTime);
		TFTLCD_SetScreenId(SCREEN_ID_PRINT);
		break;

	case TIME_SELECT_END_PRINT_TIME:
		TFTLCD_SelectTimeUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TIME_END_TEXT,
				DISPLAY_Status.selectTime);
		TFTLCD_SetScreenId(SCREEN_ID_PRINT);
		break;

	default:
		break;
	}
}

/*******************************************************************************
 * function�������������û���ֵѡ�������洦��
 */
static void PasswordSelect(char numb)
{
	/* ���벻�ܳ���4λ�� */
	if (DISPLAY_Status.passwordBufferIndex < 4)
	{
		/* ѡ�����ֵ�����뻺�� */
		DISPLAY_Status.passwordBuffer[DISPLAY_Status.passwordBufferIndex] = numb;
		/* ���»���λ */
		DISPLAY_Status.passwordBufferIndex++;
		/* ���½����ı�������λ�� */
		TFTLCD_SetPasswordUpdate(DISPLAY_Status.passwordBufferIndex);
	}
}

/*******************************************************************************
 *
 */
static void GetAlarmLimitValue(float* value)
{
	char str[6];
	uint8_t length = TFTLCD_RecvBuffer.bufferSize - 12;

	/* ��ȡ�ı�ֵ��ֵ�ĳ���Ϊ���ճ���-������Ϣ */
	memcpy(str, (char*)(&TFTLCD_RecvBuffer.date.recvBuf.buf[1]), length);

//	str[length] = '\0';
	*value = atof(str);
}

/*******************************************************************************
 * function:��ȡ�����ֻ�����
 */
static void GetAlarmCode(char* code)
{
	memcpy(code, (char*)(&TFTLCD_RecvBuffer.date.recvBuf.buf[1]), 11);
}






