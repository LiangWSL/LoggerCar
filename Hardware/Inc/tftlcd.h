#ifndef __TFTLCD_H
#define __TFTLCD_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "common.h"
#include "analog.h"
#include "rt.h"
#include "file.h"

#define TFTLCD_UART					(huart4)

#define TFTLCD_PWR_ENABLE() \
		HAL_GPIO_WritePin(SCREEN_PWR_CTRL_GPIO_Port, SCREEN_PWR_CTRL_Pin, GPIO_PIN_RESET);
#define TFTLCD_PWR_DISABLE() \
		HAL_GPIO_WritePin(SCREEN_PWR_CTRL_GPIO_Port, SCREEN_PWR_CTRL_Pin, GPIO_PIN_SET);

#define TFTLCD_UART_DMA_FLAG_GL		(DMA_FLAG_GL3)

/*******************************************************************************
 * ָ���֡ͷ��֡β
 */
#define TFTLCD_CMD_HEAD								(0XEE)
#define TFTLCD_CMD_TAIL1							(0XFF)
#define TFTLCD_CMD_TAIL2							(0XFC)
#define TFTLCD_CMD_TAIL3							(0XFF)
#define TFTLCD_CMD_TAIL4							(0XFF)

#define TFTLCD_UART_RX_DATA_SIZE_MAX				(50)

/*******************************************************************************
 * ָ���ʽ
 */
#define TFTLCD_CMD_BATCH_UPDATE				(uint16_t)(0x12B1)
#define TFTLCD_CMD_TEXT_UPDATE				(uint16_t)(0x10B1)		/* �����ı� */
#define TFTLCD_CMD_SET_SCREEN				(uint16_t)(0X00B1)
#define TFTLCD_CMD_BUTTON					(uint16_t)(0x11B1)
#define TFTLCD_CMD_BUTTON_SELECT			(uint16_t)(0x11B1)		/* ���ݴ�ʣ�ѡ��ؼ��Ͱ�ť�ؼ���ͬһ��cmd */
#define TFTLCD_CMD_SELECT					(uint16_t)(0x14B1)		/* ѡ��ؼ�ֵ�ϴ� */
#define TFTLCD_CMD_ICON_DISP				(uint16_t)(0x23B1)		/* ͼ��ؼ���ʾ */



/* �ؼ�ID */
#define CTRL_TYPE_ID_TEMP1					(uint16_t)(1)		/* �¶�1 */
#define CTRL_TYPE_ID_TEMP2					(uint16_t)(2)		/* �¶�2 */
#define CTRL_TYPE_ID_TEMP3					(uint16_t)(3)		/* �¶�3 */
#define CTRL_TYPE_ID_TEMP4					(uint16_t)(4)		/* �¶�4 */
#define CTRL_TYPE_ID_HUMI1					(uint16_t)(5)		/* ʪ��1 */
#define CTRL_TYPE_ID_HUMI2					(uint16_t)(6)		/* ʪ��2 */
#define CTRL_TYPE_ID_HUMI3					(uint16_t)(7)		/* ʪ��3 */
#define CTRL_TYPE_ID_HUMI4					(uint16_t)(8)		/* ʪ��4 */
#define CTRL_TYPE_ID_REAL_TIME				(uint16_t)(9)		/* ʵʱʱ�� */

/******************************************************************************/
/* ������ */
typedef enum
{
	SCREEN_ID_START,							/* �������� */
	SCREEN_ID_MENU,								/* ������ */
	SCREEN_ID_CUR_DATA,							/* ��ǰ���� */
	SCREEN_ID_HIS_DATA,							/* ��ʷ���� */
	SCREEN_ID_DATA_EXPORT,						/* ���ݵ��� */
	SCREEN_ID_PRINT,							/* ��ӡ���� */
	SCREEN_ID_ABOUT,							/* ���� */
	SCREEN_ID_ADD_DEVICES,						/* ����豸 */
	SCREEN_ID_SETTING,							/* ���� */
	SCREEN_ID_SETTING_MENU,						/* ���ò˵� */
	SCREEN_ID_PRINT_SETTING,					/* ��ӡ���� */
	SCREEN_ID_SETTING_ALARM,					/* ��ʪ�������ޱ������� */
	SCREEN_ID_SETTING_ALARM_CODE,				/* ������������ */
 	SCREEN_ID_UPDATE,							/* ϵͳ���� */
 	SCREEN_ID_CURVE,							/* ���� */
 	SCREEN_ID_PRINT_TIME_SELECT,				/* ��ӡʱ��ѡ�� */
} TFTLCD_ScreenIDEnum;

