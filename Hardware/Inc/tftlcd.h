#ifndef __TFTLCD_H
#define __TFTLCD_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "analog.h"
#include "rt.h"
#include "file.h"
#include "public.h"

#define TFTLCD_UART					(huart2)

#define TFTLCD_PWR_ENABLE() \
		HAL_GPIO_WritePin(SCREEN_PWR_CTRL_GPIO_Port, SCREEN_PWR_CTRL_Pin, GPIO_PIN_RESET);
#define TFTLCD_PWR_DISABLE() \
		HAL_GPIO_WritePin(SCREEN_PWR_CTRL_GPIO_Port, SCREEN_PWR_CTRL_Pin, GPIO_PIN_SET);

#define TFTLCD_UART_DMA_FLAG_GL		(DMA_FLAG_GL6)

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
#define TFTLCD_CMD_SET_SCREEN				(uint16_t)(0X00B1)
#define TFTLCD_CMD_SCREEN_ID_GET			(uint16_t)(0x01B1)		/* ��ȡ���� */
#define TFTLCD_CMD_TEXT_UPDATE				(uint16_t)(0x10B1)		/* �����ı� */
#define TFTLCD_CMD_BUTTON					(uint16_t)(0x11B1)
#define TFTLCD_CMD_BUTTON_SELECT			(uint16_t)(0x11B1)		/* ���ݴ�ʣ�ѡ��ؼ��Ͱ�ť�ؼ���ͬһ��cmd */
#define TFTLCD_CMD_BATCH_UPDATE				(uint16_t)(0x12B1)
#define TFTLCD_CMD_SELECT					(uint16_t)(0x14B1)		/* ѡ��ؼ�ֵ�ϴ� */
#define TFTLCD_CMD_SET_FORE_COLOR			(uint16_t)(0x19B1)		/* �����ı��ؼ�ǰ��ɫ */
#define TFTLCD_CMD_ICON_DISP				(uint16_t)(0x23B1)		/* ͼ��ؼ���ʾ */

#define TFTLCD_CMD_CURVE_ADD_CHANNEL		(uint16_t)(0x30B1)		/* �����������ͨ�� */
#define TFTLCD_CMD_CURVE_DELETE_CHANNEL		(uint16_t)(0x31B1)		/* ����ɾ������ͨ�� */
#define TFTLCD_CMD_CURVE_SET_PARAM  		(uint16_t)(0x34B1)		/* �������ô�ֱˮƽ�����š�ƽ�� */
#define TFTLCD_CMD_CURVE_ADD_DATA_TAIL		(uint16_t)(0x32B1)		/* ָ��ͨ��ĩ����������� */
#define TFTLCD_CMD_CURVE_ADD_DATA_FRONT		(uint16_t)(0x35B1)		/* ָ��ͨ��ǰ����������� */
#define TFTLCD_CMD_CURVE_CLEAR_DATA			(uint16_t)(0x33B1)		/* ���ָ��ͨ�������� */

#define TFTLCD_ALARM_COLOR					(0xF800)				/* ������ɫ */
#define TFTLCD_PERWARM_COLOR				(0xFF10)				/* Ԥ����ɫ */
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

/* ��ʷ���߽���ؼ�ID */
typedef enum
{
	CTL_ID_HIS_DATA_CURVE = 6,
} TFTLCD_HisDataCurveCtlIdEnum;

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
	CTL_ID_PRINT_DEFAULT,								/* Ĭ�ϴ�ӡ */
	CTL_ID_PRINT_CUSTOM,								/* �Զ����ӡ */
	CTL_ID_PRINT_MODE_SELECT_INTEGRATED,				/* ѡ��һ��ʽ��ӡ��ģʽ */
	CTL_ID_PRINT_MODE_SELECT_BLE,						/* ѡ��������ӡģʽ */
	CTL_ID_PRINT_LINK_BLE,								/* ����������ӡ�� */
	CTL_ID_PRINT_TEXT_BLE_STATUS,						/* ������ӡ������״̬��ʾ  */
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

