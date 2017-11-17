#ifndef __BLE_H
#define __BLE_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "display.h"

/******************************************************************************/
#define BLE_UART 						(huart3)
#define BLE_UART_DMA_RX_FLAG			(DMA_FLAG_GL3)

#define BLE_UART_RX_DATA_SIZE_MAX		(40)



/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* ���ֽڶ��� */

typedef struct
{
	uint8_t recvBuffer[BLE_UART_RX_DATA_SIZE_MAX];			/* ���ջ��� */
	uint8_t bufferSize;										/* �����С */
} BLE_RecvBufferTypedef;

#pragma pack(pop)

/******************************************************************************/
void BLE_Init(void);
void BLE_UartIdleDeal(void);
//PrintModeEnum BLE_LinkPrint(void);

void BLE_SendCmd(char* str);
#endif
