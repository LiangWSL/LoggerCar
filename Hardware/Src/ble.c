#include "ble.h"
#include "TFTLCDProcess.h"
#include "public.h"

#include "tftlcd.h"

/******************************************************************************/
BLE_RecvBufferTypedef BLE_RecvBuffer;
uint8_t BLE_RecvData[BLE_UART_RX_DATA_SIZE_MAX];

extern osThreadId tftlcdTaskHandle;


/******************************************************************************/


/*******************************************************************************
 *
 */
void BLE_Init(void)
{
//	__HAL_UART_ENABLE_IT(&BLE_UART, UART_IT_RXNE);

	UART_DMAIdleConfig(&BLE_UART, BLE_RecvData, BLE_UART_RX_DATA_SIZE_MAX);
}

/*******************************************************************************
 *
 */
void BLE_UartIdleDeal(void)
{
	uint32_t tmp_flag = 0, tmp_it_source = 0;

	tmp_flag = __HAL_UART_GET_FLAG(&BLE_UART, UART_FLAG_IDLE);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(&BLE_UART, UART_IT_IDLE);
	if((tmp_flag != RESET) && (tmp_it_source != RESET))
	{
		__HAL_DMA_DISABLE(BLE_UART.hdmarx);
		__HAL_DMA_CLEAR_FLAG(BLE_UART.hdmarx, BLE_UART_DMA_RX_FLAG);

		/* Clear Uart IDLE Flag */
		__HAL_UART_CLEAR_IDLEFLAG(&BLE_UART);

		BLE_RecvBuffer.bufferSize = BLE_UART_RX_DATA_SIZE_MAX
						- __HAL_DMA_GET_COUNTER(BLE_UART.hdmarx);

		memcpy(BLE_RecvBuffer.recvBuffer, BLE_RecvData, BLE_RecvBuffer.bufferSize);
		memset(BLE_RecvData, 0, BLE_RecvBuffer.bufferSize);

		osSignalSet(tftlcdTaskHandle, TFTLCD_TASK_BLE_RECV_ENABLE);

		BLE_UART.hdmarx->Instance->CNDTR = BLE_UART.RxXferSize;
		__HAL_DMA_ENABLE(BLE_UART.hdmarx);
	}
}



/*******************************************************************************
 * @brief ����������ӡ��
 * 		  �������Ӳ��裺
 * 		  ����ATָ��鿴�豸�Ƿ�����������ͬ��������
 * 		  ����SCAN�����ܱߵ�������
 * 		  ��������·��������ӡ�������¼���������
 * 		  �ȴ���������
 * 		  ���ݱ����������
 * 		  ������ɼ��ɿ�ʼ��ӡ
 */
//PrintModeEnum BLE_LinkPrint(void)
//{
////	volatile BLE_ModeEnum BLE_Mode = BLE_MODE_TEST;
////	char* expectString;							/* Ԥ���յ����ַ��� */
////	osEvent signal;
//	char BLE_CMD_SPP_CONNECTED[14] = "AT+SPPCONN= \r\n";
////	uint8_t timeOutCnt = 0;						/* ��ʱ���� */
////	char* bleIndex;

////	osThreadSetPriority(tftlcdTaskHandle, osPriorityRealtime);

//	while (1)
//	{
//		switch (BLE_Mode)
//		{
//		/* ���Ͳ���ָ�� */
//		case BLE_MODE_TEST:
//			BLE_SendCmd(BLE_CMD_AT_TEST);
////			expectString = BLE_CMD_AT_TEST_RESPOND;
//			BLE_Mode = BLE_MODE_TEST_FINISH;
//			break;

//		/* ����ɨ�� */
//		case BLE_MODE_SCAN:
//			BLE_SendCmd(BLE_CMD_SCAN_DEVICE);
////			expectString = BLE_CMD_SCAN_DEVICE_RESPOND;
//			BLE_Mode = BLE_MODE_SCAN_DEVICE;
//			TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
//										"����������ӡ��", 14);
//			break;

//		/* SPP���� */
//		case BLE_MODE_SPP_CONNECTED:
//			BLE_SendCmd(BLE_CMD_SPP_CONNECTED);
////			expectString = BLE_CMD_SPP_CONNECTED_RESPOND;
//			BLE_Mode = BLE_MODE_SPP_CONNECTED_FINISH;
//			break;

//		default:
//			break;
//		}