/* �������ý���ؼ�ID */
typedef enum
{
	CTL_ID_SET_PASSWORD_TEXT = 6,
	CTL_ID_SET_PASSWORD_NUMB_1,
	CTL_ID_SET_PASSWORD_NUMB_2,
	CTL_ID_SET_PASSWORD_NUMB_3,
	CTL_ID_SET_PASSWORD_NUMB_4,
	CTL_ID_SET_PASSWORD_NUMB_5,
	CTL_ID_SET_PASSWORD_NUMB_6,
	CTL_ID_SET_PASSWORD_NUMB_7,
	CTL_ID_SET_PASSWORD_NUMB_8,
	CTL_ID_SET_PASSWORD_NUMB_9,
	CTL_ID_SET_PASSWORD_CLEAR,
	CTL_ID_SET_PASSWORD_NUMB_0,
	CTL_ID_SET_PASSWORD_ENTER,
} TFTLCD_SetPasswordCtlIdEnum;

/* ���������޽���ؼ�ID */
typedef enum
{
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH1 = 6,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH1,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH1,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH1,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH2,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH2,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH2,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH2,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH3,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH3,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH3,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH3,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH4,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH4,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH4,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH4,
	CTL_ID_SET_ALARM_LIMIT_SAVE,
} TFTLCD_SetAlarmLimitCtlIdEnum;

/* ���������޽���2�ؼ�ID */
typedef enum
{
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH5 = 6,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH5,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH5,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH5,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH6,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH6,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH6,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH6,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH7,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH7,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH7,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH7,
	CTL_ID_SET_ALARM_LIMIT_ALARM_UP_CH8,
	CTL_ID_SET_ALARM_LIMIT_ALARM_DOWN_CH8,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_UP_CH8,
	CTL_ID_SET_ALARM_LIMIT_PERWARN_DOWN_CH8,
	CTL_ID_SET_ALARM_LIMIT_2_SAVE
} TFTLCD_SetAlarmLimit2CtlIdEnum;

/* �����������ؼ�ID */
typedef enum
{
	CTL_ID_SET_ALARM_CODE_1 = 6,
	CTL_ID_SET_ALARM_CODE_2,
	CTL_ID_SET_ALARM_CODE_3,
	CTL_ID_SET_ALARM_CODE_SAVE
} TFTLCD_SetAlarmCodeCtlIdEnum;

/* �����޸Ľ���ؼ�ID */
typedef enum
{
	CTL_ID_SET_PASSWORD_CHANGE_OLD = 6,
	CTL_ID_SET_PASSWORD_CHANGE_NEW,
	CTL_ID_SET_PASSWORD_CHANGE_NEW_AGAIN,
	CTL_ID_SET_PASSWORD_CHANGE_SAVE,
} TFTLCD_SetPasswordChangeCtlIdEnum;

/* ������Ϣ����ؼ�ID */
typedef enum
{
	CTL_ID_ABOUT_DEVICE_SN = 6,
	CTL_ID_ABOUT_DEVICE_TYPE,
	CTL_ID_ABOUT_CHANNEL_NUMB,
	CTL_ID_ABOUT_FIRM_VERSION,
	CTL_ID_ABOUT_OS_VERSION,
	CTL_ID_ABOUT_DEVICE_CAPACITY,
} TFTLCD_AboutCtlIdEnum;

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
	SCREEN_ID_SET_ALARM_LIMIT_2,				/* ����������2 */
	SCREEN_ID_SET_ALARM_CODE,					/* �������� */
	SCREEN_ID_SET_MESSAGE,						/* ����ǩ�� */
	SCREEN_ID_SET_UPDATE,						/* �̼����� */
	SCREEN_ID_SET_CHANGE_PASSWORD,				/* �޸����� */
	SCREEN_ID_ABOUT_DEVICE,						/* �����豸 */
	SCREEN_ID_ABOUT_LUGE,						/* ����·�� */
	SCREEN_ID_TIME_SELECT,
} TFTLCD_ScreenIDEnum;

/**************************Сͼ��ؼ�*********************************************/
typedef enum
{
	ICON_BAT_CHARGE,							/* ���ڳ�� */
	ICON_BAT_CAPACITY_80,						/* ��ص���>80% */
	ICON_BAT_CAPACITY_60,						/* ��ص���>60% */
	ICON_BAT_CAPACITY_40,						/* ��ص���>40% */
	ICON_BAT_CAPACITY_20,						/* ��ص���>20% */
	ICON_BAT_CAPACITY_0,						/* ��ص���>0% */
} IconBatCapacityEnum;

typedef enum
{
	ICON_SIGNAL_QUALITY_31_21,					/* 31>�ź�ǿ��>21 */
	ICON_SIGNAL_QUALITY_21_11,					/* 21>�ź�ǿ��>11 */
	ICON_SIGNAL_QUALITY_11_0,					/* 11>�ź�ǿ��>0 */
} IconSignalQualityEnum;

