#ifndef __GPRS_H
#define __GPRS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "param.h"
#include "exFlash.h"

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
#define GPRS_PATCH_PACK_NUMB_MAX		  (20)				/* ���֧�ֲ����������� */
#define GPRS_PACK_HEAD					  (uint8_t)(0X31)
#define GPRS_PACK_TAIL					  (uint8_t)(0x32)

#define GPRS_UART_RX_DATA_SIZE_MAX		  (50)
#define GPRS_SIGNAL_QUALITY_OFFSET		  (8)

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* ���ֽڶ��� */

typedef struct
{
	uint8_t recvBuffer[GPRS_UART_RX_DATA_SIZE_MAX];			/* ���ջ��� */
	uint8_t bufferSize;										/* �����С */
} GPRS_RecvBufferTypedef;

typedef struct
{
	uint8_t  year;								/* ʱ�� */
	uint8_t  month;
	uint8_t  day;
	uint8_t  hour;
	uint8_t  min;
	uint8_t  sec;
	uint8_t  batteryLevel;						/* ��ص��� */
	uint8_t  externalPowerStatus;				/* �ⲿ���״̬ */
	uint32_t longitude;							/* ���� */
	uint32_t latitude;							/* γ�� */
	uint8_t  resever;							/* ���� */
	uint16_t analogValue[8];					/* ģ����ֵ */
} GPRS_SendInfoTypedef;

typedef struct
{
	uint8_t head;											/* ����ͷ */
	uint8_t dateSizeH;										/* �ֽ��� ��λ */
	uint8_t dateSizeL;										/* �ֽ��� ��λ */
	char seriaNumber[10];									/* SN�� */
	LocationTypdEnum locationType;							/* ��λ��־ */
	uint8_t firmwareVersion;								/* �̼��汾 */
	uint8_t recordInterval;									/* ��¼��� */
	uint8_t overLimitInterval;								/* ������ */
	uint8_t resever[6];										/* Ԥ�� */
	uint8_t exitAnalogChannelNumb;							/* ģ����nͨ���� */
	ParamTypeTypedef param[ANALOG_CHANNEL_NUMB];			/* n��ͨ������ */
	uint8_t dataPackNumbH;									/* ��������m��λ */
	uint8_t dataPackNumbL;									/* ��������m��λ */
	GPRS_SendInfoTypedef dataPack[GPRS_PATCH_PACK_NUMB_MAX];		/* m�����ݵ� */
	uint8_t tail;											/* ����β */
	uint8_t verifyData;										/* У������ */
} GPRS_SendBufferTypedef;

#pragma pack(pop)

/******************************************************************************/
extern GPRS_RecvBufferTypedef GPRS_RecvBuffer;
extern uint8_t GPRS_signalQuality;			/* GPRS�ź����� */
extern GPRS_SendBufferTypedef GPRS_SendBuffer;

/******************************************************************************/
void GPRS_Init(void);
void GPRS_SendCmd(char* str);
void GPRS_RstModule(void);
void GPRS_SendProtocol(GPRS_SendBufferTypedef* sendBuf, uint8_t patchPack);
uint8_t GPRS_GetSignalQuality(uint8_t* buf);
void GPRS_UartIdleDeal(void);

#endif
