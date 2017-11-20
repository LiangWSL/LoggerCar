#ifndef __GPRS_H
#define __GPRS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "param.h"
#include "exFlash.h"
#include "analog.h"
#include "file.h"
#include "rt.h"

/******************************************************************************/
#define GPRS_UART 						(huart1)
#define GPRS_UART_DMA_RX_FLAG			(DMA_FLAG_GL5)

#define GPRS_PWR_CTRL_ENABLE() \
		HAL_GPIO_WritePin(O_GSM_PWR_GPIO_Port, O_GSM_PWR_Pin, GPIO_PIN_SET);
#define GPRS_PWR_CTRL_DISABLE() \
		HAL_GPIO_WritePin(O_GSM_PWR_GPIO_Port, O_GSM_PWR_Pin, GPIO_PIN_RESET);

#define GPRS_RST_CTRL_ENABLE() \
		HAL_GPIO_WritePin(O_GSM_RST_GPIO_Port, O_GSM_RST_Pin, GPIO_PIN_RESET);
#define GPRS_RST_CTRL_DISABLE() \
		HAL_GPIO_WritePin(O_GSM_RST_GPIO_Port, O_GSM_RST_Pin, GPIO_PIN_SET);

/******************************************************************************/
#define GPRS_PATCH_PACK_NUMB_MAX		  	(20)				/* ���֧�ֲ����������� */
#define GPRS_PACK_HEAD					  	(0X37)
#define GPRS_PACK_TAIL					  	(0x38)

#define GPRS_UART_RX_DATA_SIZE_MAX		  	(50)
#define GPRS_SIGNAL_QUALITY_OFFSET		  	(8)

#define GPRS_MESSAGE_BYTES_MAX			  	(70)

#define GPRS_PARAM_DATA_VERSION				(0x02)
#define GPRS_PARAM_DEVICE_TYPE_CODE			(0x0A0A)
#define GPRS_PARAM_FIRMWARE_VERSION			(0x01)
#define GPRS_PARAM_MESSAGE_PACK_VERSION		(0x01)
#define GPRS_PARAM_DATA_PACK_VERSION		(0x01)

/******************************************************************************/
typedef enum
{
	GPRS_PACK_TYPE_MESSAGE = 0x01,						/* ���Ű� */
	GPRS_PACK_TYPE_DATA,								/* ���ݰ� */
} GPRS_PackTypeEnum;

typedef enum
{
	GPRS_RECORD_STATUS_CURRENT,							/* ʵʱ���� */
	GPRS_RECORD_STATUS_RECORD,							/* ��ʷ���� */
} GPRS_RecordStatusEnum;								/* ���ݰ��������� */

typedef enum
{
	GPRS_EXTERNAL_PWR_OFF,								/* ���ⲿ��Դ */
	GPRS_EXTERNAL_PWR_ON,								/* ���ⲿ��Դ */
} GPRS_ExternalPowerStatusEnum;							/* �ⲿ��Դ״̬ */

typedef enum
{
	GPRS_GET_CMD_DISABLE,								/* �޲��� */
	GPRS_GET_CMD_ENABLE,								/* ƽ̨�в�����Ҫ�·����豸 */
} GPRS_GetCmdStatusEnum;

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* ���ֽڶ��� */
/******************************************************************************/
/* ���ղ��� */
typedef struct
{
	uint8_t packVersion;								/* ����汾 */
	uint8_t packSize;									/* ���峤�� */
	ErrorStatus recvResult;								/* ���ս�� */
} RecvMessageBufferTypedef;

typedef struct
{
	uint8_t packVersion;								/* ����汾 */
	uint8_t packSizeH;									/* ���峤�� */
	uint8_t packSizeL;
	GPRS_RecordStatusEnum recordStatus;					/* ��¼��� */
	ErrorStatus recvResult;								/* ���ս�� */
	GPRS_GetCmdStatusEnum getCmdStatus;					/* ƽ̨�·�ָ���־ */
} RecvDataBufferTypedef;

typedef struct
{
	uint8_t head;										/* ����ͷ */
	uint8_t dataSizeH;									/* �ֽ���H */
	uint8_t dataSizeL;
	char    serialNumber[10];							/* SN�� */
	uint8_t serverYear;									/* ������ʱ�� */
	uint8_t serverMonth;
	uint8_t serverDay;
	uint8_t serverHour;
	uint8_t serverMin;
	uint8_t serverSec;
	uint8_t packCountH;									/* ���ݰ����H */
	uint8_t packCountL;									/* ���ݰ����L */
	GPRS_PackTypeEnum packType;							/* �������� */
	union
	{
		RecvMessageBufferTypedef MessageBuffer;			/* ���Ű����� */
		RecvDataBufferTypedef    DataBuffer;			/* ���ݰ����� */
	} PackBuffer;										/* ���� */
	uint8_t tail;										/* ����β */
	uint8_t verify;										/* У��� */
} GPRS_RecvPackTypedef;

