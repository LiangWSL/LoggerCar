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

#define TFTLCD_UART_RX_DATA_SIZE_MAX				(100)

/*******************************************************************************
 * ָ���ʽ
 */
#define TFTLCD_CMD_SCREEN_ID_CHANGE			(uint16_t)(0x00B1)		/* �л����� */
#define TFTLCD_CMD_SCREEN_ID_GET			(uint16_t)(0x01B1)		/* ��ȡ���� */
#define TFTLCD_CMD_BATCH_UPDATE				(uint16_t)(0x12B1)
#define TFTLCD_CMD_TEXT_UPDATE				(uint16_t)(0x10B1)		/* �����ı� */
#define TFTLCD_CMD_SET_SCREEN				(uint16_t)(0X00B1)
#define TFTLCD_CMD_BUTTON					(uint16_t)(0x11B1)
#define TFTLCD_CMD_BUTTON_SELECT			(uint16_t)(0x11B1)		/* ���ݴ�ʣ�ѡ��ؼ��Ͱ�ť�ؼ���ͬһ��cmd */
#define TFTLCD_CMD_SELECT					(uint16_t)(0x14B1)		/* ѡ��ؼ�ֵ�ϴ� */
#define TFTLCD_CMD_ICON_DISP				(uint16_t)(0x23B1)		/* ͼ��ؼ���ʾ */


/****************************�ؼ�IDö��******************************************/
/* ״̬�������ؼ�ID */
typedef enum
{
	CTL_ID_REALTIME,
	CTL_ID_BAT_QUANTITY,
	CTL_ID_BAT_QUANTITY_PERCENT,
	CTL_ID_SIGNAL_QUALITY,
	CTL_ID_ALARM_ICON
} TFTLCD_CommonCtlIdEnum;

/* ʵʱ���ݽ���ؼ�ID */
typedef enum
{
	CTL_ID_DATA_CH1 = 6,
	CTL_ID_DATA_CH2,
	CTL_ID_DATA_CH3,
	CTL_ID_DATA_CH4,
	CTL_ID_DATA_CH5,
	CTL_ID_DATA_CH6,
	CTL_ID_DATA_CH7,
	CTL_ID_DATA_CH8,
	CTL_ID_DATA_CH9,
	CTL_ID_DATA_CH10,
	CTL_ID_DATA_CH11,
	CTL_ID_DATA_CH12,
	CTL_ID_DATA_CH13,
	CTL_ID_DATA_CH14,
} TFTLCD_DataCtlIdEnum;

/* ��ʷ���ݽ���ؼ�ID */
typedef enum
{
	CTL_ID_DIS_DATA_1 = 6,
	CTL_ID_DIS_DATA_2,
	CTL_ID_DIS_DATA_3,
	CTL_ID_DIS_DATA_4,
	CTL_ID_PAGE_UP,
	CTL_ID_PAGE_DOWN,
	CTL_ID_DIS_CURVE,
} TFTLCD_HisDataCtlIdEnum;

/* ���ݴ�ӡ����ؼ�ID */
typedef enum
{
	CTL_ID_PRINT_TIME_START_TEXT = 6,
	CTL_ID_PRINT_TIME_END_TEXT,
	CTL_ID_CHANNAL_SELECT_CH1_ICON,
	CTL_ID_CHANNAL_SELECT_CH2_ICON,
	CTL_ID_CHANNAL_SELECT_CH3_ICON,
	CTL_ID_CHANNAL_SELECT_CH4_ICON,
	CTL_ID_CHANNAL_SELECT_CH5_ICON,
	CTL_ID_CHANNAL_SELECT_CH6_ICON,
	CTL_ID_CHANNAL_SELECT_CH7_ICON,
	CTL_ID_CHANNAL_SELECT_CH8_ICON,
	CTL_ID_PRINT_TIME_START_TOUCH,
	CTL_ID_PRINT_TIME_END_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH1_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH2_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH3_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH4_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH5_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH6_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH7_TOUCH,
	CTL_ID_CHANNAL_SELECT_CH8_TOUCH,
} TFTLCD_PrintDataCtlIdEnum;

/* ʱ��ѡ�����ؼ�ID */
typedef enum
{
	CTL_ID_TIME_SELECT_YEAR,
	CTL_ID_TIME_SELECT_MONTH,
	CTL_ID_TIME_SELECT_DAY,
	CTL_ID_TIME_SELECT_HOUR,
	CTL_ID_TIME_SELECT_MIN,
	CTL_ID_TIME_SELECT_CANCEL,
	CTL_ID_TIME_SELECT_OK,
} TFTLCD_TimeSelectCtlIdEnum;

