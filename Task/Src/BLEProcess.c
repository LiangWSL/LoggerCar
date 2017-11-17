#include "BLEProcess.h"
#include "tftlcd.h"
#include "ble.h"

/******************************************************************************/
extern BLE_RecvBufferTypedef BLE_RecvBuffer;

/*******************************************************************************
 *
 */
void BLEPROCESS_TASK(void)
{
	volatile BLE_ModeEnum BLE_Mode = BLE_MODE_TEST;
	char* expectString;							/* Ԥ���յ����ַ��� */
	osEvent signal;
	char BLE_CMD_SPP_CONNECTED[14] = "AT+SPPCONN= \r\n";
	uint8_t timeOutCnt = 0;						/* ��ʱ���� */
	char* bleIndex;

	while (1)
	{
		switch (BLE_Mode)
		{
		/* ���Ͳ���ָ�� */
		case BLE_MODE_TEST:
			BLE_SendCmd(BLE_CMD_AT_TEST);
			expectString = BLE_CMD_AT_TEST_RESPOND;
			BLE_Mode = BLE_MODE_TEST_FINISH;
			break;

		/* ����ɨ�� */
		case BLE_MODE_SCAN:
			BLE_SendCmd(BLE_CMD_SCAN_DEVICE);
			expectString = BLE_CMD_SCAN_DEVICE_RESPOND;
			BLE_Mode = BLE_MODE_SCAN_DEVICE;
//			TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
//										"����������ӡ��", 14);
			break;

		/* SPP���� */
		case BLE_MODE_SPP_CONNECTED:
			BLE_SendCmd(BLE_CMD_SPP_CONNECTED);
			expectString = BLE_CMD_SPP_CONNECTED_RESPOND;
			BLE_Mode = BLE_MODE_SPP_CONNECTED_FINISH;
			break;

		default:
			break;
		}

		signal = osSignalWait(TFTLCD_TASK_BLE_RECV_ENABLE, 1000);
		if (signal.status == osEventTimeout)
		{
			timeOutCnt++;
			if (timeOutCnt > 10)
			{
				TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
					"��ӡ������ʧ��", 14);
			}
		}
		else if ((signal.value.signals & TFTLCD_TASK_BLE_RECV_ENABLE)
						== TFTLCD_TASK_BLE_RECV_ENABLE)
		{
			timeOutCnt = 0;

			if (NULL != strstr((char*)BLE_RecvBuffer.recvBuffer, expectString))
			{
				switch (BLE_Mode)
				{
				case BLE_MODE_TEST_FINISH:
					TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
							"����������ӡ��", 14);
					BLE_Mode = BLE_MODE_SCAN;
					break;

				case BLE_MODE_SCAN_DEVICE:
					TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
							"�ҵ���ӡ��", 10);
					bleIndex = strstr((char*)BLE_RecvBuffer.recvBuffer, "+SCAN=");
//					BLE_CMD_SPP_CONNECTED[BLE_SCAN_CONNECT_INDEX_OFFSET]
//						 = BLE_RecvBuffer.recvBuffer[bleIndex + 6];
					BLE_CMD_SPP_CONNECTED[BLE_SCAN_CONNECT_INDEX_OFFSET] = *(bleIndex + 6);
					BLE_Mode = BLE_MODE_SCAN_FINISH;
					expectString = BLE_CMD_SCAN_FINISH_RESPOND;
					break;

				case BLE_MODE_SCAN_FINISH:
					BLE_Mode = BLE_MODE_SPP_CONNECTED;
					break;

				case BLE_MODE_SPP_CONNECTED_FINISH:
					TFTLCD_TextValueUpdate(SCREEN_ID_PRINT, CTL_ID_PRINT_TEXT_BLE_STATUS,
							"���ӳɹ�", 14);
					BLE_Mode = BLE_MODE_LINK_DEVICE;

					osThreadSuspend(NULL);
					break;

				default:
					break;
				}
			}
		}
	}
}



