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
#define BLE_CMD_OK_RESPOND				("OK")
/* ����ATָ�� */
#define BLE_CMD_AT_TEST					("AT\r\n")
#define BLE_CMD_AT_TEST_RESPOND			(BLE_CMD_OK_RESPOND)
#define BLE_CMD_SCAN_DEVICE				("AT+SCAN=1\r\n")
#define BLE_CMD_SCAN_DEVICE_RESPOND		("LG-PB")			/* ע�⣺����ط�Ϊ������ӡ������ */
#define BLE_CMD_SCAN_FINISH_RESPOND		("+SCAN}")			/* �������� */
#define BLE_CMD_STOP_SCAN				("AT+SCAN=0\r\n")
#define BLE_CMD_STOP_SCAN_RESPOND		(BLE_CMD_OK_RESPOND)
#define BLE_CMD_SPP_CONNECTED_RESPOND	(BLE_CMD_OK_RESPOND)

#define BLE_SCAN_DEVICE_INDEX_OFFSET	(8)					/* ɨ���豸���ƫ�� */
#define BLE_SCAN_CONNECT_INDEX_OFFSET	(11)

/******************************************************************************/
typedef enum
{
	BLE_MODE_TEST,						/* ���� */
	BLE_MODE_TEST_FINISH,
	BLE_MODE_SCAN,						/* �������� */
	BLE_MODE_SCAN_DEVICE,				/* �����豸 */
	BLE_MODE_SCAN_FINISH,
	BLE_MODE_STOP_SCAN,					/* ֹͣ���� */
	BLE_MODE_STOP_SCAN_FINISH,
	BLE_MODE_SPP_CONNECTED,				/* �������� */
	BLE_MODE_SPP_CONNECTED_FINISH,
	BLE_MODE_LINK_DEVICE,				/* �������ӳɹ� */
} BLE_ModeEnum;

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
PrintModeEnum BLE_LinkPrint(void);


#endif