/******************************************************************************/
/* ��ӡ����ؼ���� */
typedef enum
{
	PRINT_CTRL_ID_BACK = 1,
	PRINT_CTRL_ID_SET,
	PRINT_CTRL_ID_START_TIME,
	PRINT_CTRL_ID_END_TIME,
	PRINT_CTRL_ID_START_TIME_BUTTON,
	PRINT_CTRL_ID_END_TIME_BUTTON,
	PRINT_CTRL_ID_START_PRINT,
	PRINT_CTRL_ID_CHANNEL_1_ICON,
	PRINT_CTRL_ID_CHANNEL_2_ICON,
	PRINT_CTRL_ID_CHANNEL_3_ICON,
	PRINT_CTRL_ID_CHANNEL_4_ICON,
	PRINT_CTRL_ID_CHANNEL_5_ICON,
	PRINT_CTRL_ID_CHANNEL_6_ICON,
	PRINT_CTRL_ID_CHANNEL_7_ICON,
	PRINT_CTRL_ID_CHANNEL_8_ICON,
	PRINT_CTRL_ID_CHANNEL_1_BUTTON,
	PRINT_CTRL_ID_CHANNEL_2_BUTTON,
	PRINT_CTRL_ID_CHANNEL_3_BUTTON,
	PRINT_CTRL_ID_CHANNEL_4_BUTTON,
	PRINT_CTRL_ID_CHANNEL_5_BUTTON,
	PRINT_CTRL_ID_CHANNEL_6_BUTTON,
	PRINT_CTRL_ID_CHANNEL_7_BUTTON,
	PRINT_CTRL_ID_CHANNEL_8_BUTTON,
}CtrlID_PrintEnum;

/* ��ӡʱ��ѡ�����ؼ���� */
typedef enum
{
	TIME_SELECT_CTRL_ID_YEAR,
	TIME_SELECT_CTRL_ID_MONTH,
	TIME_SELECT_CTRL_ID_DAY,
	TIME_SELECT_CTRL_ID_HOUR,
	TIME_SELECT_CTRL_ID_MIN,
	TIME_SELECT_CTRL_ID_CANCEL,
	TIME_SELECT_CTRL_ID_OK
} CtrlID_TimeSelectEnum;



/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* ���ֽڶ��� */

typedef struct
{
	uint8_t ctrlIdH;											/* ����ID */
	uint8_t ctrlIdL;
	uint8_t sizeH;
	uint8_t sizeL;
	char value[5];				/* �����������4���ֽ� */
} BatchUpdateTypedef;

typedef struct
{
	char year[4];
	char str1;
	char month[2];
	char str2;
	char day[2];
	char str3;
	char hour[2];
	char str4;
	char min[2];
	char str5;
	char sec[2];
} TFTLCD_TimeUpdateTypedef;

typedef struct
{
	uint8_t ctrlIdH;											/* ����ID */
	uint8_t ctrlIdL;
	union
	{
		char date[TFTLCD_UART_RX_DATA_SIZE_MAX];
		TFTLCD_TimeUpdateTypedef time;
	}value;

} UpdateTypedef;

typedef struct
{
	uint8_t head;												/* ֡ͷ */
	uint16_t cmd;												/* ָ�� */
	uint8_t screenIdH;											/* ����ID */
	uint8_t screenIdL;
	union
	{
		BatchUpdateTypedef batchDate[8];		/* ������������ */
		UpdateTypedef data;						/* buffer */
	}buf;
	uint8_t tail[4];											/* ֡β */
} TFTLCD_SendBufferTypedef;

/******************************************************************************/
/* �������� */
typedef struct
{
	uint8_t head;												/* ֡ͷ */
	uint16_t cmd;												/* ָ�� */
	uint8_t screenIdH;											/* ����ID */
	uint8_t screenIdL;
	uint8_t ctrlIDH;
	uint8_t ctrlIDL;
	uint8_t buf[TFTLCD_UART_RX_DATA_SIZE_MAX];
} RecvDetail;

typedef struct
{
	union
	{
		RecvDetail recvBuf;
		uint8_t buf[TFTLCD_UART_RX_DATA_SIZE_MAX];
	} date;
	uint8_t bufferSize;											/* �����С */
} TFTLCD_RecvBufferTypedef;

#pragma pack(pop)

/******************************************************************************/
extern TFTLCD_RecvBufferTypedef TFTLCD_RecvBuffer;

/******************************************************************************/
void TFTLCD_Init(void);
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog);
void TFTLCD_RealtimeRefresh(RT_TimeTypedef* rt);
void TFTLCD_UartIdleDeal(void);
ErrorStatus TFTLCD_CheckHeadTail(void);
void TFTLCD_printTimeUpdate(FILE_RealTime* rt, CtrlID_PrintEnum ctrl);
void TFTLCD_printChannelSelectICON(CtrlID_PrintEnum ctrl, uint8_t status);



#endif