//#if 0
//		signal = osSignalWait(TFTLCD_TASK_BLE_RECV_ENABLE, 1000);
//		if (signal.status == osEventTimeout)
//		{
//			timeOutCnt++;
//		}
//		else if ((signal.value.signals & TFTLCD_TASK_BLE_RECV_ENABLE)
//						== TFTLCD_TASK_BLE_RECV_ENABLE)
//		{
//			timeOutCnt = 0;

//			if (NULL != strstr((char*)BLE_RecvBuffer.recvBuffer, expectString))
//			{
//				switch (BLE_Mode)
//				{
//				case BLE_MODE_TEST_FINISH:
//					TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
//							"����������ӡ��", 14);
//					BLE_Mode = BLE_MODE_SCAN;
//					break;

//				case BLE_MODE_SCAN_DEVICE:
//					TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
//							"�ҵ���ӡ��", 10);
//					bleIndex = strstr((char*)BLE_RecvBuffer.recvBuffer, "+SCAN=");
////					BLE_CMD_SPP_CONNECTED[BLE_SCAN_CONNECT_INDEX_OFFSET]
////						 = BLE_RecvBuffer.recvBuffer[bleIndex + 6];
//					BLE_CMD_SPP_CONNECTED[BLE_SCAN_CONNECT_INDEX_OFFSET] = *(bleIndex + 6);
//					BLE_Mode = BLE_MODE_SCAN_FINISH;
//					expectString = BLE_CMD_SCAN_FINISH_RESPOND;
//					break;

//				case BLE_MODE_SCAN_FINISH:
//					BLE_Mode = BLE_MODE_SPP_CONNECTED;
//					break;

//				case BLE_MODE_SPP_CONNECTED_FINISH:
//					TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
//							"���ӳɹ�", 14);
//					BLE_Mode = BLE_MODE_LINK_DEVICE;
//					break;

//				default:
//					break;
//				}

////				osDelay(1000);
//			}
//		}
//#endif
//		/* �����������,����δ�������������˳�whileѭ�� */
////		if ((BLE_Mode == BLE_MODE_LINK_DEVICE) || (timeOutCnt > 10))
//		if (BLE_Mode == BLE_MODE_LINK_DEVICE)
//		{
////			if (timeOutCnt > 10)
////			{
////				TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
////					"��ӡ������ʧ��", 14);
////			}
//			break;
//		}
//	}

////	osThreadSetPriority(tftlcdTaskHandle, osPriorityNormal);

//	if (BLE_Mode == BLE_MODE_LINK_DEVICE)
//		return PRINT_MODE_BLE_LINK;
//	else
//		return PRINT_MODE_BLE_UNLINK;
//}

/*******************************************************************************
 *
 */
void BLE_SendCmd(char* str)
{
	HAL_UART_Transmit_DMA(&BLE_UART, (uint8_t*)str, strlen(str));
}

/*******************************************************************************
 *
 */
//void USART3_IRQHandler(void)
//{
//	char* bleIndex;

//	if (__HAL_UART_GET_FLAG(&BLE_UART, UART_FLAG_RXNE) == SET)
//	{
//		/* ������ձ�־λ */
//		__HAL_UART_CLEAR_FLAG(&BLE_UART, UART_FLAG_RXNE);

//		/* �������ֵ */
//		BLE_RecvBuffer.recvBuffer[BLE_RecvBuffer.bufferSize] = BLE_UART.Instance->DR;
//		/* �жϽ��� */
//		if ((BLE_RecvBuffer.recvBuffer[BLE_RecvBuffer.bufferSize] == 0x0D)
//			&& (BLE_RecvBuffer.recvBuffer[BLE_RecvBuffer.bufferSize - 1] == 0x0A))
//		{
//			switch (BLE_Mode)
//			{
//			case BLE_MODE_TEST_FINISH:
//				BLE_Mode = BLE_MODE_SCAN;
//				break;

//			case BLE_MODE_SCAN_DEVICE:
//				bleIndex = strstr((char*)BLE_RecvBuffer.recvBuffer, "+SCAN=");
//				BLE_CMD_SPP_CONNECTED[BLE_SCAN_CONNECT_INDEX_OFFSET] = *(bleIndex + 6);
//				BLE_Mode = BLE_MODE_SCAN_FINISH;
//				break;

//			case BLE_MODE_SCAN_FINISH:
//				BLE_Mode = BLE_MODE_SPP_CONNECTED;
//				break;

//			case BLE_MODE_SPP_CONNECTED_FINISH:
//				BLE_Mode = BLE_MODE_LINK_DEVICE;
//				break;

//			default:
//				break;
//			}
//		}
//	}
//}