typedef enum
{
	ICON_ALARM_ON,								/* �б����ź� */
	ICON_ALARM_OFF,								/* �ޱ����ź� */
} IconAlarmStatusEnum;

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

	uint8_t batCtlIdH;
	uint8_t batCtlIdL;
	uint8_t batSizeH;
	uint8_t batSizeL;
	char    batCapacity[3];
} StatusBarTextTypedef;					/* ״̬���ı����� */

typedef struct
{
	uint8_t batCtlIdH;					/* ��ص���ͼ�� */
	uint8_t batCtlIdL;
	uint8_t batSizeH;
	uint8_t batSizeL;
	uint8_t batCapacityH;
	uint8_t batCapacityL;

	uint8_t signalCtlIdH;				/* �ź�ǿ��ͼ�� */
	uint8_t signalCtlIdL;
	uint8_t signalSizeH;
	uint8_t signalSizeL;
	uint8_t signalCapacityH;
	uint8_t signalCapacityL;

	uint8_t alarmCtlIdH;				/* ����ͼ�� */
	uint8_t alarmCtlIdL;
	uint8_t alarmSizeH;
	uint8_t alarmSizeL;
	uint8_t alarmCapacityH;
	uint8_t alarmCapacityL;
} StatusBarIconTypedef;					/* ״̬��ͼ����� */

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
	uint8_t dataLengthH;					/* ���ݳ��� */
	uint8_t dataLengthL;
	uint8_t data;							/* ���� */
} CurveDataTypedef;

typedef struct
{
	uint8_t ctlIdH;
	uint8_t ctlIdL;
	uint8_t channel;						/* ����ͨ�������֧��8�� */
//	union
//	{
//		CurveDataTypedef curveData;
//		uint16_t         curveColor;
//
//	};
	uint8_t dataLengthH;					/* ���ݳ��� */
	uint8_t dataLengthL;
	uint8_t data;							/* ���� */
} CurveTypedef;

typedef struct
{
	uint8_t head;									/* ֡ͷ */
	uint16_t cmd;									/* ָ�� */
	uint8_t screenIdH;								/* ����ID */
	uint8_t screenIdL;
	union
	{
		uint8_t data[TFTLCD_UART_RX_DATA_SIZE_MAX];
		StatusBarTextTypedef statusBarText;			/* ״̬���ı����� */
		StatusBarIconTypedef statusBarIcon;			/* ״̬��ͼ����� */
		AnalogTypedef        analogValue[8];		/* ģ������������ */
		HistoryDateTypedef   HistoryDate;			/* ��ʷ������������ */
		UpdateTypedef 		 update;				/* ���ݸ��� */
		CurveTypedef  		 curve;					/* ���� */
	}buffer;
	uint8_t tail[4];								/* ֡β */
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
void TFTLCD_SetScreenId(TFTLCD_ScreenIDEnum screen);
void TFTLCD_TextValueUpdate(uint16_t screenID, uint16_t ctlID, char* str, uint8_t size);
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog);
void TFTLCD_AnalogDataAlarmDisplay(ANALOG_ValueTypedef* analog);
void TFTLCD_StatusBarTextRefresh(uint16_t screenID, RT_TimeTypedef* rt, uint8_t batQuantity);
void TFTLCD_StatusBarIconRefresh(uint16_t screenID);
void TFTLCD_HistoryDataFormat(FILE_SaveStructTypedef* saveInfo, TFTLCD_HisDataCtlIdEnum typeID);
void TFTLCD_ChannelSelectICON(TFTLCD_ScreenIDEnum screen, uint16_t typeID, uint8_t status);
void TFTLCD_SelectTimeUpdate(TFTLCD_ScreenIDEnum screen, uint16_t ctlID, FILE_RealTimeTypedef* time);

void TFTLCD_UartIdleDeal(void);
ErrorStatus TFTLCD_CheckHeadTail(void);
//void TFTLCD_printTimeUpdate(FILE_RealTime* rt, CtrlID_PrintEnum ctrl);
//void TFTLCD_printChannelSelectICON(CtrlID_PrintEnum ctrl, uint8_t status);

void TFTLCD_HistoryDataCurveFormat(FILE_SaveStructTypedef* saveInfo);
void TFTLCD_SetPasswordUpdate(uint8_t numb);
#endif