/****************************����IDö��******************************************/
/* ������ */
typedef enum
{
	SCREEN_ID_START,							/* �������� */
	SCREEN_ID_CUR_DATA_2CH,						/* ʵʱ���� 2ͨ�� */
	SCREEN_ID_CUR_DATA_4CH,						/* ʵʱ���� 4ͨ�� */
	SCREEN_ID_CUR_DATA_8CH,						/* ʵʱ���� 8ͨ�� */
	SCREEN_ID_CUR_DATA_14CH,					/* ʵʱ���� 14ͨ�� */
	SCREEN_ID_HIS_DATA,							/* ��ʷ���� */
	SCREEN_ID_HIS_DATA_CURVE,					/* ��ʷ���� ���� */
	SCREEN_ID_PRINT,							/* ���ݴ�ӡ */
	SCREEN_ID_PRINT_DETAIL,						/* ���ݴ�ӡ������ϸ */
	SCREEN_ID_DATA_EXPORT,						/* ���ݵ��� */
	SCREEN_ID_SET_PASSWORD,						/* ���� */
	SCREEN_ID_SET_ALARM_LIMIT,					/* ���������� */
	SCREEN_ID_SET_ALARM_CODE,					/* �������� */
	SCREEN_ID_SET_MESSAGE,						/* ����ǩ�� */
	SCREEN_ID_SET_UPDATE,						/* �̼����� */
	SCREEN_ID_SET_CHANGE_PASSWORD,				/* �޸����� */
	SCREEN_ID_ABOUT_DEVICE,						/* �����豸 */
	SCREEN_ID_ABOUT_LUGE,						/* ����·�� */
	SCREEN_ID_PRINT_TIME_SELECT,
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
	uint8_t timeCtlIdH;
	uint8_t timeCtlIdL;
	uint8_t timeSizeH;
	uint8_t timeSizeL;
	char    year[4];
	char    str1;
	char    month[2];
	char    str2;
	char    day[2];
	char    str3;
	char    hour[2];
	char    str4;
	char    min[2];

	uint8_t signalCtlIdH;
	uint8_t signalCtlIdL;
	uint8_t signalSizeH;
	uint8_t signalSizeL;
	char    signalQuality[3];
} StatusBarUpdateTypedef;					/* ״̬������ */

typedef struct
{
	uint8_t ctrlIdH;
	uint8_t ctrlIdL;
	uint8_t sizeH;
	uint8_t sizeL;
	char value[5];							/* ģ������ֵ5λ�� */
} AnalogTypedef;							/* ģ����ֵ���� */

typedef struct
{
	uint8_t ctrlIdH;
	uint8_t ctrlIdL;
	uint8_t sizeH;
	uint8_t sizeL;
	char value[5];
} HistoryDateTypedef;						/* ��ʷ���ݸ��� */

typedef struct
{
	uint8_t ctrlIdH;											/* ����ID */
	uint8_t ctrlIdL;
	union
	{
		char date[TFTLCD_UART_RX_DATA_SIZE_MAX];
//		TFTLCD_TimeUpdateTypedef time;
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
		uint8_t data[TFTLCD_UART_RX_DATA_SIZE_MAX];
		union
		{
			StatusBarUpdateTypedef statusBarUpdate;		/* ״̬���������� */
			AnalogTypedef          analogValue[8];		/* ģ������������ */
			HistoryDateTypedef     HistoryDate;			/* ��ʷ������������ */
		} batch;
		UpdateTypedef update;						/* buffer */
	}buffer;
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

typedef struct
{
	TFTLCD_ScreenIDEnum curScreenID;							/* ��ǰ����ID */
} TFTLCD_StatusTypedef;

#pragma pack(pop)

/******************************************************************************/
extern TFTLCD_RecvBufferTypedef TFTLCD_RecvBuffer;
extern TFTLCD_SendBufferTypedef TFTLCD_SendBuffer;
extern TFTLCD_StatusTypedef TFTLCD_status;

/******************************************************************************/
void TFTLCD_Init(void);
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog);
void TFTLCD_StatusBarTextRefresh(uint16_t screenID, RT_TimeTypedef* rt, uint8_t batQuantity);
void TFTLCD_HistoryDataFormat(FILE_SaveInfoTypedef* saveInfo, TFTLCD_HisDataCtlIdEnum typeID);
void TFTLCD_ChannelSelectICON(TFTLCD_ScreenIDEnum screen, uint16_t typeID, uint8_t status);

void TFTLCD_UartIdleDeal(void);
ErrorStatus TFTLCD_CheckHeadTail(void);
//void TFTLCD_printTimeUpdate(FILE_RealTime* rt, CtrlID_PrintEnum ctrl);
void TFTLCD_printChannelSelectICON(CtrlID_PrintEnum ctrl, uint8_t status);



#endif
