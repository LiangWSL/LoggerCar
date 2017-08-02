#ifndef __TFTLCD_H
#define __TFTLCD_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "common.h"
#include "analog.h"
#include "rt.h"

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
#define TFTLCD_CMD_BATCH_UPDATE				(0x12B1)
#define TFTLCD_CMD_TIME_UPDATE				(0x10B1)

/* ��ĻID */
#define SCREEN_ID_START						(uint16_t)(0)		/* �������� */
#define SCREEN_ID_MENU						(uint16_t)(1)		/* ������ */
#define SCREEN_ID_CUR_DATA					(uint16_t)(2)		/* ��ǰ���� */
#define SCREEN_ID_HIS_DATA					(uint16_t)(3)		/* ��ʷ���� */
#define SCREEN_ID_DATA_EXPORT				(uint16_t)(4)		/* ���ݵ��� */
#define SCREEN_ID_PRINT						(uint16_t)(5)		/* ��ӡ���� */
#define SCREEN_ID_ABOUT						(uint16_t)(6)		/* ���� */
#define SCREEN_ID_ADD_DEVICES				(uint16_t)(7)		/* ����豸 */
#define SCREEN_ID_SETTING					(uint16_t)(8)		/* ���� */
#define SCREEN_ID_SETTING_MENU				(uint16_t)(9)		/* ���ò˵� */
#define SCREEN_ID_PRINT_SETTING				(uint16_t)(10)	/* ��ӡ���� */
#define SCREEN_ID_SETTING_ALARM				(uint16_t)(11)	/* ��ʪ�������ޱ������� */
#define SCREEN_ID_SETTING_ALARM_CODE		(uint16_t)(12)	/* ������������ */
#define SCREEN_ID_UPDATE					(uint16_t)(13)	/* ϵͳ���� */
#define SCREEN_ID_CURVE						(uint16_t)(14)	/* ���� */
#define SCREEN_ID_PRINT_TIME_SELECT			(uint16_t)(15)	/* ��ӡʱ��ѡ�� */

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

typedef struct
{
	uint8_t recvBuffer[TFTLCD_UART_RX_DATA_SIZE_MAX];			/* ���ջ��� */
	uint8_t bufferSize;											/* �����С */
} TFTLCD_BufferStatusTypedef;

/***********************�������¿ؼ���ֵ********************************************/
#define BATCH_UPDATE_CONTROL_MAX		(20)	/* ���֧���������Ŀؼ��� */
#define BATCH_UPDATE_DATA_MAX			(5)		/* �����ؼ���������ֽ��� */
typedef struct
{
	uint8_t controlIdH;								/* �ؼ�ID */
	uint8_t controlIdL;
	uint8_t sizeH;									/* ���¿ؼ���ֵ���� */
	uint8_t sizeL;
	char strData[BATCH_UPDATE_DATA_MAX];		/* ������ֵ */
} BatchUpdataData;

typedef struct
{
	uint8_t cmdH;									/* ָ��� */
	uint8_t cmdL;									/* ָ��� */
	uint8_t screenIdH;								/* ����ID */
	uint8_t screenIdL;
	BatchUpdataData updateData[BATCH_UPDATE_CONTROL_MAX];
												/* �ؼ����µ���ֵ */
} TFTLCD_BatchUpdateStructTypedef;

/***********************RealTime Struct****************************************/
typedef struct
{
	uint8_t cmdH;									/* ָ��� */
	uint8_t cmdL;									/* ָ��� */
	uint8_t screenIdH;								/* ����ID */
	uint8_t screenIdL;
	uint8_t controlIdH;								/* �ؼ�ID */
	uint8_t controlIdL;

	uint32_t year;
	char symbol1;
	uint16_t month;
	char symbol2;
	uint16_t day;
	char symbol3;
	uint16_t hour;
	char symbol4;
	uint16_t min;
	char symbol5;
	uint16_t sec;
} TFTLCD_RealTimeUpdateTypedef;

#pragma pack(pop)

/******************************************************************************/
void TFTLCD_Init(void);
void TFTLCD_AnalogDataRefresh(ANALOG_ValueTypedef* analog);
void TFTLCD_RealtimeRefresh(RT_TimeTypedef* rt);
void TFTLCD_UartIdleDeal(void);






#endif
