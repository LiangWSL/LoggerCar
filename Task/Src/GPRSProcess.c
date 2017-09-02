#include "GPRSProcess.h"
#include "gprs.h"
#include "gps.h"
#include "rt.h"

#include "osConfig.h"
#include "MainProcess.h"

/*******************************************************************************
 *
 */
void GPRSPROCESS_Task(void)
{
	osEvent signal;
	GPRS_ModuleStatusEnum moduleStatus = MODULE_INVALID;		/* GPRSģ��״̬ */
	char* expectString;											/* Ԥ���յ����ַ��� */

	GPS_LocateTypedef  location;
	RT_TimeTypedef*    eTime;

	GPRS_TaskStatusEnum taskStatus;
	uint8_t moduleTimeoutCnt;									/* ģ�鳬ʱ���� */
	uint8_t moduleErrorCnt;										/* ģ����մ���ָ����� */
	uint16_t curPatchPack;										/* �����ϴ����� */
	BOOL gprsInited;											/* gprs���ܳ�ʼ����־λ */

	GPRS_Init(&GPRS_SendBuffer);

	while(1)
	{
		/* ���Ͳ��� */
		switch (moduleStatus)
		{
		/* ���ģ����Ч������ִ�п��� */
		case MODULE_INVALID:
			DebugPrintf("ģ�鿪��\r\n");
			/* ���� */
			GPRS_PWR_CTRL_ENABLE();
			expectString = AT_CMD_POWER_ON_READY_RESPOND;
			moduleStatus = MODULE_VALID;
			break;

		/* ���ò����� */
		case SET_BAUD_RATE:
			DebugPrintf("���ò�����\r\n");
			GPRS_SendCmd(AT_CMD_SET_BAUD_RATE);
			expectString = AT_CMD_SET_BAUD_RATE_RESPOND;
			moduleStatus = SET_BAUD_RATE_FINISH;
			break;

		/* �رջ���ģʽ */
		case ECHO_DISABLE:
			DebugPrintf("�رջ���ģʽ\r\n");
			GPRS_SendCmd(AT_CMD_ECHO_DISABLE);
			expectString = AT_CMD_ECHO_DISABLE_RESPOND;
			moduleStatus = ECHO_DISABLE_FINISH;
			break;

		/* ʹ��GPS���� */
		case ENABLE_GPS:
			DebugPrintf("ʹ��GPS����\r\n");
			GPRS_SendCmd(AT_CMD_GPS_ENABLE);
			expectString = AT_CMD_GPS_ENABLE_RESPOND;
			moduleStatus = ENABLE_GPS_FINISH;
			break;

		/* ��ʼ״̬ */
		case INIT:
			signal = osMessageGet(gprsTaskMessageQid, osWaitForever);
			taskStatus = (GPRS_TaskStatusEnum)signal.value.v;
			switch (taskStatus)
			{
			/* ������λ */
			case START_TASK_GPS:
				moduleStatus = GET_GPS_GNRMC;
				break;

			/* ����GPRS�������� */
			case START_TASK_GPRS:
				/* ��ȡ���η��͵����� */
				signal = osMessageGet(infoCntMessageQId, 2000);
				curPatchPack = signal.value.v;

				/* ��ȡ��ǰʱ������У׼ */
				signal = osMessageGet(adjustTimeMessageQId, 2000);
				eTime = (RT_TimeTypedef*)signal.value.v;

				/* ���ģ���Ѿ���ʼ����� */
				if (gprsInited == TRUE)
					moduleStatus = GET_SIGNAL_QUALITY;
				else
					moduleStatus = CHECK_SIM_STATUS;
				break;

			/* �������ŷ��� */
			case START_TASK_GSM:
				break;

			default:
				break;
			}

			/* ��ȡ״̬���ȷ�һ��AT���� */
			GPRS_SendCmd(AT_CMD_CHECK_STATUS);
			expectString = AT_CMD_CHECK_STATUS_RESPOND;
			break;

		/* ��ȡGNRMC��λֵ */
		case GET_GPS_GNRMC:
			DebugPrintf("��ȡGNRMC��λֵ\r\n");
			/* GPS����ʹ�ܱȽ�������Ҫ����ʱһ��ʱ�� */
			GPRS_SendCmd(AT_CMD_GPS_GET_GNRMC);
			expectString = AT_CMD_GPS_GET_GNRMC_RESPOND;
			moduleStatus = GET_GPS_GNRMC_FINISH;
			break;

		/* ��ѯSIM��״̬ */
		case CHECK_SIM_STATUS:
			DebugPrintf("��ѯsim��״̬\r\n");
			GPRS_SendCmd(AT_CMD_CHECK_SIM_STATUS);
			expectString = AT_CMD_CHECK_SIM_STATUS_RESPOND;
			moduleStatus = CHECK_SIM_STATUS_FINISH;
			break;

		/* ��������״̬ */
		case SEARCH_NET_STATUS:
			DebugPrintf("��������\r\n");
			GPRS_SendCmd(AT_CMD_SEARCH_NET_STATUS);
			expectString = AT_CMD_SEARCH_NET_STATUS_RESPOND;
			moduleStatus = SEARCH_NET_STATUS_FINISH;
			break;

		/* ��ȡ��Ӫ����Ϣ */
		case GET_OPERATOR:
			DebugPrintf("��ȡ��Ӫ����Ϣ\r\n");
			GPRS_SendCmd(AT_CMD_GET_OPERATOR);
			expectString = AT_CMD_GET_OPERATOR_RESPOND;
			moduleStatus = GET_OPERATOR_FINISH;
			break;

		/* �����ַ�����ʽΪGSM */
		case SET_TEXT_FORMAT_GSM:
			DebugPrintf("�����ַ�����ʽΪGSM\r\n");
			GPRS_SendCmd(AT_CMD_SET_TEXT_FORMAT_GSM);
			expectString = AT_CMD_SET_TEXT_FORMAT_GSM_RESPOND;
			moduleStatus = SET_TEXT_FORMAT_GSM_FINISH;
			break;

		/* ��ȡ�������� */
		case GET_SUBSCRIBER_NUMB:
			DebugPrintf("��ȡ��������\r\n");
			GPRS_SendCmd(AT_CMD_GET_SUBSCRIBER_NUMB);
			expectString = AT_CMD_GET_SUBSCRIBER_NUMB_RESPOND;
			moduleStatus = GET_SUBSCRIBER_NUMB_FINISH;
			break;

		/* ����GPRS״̬ */
		case CHECK_GPRS_STATUS:
			DebugPrintf("����GPRS״̬\r\n");
			GPRS_SendCmd(AT_CMD_CHECK_GPRS_STATUS);
			expectString = AT_CMD_CHECK_GPRS_STATUS_RESPOND;
			moduleStatus = CHECK_GPRS_STATUS_FINISH;
			break;

		/* ���õ����ӷ�ʽ */
		case SET_SINGLE_LINK:
			DebugPrintf("���õ�����ʽ\r\n");
			GPRS_SendCmd(AT_CMD_SET_SINGLE_LINK);
			expectString = AT_CMD_SET_SINGLE_LINK_RESPOND;
			moduleStatus = SET_SINGLE_LINK_FINISH;
			break;

		/* ����Ϊ͸��ģʽ */
		case SET_SERIANET_MODE:
			DebugPrintf("����͸��ģʽ\r\n");
			GPRS_SendCmd(AT_CMD_SET_SERIANET_MODE);
			expectString = AT_CMD_SET_SERIANET_MODE_RESPOND;
			moduleStatus = SET_SERIANET_MODE_FINISH;
			break;

		/* ����APN���� */
		case SET_APN_NAME:
			DebugPrintf("����APN����\r\n");
			GPRS_SendCmd(AT_CMD_SET_APN_NAME);
			expectString = AT_CMD_SET_APN_NAME_RESPOND;
			moduleStatus = SET_APN_NAME_FINISH;
			break;

		/* ����PDP���� */
		case ACTIVE_PDP:
			DebugPrintf("����PDP����\r\n");
			GPRS_SendCmd(AT_CMD_ACTIVE_PDP);
			expectString = AT_CMD_ACTIVE_PDP_RESPOND;
			moduleStatus = ACTIVE_PDP_FINISH;
			break;

		/* ��ȡ����IP��ַ */
		case GET_SELF_IP_ADDR:
			DebugPrintf("��ȡ����IP��ַ\r\n");
			GPRS_SendCmd(AT_CMD_GET_SELF_IP_ADDR);
			expectString = AT_CMD_GET_SELF_IP_ADDR_RESPOND;
			moduleStatus = GET_SELF_IP_ADDR_FINISH;
			break;

		/* ��ȡ�ź����� */
		case GET_SIGNAL_QUALITY:
			DebugPrintf("��ȡ�ź�����\r\n");
			GPRS_SendCmd(AT_CMD_GET_SIGNAL_QUALITY);
			expectString = AT_CMD_GET_SIGNAL_QUALITY_RESPOND;
			moduleStatus = GET_SIGNAL_QUALITY_FINISH;
			break;

		/* ���÷�������ַ */
		case SET_SERVER_IP_ADDR:
			DebugPrintf("��ȡ��������ַ\r\n");
			GPRS_SendCmd(AT_CMD_SET_SERVER_IP_ADDR);
			expectString = AT_CMD_SET_SERVER_IP_ADDR_RESPOND;
			moduleStatus = SET_SERVER_IP_ADDR_FINISH;
			break;

		/* ģ��׼������ */
		case READY:
			DebugPrintf("ģ��׼�����ˣ���������\r\n");
			/* �������ݵ�ƽ̨ */
			GPRS_SendProtocol(&GPRS_SendBuffer, curPatchPack);
			expectString = AT_CMD_DATA_SEND_SUCCESS_RESPOND;
			moduleStatus = DATA_SEND_FINISH;
			break;

		/* �˳�͸��ģʽ */
		case EXTI_SERIANET_MODE:
			DebugPrintf("�˳�͸��ģʽ\r\n");
			GPRS_SendCmd(AT_CMD_EXIT_SERIANET_MODE);
			expectString = AT_CMD_EXIT_SERIANET_MODE_RESPOND;
			moduleStatus = EXTI_SERIANET_MODE_FINISH;
			break;

		/* �˳�����ģʽ */
		case EXTI_LINK_MODE:
			DebugPrintf("�˳�����ģʽ\r\n");
			GPRS_SendCmd(AT_CMD_EXIT_LINK_MODE);
			expectString = AT_CMD_EXIT_LINK_MODE_RESPOND;
			moduleStatus = EXTI_LINK_MODE_FINISH;
			break;

		/* �ر��ƶ����� */
		case SHUT_MODULE:
			DebugPrintf("�ر��ƶ�����\r\n");
			GPRS_SendCmd(AT_CMD_SHUT_MODELU);
			expectString = AT_CMD_SHUT_MODELU_RESPOND;
			moduleStatus = SHUT_MODULE_FINISH;
			break;

		default:
			break;
		}

		signal = osSignalWait(GPRS_PROCESS_TASK_RECV_ENABLE, 3000);
		/* ���ͳ�ʱ */
		if (signal.status == osEventTimeout)
		{
			DebugPrintf("GMSģ��ָ����յȴ���ʱ\r\n");
			/* ģ�鳬ʱ����,�������2�Σ��������η��ͣ��������� */
			moduleTimeoutCnt++;
			switch (moduleStatus)
			{
			/* ������Ϊ���Ź������أ����µ�Ƭ��������Ҳ��Ҫ����ģ�� */
			case MODULE_VALID:
				if (moduleTimeoutCnt > 1)
				{
					moduleTimeoutCnt = 0;
					/* �Ͽ���Դ���ƽţ����¿���ģ�� */
					GPRS_PWR_CTRL_DISABLE();
					osDelay(50);
					moduleStatus = MODULE_INVALID;
				}
				break;

			/* ���͵�ƽ̨������û���յ���,�����������ݷ��ͣ���ģʽ�л����˳�͸��ģʽ */
			case DATA_SEND_FINISH:
				if (moduleTimeoutCnt > 2)
				{
					moduleTimeoutCnt = 0;
					moduleStatus = EXTI_SERIANET_MODE;
					osMessageGet(realtimeMessageQId, 1);
					DebugPrintf("δ���յ�ƽ̨��ȷ����\r\n");
				}

				break;

			/* GPS�������̱Ƚ�������ʱ���Գ�ʱ�ȴ� */
			case ENABLE_GPS_FINISH:
				if (moduleTimeoutCnt > 3)
				{
					moduleTimeoutCnt = 0;
					moduleStatus = INIT;
				}
				break;

			case GET_GPS_GNRMC_FINISH:
				if (moduleTimeoutCnt > 3)
				{
					moduleTimeoutCnt = 0;
					moduleStatus = INIT;
				}
				break;

			default:
				/* ���������״̬��ǰ�ƶ�һ�� */
				moduleStatus--;

				if (moduleTimeoutCnt > 2)
				{
					moduleTimeoutCnt = 0;
					moduleStatus = INIT;
					DebugPrintf("ģ��ָ����ճ�ʱ3��,�������η���\r\n");
				}
				break;
			}
		}
		else if ((signal.value.signals & GPRS_PROCESS_TASK_RECV_ENABLE)
				== GPRS_PROCESS_TASK_RECV_ENABLE)
		{
			/* ���յ��������ݣ��򽫳�ʱ������� */
			moduleTimeoutCnt = 0;

			/* Ѱ��Ԥ�ڽ��յ��ַ����Ƿ��ڽ��յ������� */
			if (NULL != strstr((char*)GPRS_RecvBuffer.recvBuffer, expectString))
			{
				/* ��ȷ���գ�����մ������ */
				moduleErrorCnt = 0;

				switch (moduleStatus)
				{
				/* ģ����� */
				case MODULE_VALID:
					DebugPrintf("ģ�����\r\n");
					/* ������ɣ��Ͽ�power�������� */
					GPRS_PWR_CTRL_DISABLE();

					/* ģ�鿪���ʵ���ʱ */
					osDelay(5000);
					moduleStatus = SET_BAUD_RATE;
					break;

				/* ���ò�������� */
				case SET_BAUD_RATE_FINISH:
					DebugPrintf("���ò��������\r\n");
					moduleStatus = ECHO_DISABLE;
					break;

				/* �رջ���ģʽ��� */
				case ECHO_DISABLE_FINISH:
					DebugPrintf("�رջ���ģʽ���\r\n");
					moduleStatus = ENABLE_GPS;
					break;

				/* ʹ��GPS������� */
				case ENABLE_GPS_FINISH:
					DebugPrintf("ʹ��GPS�������\r\n");
					/* ʹ��GPS���ܺ󣬿����Ѿ���ɣ��ص�Initģʽ */
					moduleStatus = INIT;
					/* ����GPS��5s�ٻ�ȡ��λ���� */
					osDelay(5000);
					break;

				/* ��ȡGNRMC��λֵ��� */
				case GET_GPS_GNRMC_FINISH:
					DebugPrintf("��ȡGNRMC��λֵ���\r\n");
					/* ת����λ���� */
					GPS_GetLocation(GPRS_RecvBuffer.recvBuffer, &location);
					printf("��λ������%50s\r\n",GPRS_RecvBuffer.recvBuffer);
					/* ���ݶ�λ��Ϣ */
					osMessagePut(infoMessageQId, (uint32_t)&location, 100);
					osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPS_CONVERT_FINISH);
					/* GPS��λ�ɹ����ص�init״̬ */
					moduleStatus = INIT;
					break;

				/* ���sim��״̬��� */
				case CHECK_SIM_STATUS_FINISH:
					DebugPrintf("���sim��״̬���\r\n");
					moduleStatus = SEARCH_NET_STATUS;
					break;

				/* ��������״̬��� */
				case SEARCH_NET_STATUS_FINISH:
					DebugPrintf("��������״̬���\r\n");
					moduleStatus = GET_OPERATOR;
					break;

				/* ��ȡ��Ӫ����Ϣ��� */
				case GET_OPERATOR_FINISH:
					DebugPrintf("��ȡ��Ӫ����Ϣ���\r\n");
//					moduleStatus = SET_TEXT_FORMAT_GSM;
					moduleStatus = CHECK_GPRS_STATUS;
					break;

				/* �����ַ�����ʽΪGSM��� */
				case SET_TEXT_FORMAT_GSM_FINISH:
					DebugPrintf("�����ַ�����ʽΪGSM���\r\n");
					moduleStatus = GET_SUBSCRIBER_NUMB;
					break;

				/* ��ȡ����������� */
				case GET_SUBSCRIBER_NUMB_FINISH:
					DebugPrintf("��ȡ�����������\r\n");
					moduleStatus = CHECK_GPRS_STATUS;
					break;

				/* ����GPRS״̬��� */
				case CHECK_GPRS_STATUS_FINISH:
					DebugPrintf("����GPRS״̬���\r\n");
					moduleStatus = SET_SINGLE_LINK;
					break;

				/* ���õ�����ʽ��� */
				case SET_SINGLE_LINK_FINISH:
					DebugPrintf("���õ�����ʽ���\r\n");
					moduleStatus = SET_SERIANET_MODE;
					break;

				/* ����͸��ģʽ��� */
				case SET_SERIANET_MODE_FINISH:
					DebugPrintf("����͸��ģʽ���\r\n");
					moduleStatus = SET_APN_NAME;
					break;

				/* ����APN������� */
				case SET_APN_NAME_FINISH:
					DebugPrintf("����APN�������\r\n");
					moduleStatus = ACTIVE_PDP;
					break;

				/* ����PDP������� */
				case ACTIVE_PDP_FINISH:
					DebugPrintf("����PDP�������\r\n");
					moduleStatus = GET_SELF_IP_ADDR;
					break;

				/* ��ȡ����IP��ַ��� */
				case GET_SELF_IP_ADDR_FINISH:
					DebugPrintf("��ȡ����IP��ַ���\r\n");
					moduleStatus = GET_SIGNAL_QUALITY;
					/* ���GPRS���ܳ�ʼ����� */
					gprsInited = TRUE;
					break;

				/* ��ȡ�ź�������� */
				case GET_SIGNAL_QUALITY_FINISH:
					DebugPrintf("��ȡ�ź��������\r\n");
					GPRS_signalQuality = GPRS_GetSignalQuality(GPRS_RecvBuffer.recvBuffer);
					printf("�ź�ǿ��=%d\r\n", GPRS_signalQuality);
					moduleStatus = SET_SERVER_IP_ADDR;
					break;

				/* ���÷�������ַ��� */
				case SET_SERVER_IP_ADDR_FINISH:
					DebugPrintf("���÷�������ַ���\r\n");
					moduleStatus = READY;
					break;

				/* ���ݷ������ */
				case DATA_SEND_FINISH:
					DebugPrintf("���ݷ��ͳɹ�\r\n");
					printf("����������������%50s\r\n",GPRS_RecvBuffer.recvBuffer);

					/* ������ʱ������ʱ��Աȣ�ʱ��У׼ */
					RT_TimeAdjustWithCloud(GPRS_RecvBuffer.recvBuffer, eTime);

					moduleStatus = EXTI_SERIANET_MODE;

					/* GPRS������� */
					osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPRS_SEND_FINISHED);
					break;

				/* �˳�͸��ģʽ��� */
				case EXTI_SERIANET_MODE_FINISH:
					DebugPrintf("�˳�͸��ģʽ���\r\n");
					moduleStatus = EXTI_LINK_MODE;
					break;

				/* �˳�����ģʽ��� */
				case EXTI_LINK_MODE_FINISH:
					DebugPrintf("�˳�����ģʽ���\r\n");
					moduleStatus = SHUT_MODULE;
					break;

				/* �ر��ƶ�������� */
				case SHUT_MODULE_FINISH:
					DebugPrintf("�ر��ƶ��������\r\n");
					/* ģ�鷢����ɣ���״̬���ó�ʹ��GPS��λ���´�����ֱ�����ӷ�������ַ���ɷ��� */
					moduleStatus = INIT;
					break;

				default:
					break;
				}
			}
			/* ģ�鷵�ص�ָ���ȷ */
			else
			{
				/* ������� */
				moduleErrorCnt++;
				if (moduleErrorCnt >= 10)
				{
					DebugPrintf("ģ����յ�����ָ���10��\r\n");
					moduleErrorCnt = 0;
					switch (moduleStatus)
					{
					case MODULE_VALID:
						break;

					/* ���ӷ�������ַ���֡�FAIL�����ߡ�ERROR�������������Ϸ����� */
					case SET_SERVER_IP_ADDR_FINISH:
						if (NULL != strstr((char*)GPRS_RecvBuffer.recvBuffer, "FAIL ERROR"))
						{
							/* �������η��� */
							moduleStatus = INIT;

							DebugPrintf("���������Ϸ��������������η���\r\n");
						}
						break;

					case DATA_SEND_FINISH:
						break;

					case ENABLE_GPS:
						break;

					case GET_GPS_GNRMC:
						moduleStatus = INIT;
						break;

					default:
						moduleStatus = INIT;
						gprsInited = FALSE;
						DebugPrintf("ģ�����ô��󣬵ȴ��´���������\r\n");
						break;
					}
				}
			}
			memset(GPRS_RecvBuffer.recvBuffer, 0, GPRS_RecvBuffer.bufferSize);
		}
	}
}

