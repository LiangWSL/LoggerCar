#ifndef __GPRS_H
#define __GPRS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "exFlash.h"
#include "file.h"

/******************************************************************************/
#define GPRS_UART 						(huart2)
#define GPRS_PWR_CTRL_ENABLE() \
		HAL_GPIO_WritePin(GPRS_PWRKEY_CTRL_GPIO_Port, GPRS_PWRKEY_CTRL_Pin, GPIO_PIN_SET);
#define GPRS_PWR_CTRL_DISABLE() \
		HAL_GPIO_WritePin(GPRS_PWRKEY_CTRL_GPIO_Port, GPRS_PWRKEY_CTRL_Pin, GPIO_PIN_RESET);

#define GPRS_RST_CTRL_ENABLE() \
		HAL_GPIO_WritePin(GPRS_RST_CTRL_GPIO_Port, GPRS_RST_CTRL_Pin, GPIO_PIN_RESET);
#define GPRS_RST_CTRL_DISABLE() \
		HAL_GPIO_WritePin(GPRS_RST_CTRL_GPIO_Port, GPRS_RST_CTRL_Pin, GPIO_PIN_SET);

/******************************************************************************/
#define GPRS_PATCH_PACK_NUMB_MAX		  (30)				/* ���֧�ֲ����������� */
#define GPRS_PACK_HEAD					  (uint8_t)(0X31)
#define GPRS_PACK_TAIL					  (uint8_t)(0x32)

#define GPRS_UART_RX_DATA_SIZE_MAX		  (50)

#define GPRS_SIGNAL_QUALITY_OFFSET			(8)

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* ���ֽڶ��� */

typedef struct
{
	uint8_t recvBuffer[GPRS_UART_RX_DATA_SIZE_MAX];			/* ���ջ��� */
	uint8_t bufferSize;										/* �����С */
} GPRS_BufferStatusTypedef;

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
	FILE_InfoTypedef dataPack[GPRS_PATCH_PACK_NUMB_MAX];	/* m�����ݵ� */
	uint8_t tail;											/* ����β */
	uint8_t verifyData;										/* У������ */
} GPRS_StructTypedef;

#pragma pack(pop)
/******************************************************************************/
//extern uint8_t GPRS_RecvBuffer[GPRS_UART_RX_DATA_SIZE_MAX];
extern GPRS_BufferStatusTypedef GPRS_BufferStatus;

/******************************************************************************/
void GPRS_Init(void);
void GPRS_StructInit(GPRS_StructTypedef* sendBuf);
void GPRS_SendCmd(char* str);
void GPRS_UartIdleDeal(void);
void GPRS_SendProtocol(GPRS_StructTypedef* sendBuf, uint16_t patchPack);
void GPRS_RstModule(void);
uint8_t GPRS_GetSignalQuality(uint8_t* buf);
#endif