typedef struct
{
	union
	{
		uint8_t recvBuffer[GPRS_UART_RX_DATA_SIZE_MAX];			/* ���ջ��� */
		GPRS_RecvPackTypedef GPRS_RecvPack;						/* �������ݰ� */
	} buffer;
	uint8_t bufferSize;										/* �����С */
} GPRS_RecvBufferTypedef;

/******************************���Ͳ���******************************************/
typedef struct
{
	uint8_t  packVersion;								/* ����汾 */
	uint8_t  packSizeH;									/* ���峤��H */
	uint8_t  packSizeL;									/* ���峤��L */
	char     ICCID[20];									/* ICCID */
	char	 IMSI[15];									/* IMSI */
	char     IMEI[15];									/* IMEI */
	uint8_t  codeCount;									/* ������ */
	char     codeNumber[1][11];							/* ���� */
	uint8_t  contentCountH;								/* ���������ֽ���H */
	uint8_t  contentCountL;								/* ���������ֽ���L */
	char     content[GPRS_MESSAGE_BYTES_MAX];			/* �������� */
} MessageBufferTypedef;									/* ���Ű� */

typedef struct
{
	uint8_t year;										/* ʱ�� */
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t batteryLevel;								/* ��ص��� */
	GPRS_ExternalPowerStatusEnum externalPowerStatus;	/* �ⲿ���״̬ */
	GPS_LocationTypeEnum locationStatus;				/* ��λ��־ */
	uint32_t longitude;									/* ���� */
	uint32_t latitude;									/* γ�� */
	uint16_t analogValue[ANALOG_CHANNEL_NUMB];			/* ģ����ֵ */
} SendDataTypedef;

typedef struct
{
	uint8_t packVersion;								/* ����汾 */
	uint8_t packSizeH;									/* ���峤�� */
	uint8_t packSizeL;
	GPRS_RecordStatusEnum recordStatus;					/* ��¼��� */
	uint8_t channelCount;								/* ͨ���� */
	ParamTypeTypedef param[ANALOG_CHANNEL_NUMB];		/* n��ͨ������ */
	uint8_t dataPackCountH;								/* �������� */
	uint8_t dataPackCountL;
	SendDataTypedef SendData[GPRS_PATCH_PACK_NUMB_MAX];	/* ���ݵ� */
} DataBufferTypedef;

typedef struct
{
	uint8_t  head;										/* ����ͷ */
	uint8_t  dataSizeH;									/* �ֽ���H */
	uint8_t  dataSizeL;									/* �ֽ���L */
	uint8_t  dataVersion;								/* ���ݰ��汾 */
	char     serialNumber[10];							/* SN�� */
	uint8_t  deviceTypeCodeH;							/* �ͺű���H */
	uint8_t  deviceTypeCodeL;							/* �ͺű���L */
	uint8_t  firewareVersion;							/* �̼��汾 */
	uint8_t  year;										/* �ϴ�ʱ�� */
	uint8_t  month;
	uint8_t  day;
	uint8_t  hour;
	uint8_t  min;
	uint8_t  sec;
	uint8_t packCountH;									/* ���ݰ����H */
	uint8_t packCountL;									/* ���ݰ����L */
	GPRS_PackTypeEnum packType;							/* �������� */
	union
	{
		MessageBufferTypedef MessageBuffer;				/* ���Ű� */
		DataBufferTypedef    DataBuffer;				/* ���ݰ� */
	} PackBuffer;										/* ���� */
	uint8_t tail;										/* ����β */
	uint8_t verify;										/* У��� */
} GPRS_SendbufferTyepdef;								/* Э�� */

#pragma pack(pop)

/******************************************************************************/
extern GPRS_RecvBufferTypedef GPRS_RecvBuffer;
extern uint8_t GPRS_signalQuality;			/* GPRS�ź����� */

/******************************************************************************/
void GPRS_Init(void);
void GPRS_SendCmd(char* str);
void GPRS_SendData(uint16_t size);
void GPRS_RstModule(void);
uint8_t GPRS_GetSignalQuality(uint8_t* buf);
void GPRS_UartIdleDeal(void);
void GPRS_SendMessagePack(GPRS_SendbufferTyepdef* sendBuffer,
		RT_TimeTypedef curtime,	char* messageContent, uint16_t messageCount);

uint16_t GPRS_SendDataPackFromCurrent(GPRS_SendbufferTyepdef* sendBuffer,
		RT_TimeTypedef* curtime, ANALOG_ValueTypedef* analog,
		GPS_LocateTypedef* location);
uint16_t GPRS_SendDataPackFromRecord(GPRS_SendbufferTyepdef* sendBuffer,
		FILE_SaveStructTypedef* saveInfo, uint16_t sendPackCount, RT_TimeTypedef* curtime);
void GPRS_TimeAdjustWithCloud(GPRS_RecvPackTypedef* recvPack);
#endif
