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
	char* expectString;						/* Ԥ���յ����ַ��� */

	GPRS_StructTypedef sendStruct;			/* ���ͽṹ */
	GPS_LocateTypedef location;

	uint8_t moduleTimeoutCnt;				/* ģ�鳬ʱ���� */
	uint8_t moduleErrorCnt;					/* ģ����մ���ָ����� */
	uint16_t curPatchPack;					/* �����ϴ����� */
	BOOL gprsInited;						/* gprs���ܳ�ʼ����־λ */

	GPRS_Init();
	/* ��ʼ�����ͽṹ�� */
	GPRS_StructInit(&sendStruct);

	while(1)
	{
		/* ���Ͳ��� */
		switch (moduleStatus)
		{
		/* ���ģ����Ч������ִ�п��� */
		case MODULE_INVALID:
			printf("ģ�鿪��\r\n");
			/* ���� */
			GPRS_PWR_CTRL_ENABLE();
			expectString = AT_CMD_POWER_ON_READY_RESPOND;
			moduleStatus = MODULE_VALID;
			break;

		/* ���ò����� */
		case SET_BAUD_RATE:
			printf("���ò�����\r\n");
			GPRS_SendCmd(AT_CMD_SET_BAUD_RATE);
			expectString = AT_CMD_SET_BAUD_RATE_RESPOND;
			moduleStatus = SET_BAUD_RATE_FINISH;

			break;

		/* ʹ��GPS���� */
		case ENABLE_GPS:
			printf("ʹ��GPS����\r\n");
			GPRS_SendCmd(AT_CMD_GPS_ENABLE);
			expectString = AT_CMD_GPS_ENABLE_RESPOND;
			moduleStatus = ENABLE_GPS_FINISH;
			break;

		/* ��ȡGNRMC��λֵ */
		case GET_GPS_GNRMC:
			/* �ȴ�GPSʹ���ź� */
			signal = osSignalWait(GPRSPROCESS_GPS_ENABLE, 2000);
			if ((signal.value.signals & GPRSPROCESS_GPS_ENABLE) == GPRSPROCESS_GPS_ENABLE)
			{
				printf("��ȡGNRMC��λֵ\r\n");
				/* GPS����ʹ�ܱȽ�������Ҫ����ʱһ��ʱ�� */
				GPRS_SendCmd(AT_CMD_GPS_GET_GNRMC);
				expectString = AT_CMD_GPS_GET_GNRMC_RESPOND;
				moduleStatus = GET_GPS_GNRMC_FINISH;
			}
			break;

		/* ��ѯSIM��״̬ */
		case CHECK_SIM_STATUS:
			/* �ж�GPRS�����Ƿ��ʼ����ɣ�����ֱ����ת����ȡ��������ַ */
			if (gprsInited == TRUE)
			{
				printf("GPRS�����ѳ�ʼ��\r\n");
				GPRS_SendCmd(AT_CMD_CHECK_STATUS);
				expectString = AT_CMD_CHECK_STATUS_RESPOND;
				moduleStatus = SET_SERVER_IP_ADDR;
			}
			/* ����ִ�г�ʼ�� */
			else
			{
				printf("��ѯsim��״̬\r\n");
				GPRS_SendCmd(AT_CMD_CHECK_SIM_STATUS);
				expectString = AT_CMD_CHECK_SIM_STATUS_RESPOND;
				moduleStatus = CHECK_SIM_STATUS_FINISH;
			}

			break;

		/* ��������״̬ */
		case SEARCH_NET_STATUS:
			printf("��������\r\n");
			GPRS_SendCmd(AT_CMD_SEARCH_NET_STATUS);
			expectString = AT_CMD_SEARCH_NET_STATUS_RESPOND;
			moduleStatus = SEARCH_NET_STATUS_FINISH;
			break;

		/* ����GPRS״̬ */
		case CHECK_GPRS_STATUS:
			printf("����GPRS״̬\r\n");
			GPRS_SendCmd(AT_CMD_CHECK_GPRS_STATUS);
			expectString = AT_CMD_CHECK_GPRS_STATUS_RESPOND;
			moduleStatus = CHECK_GPRS_STATUS_FINISH;
			break;

		/* ���õ����ӷ�ʽ */
		case SET_SINGLE_LINK:
			printf("���õ�����ʽ\r\n");
			GPRS_SendCmd(AT_CMD_SET_SINGLE_LINK);
			expectString = AT_CMD_SET_SINGLE_LINK_RESPOND;
			moduleStatus = SET_SINGLE_LINK_FINISH;
			break;

		/* ����Ϊ͸��ģʽ */
		case SET_SERIANET_MODE:
			printf("����͸��ģʽ\r\n");
			GPRS_SendCmd(AT_CMD_SET_SERIANET_MODE);
			expectString = AT_CMD_SET_SERIANET_MODE_RESPOND;
			moduleStatus = SET_SERIANET_MODE_FINISH;
			break;

		/* ����APN���� */
		case SET_APN_NAME:
			printf("����APN����\r\n");
			GPRS_SendCmd(AT_CMD_SET_APN_NAME);
			expectString = AT_CMD_SET_APN_NAME_RESPOND;
			moduleStatus = SET_APN_NAME_FINISH;
			break;

		/* ����PDP���� */
		case ACTIVE_PDP:
			printf("����PDP����\r\n");
			GPRS_SendCmd(AT_CMD_ACTIVE_PDP);
			expectString = AT_CMD_ACTIVE_PDP_RESPOND;
			moduleStatus = ACTIVE_PDP_FINISH;
			break;

		/* ��ȡ����IP��ַ */
		case GET_SELF_IP_ADDR:
			printf("��ȡ����IP��ַ\r\n");
			GPRS_SendCmd(AT_CMD_GET_SELF_IP_ADDR);
			expectString = AT_CMD_GET_SELF_IP_ADDR_RESPOND;
			moduleStatus = GET_SELF_IP_ADDR_FINISH;
			/* ���GPRS���ܳ�ʼ����� */
			gprsInited = TRUE;
			break;

		/* ���÷�������ַ */
		case SET_SERVER_IP_ADDR:
			/* �ȴ�����ʹ���ź� */
			signal = osSignalWait(GPRSPROCESS_SEND_DATA_ENABLE, 20000);
			if ((signal.value.signals & GPRSPROCESS_SEND_DATA_ENABLE) == GPRSPROCESS_SEND_DATA_ENABLE)
			{
				printf("��ȡ��������ַ\r\n");
				GPRS_SendCmd(AT_CMD_SET_SERVER_IP_ADDR);
				expectString = AT_CMD_SET_SERVER_IP_ADDR_RESPOND;
				moduleStatus = SET_SERVER_IP_ADDR_FINISH;
			}
			break;

		/* ģ��׼������ */
		case READY:
			printf("ģ��׼�����ˣ���������\r\n");
			/* ��ȡ���η��͵����� */
			signal = osMessageGet(infoCntMessageQId, 100);
			curPatchPack = signal.value.v;

			/* ��ȡģ������Ϣ */
			signal = osMessageGet(infoMessageQId, 100);
			memcpy(&sendStruct.dataPack, (uint32_t*)signal.value.v,
					curPatchPack * sizeof(FILE_InfoTypedef));
			/* �������ݵ�ƽ̨ */
			GPRS_SendProtocol(&sendStruct, curPatchPack);
			expectString = AT_CMD_DATA_SEND_SUCCESS_RESPOND;
			moduleStatus = DATA_SEND_FINISH;
			break;

		/* �˳�͸��ģʽ */
		case EXTI_SERIANET_MODE:
			printf("�˳�͸��ģʽ\r\n");
			GPRS_SendCmd(AT_CMD_EXIT_SERIANET_MODE);
			expectString = AT_CMD_EXIT_SERIANET_MODE_RESPOND;
			moduleStatus = EXTI_SERIANET_MODE_FINISH;
			break;

		/* �˳�����ģʽ */
		case EXTI_LINK_MODE:
			printf("�˳�����ģʽ\r\n");
			GPRS_SendCmd(AT_CMD_EXIT_LINK_MODE);
			expectString = AT_CMD_EXIT_LINK_MODE_RESPOND;
			moduleStatus = EXTI_LINK_MODE_FINISH;
			break;

		/* �ر��ƶ����� */
		case SHUT_MODULE:
			printf("�ر��ƶ�����\r\n");
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
			printf("GMSģ��ָ����յȴ���ʱ\r\n");
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
					printf("δ���յ�ƽ̨��ȷ����\r\n");
				}

				break;

			/* GPS�������̱Ƚ�������ʱ���Գ�ʱ�ȴ� */
//			case ENABLE_GPS_FINISH:
//				break;
//
//			case GET_GPS_GNRMC_FINISH:
//				break;

			default:
				/* ���������״̬��ǰ�ƶ�һ�� */
				moduleStatus--;

				if (moduleTimeoutCnt > 2)
				{
					moduleTimeoutCnt = 0;
					moduleStatus = GET_GPS_GNRMC;
					printf("ģ��ָ����ճ�ʱ3��,�������η���\r\n");
					osThreadSuspend(NULL);
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
			if (NULL != strstr((char*)GPRS_BufferStatus.recvBuffer, expectString))
			{
				/* ��ȷ���գ�����մ������ */
				moduleErrorCnt = 0;

				switch (moduleStatus)
				{
				/* ģ����� */
				case MODULE_VALID:
					printf("ģ�����\r\n");
					/* ������ɣ��Ͽ�power�������� */
					GPRS_PWR_CTRL_DISABLE();

					/* ģ�鿪���ʵ���ʱ */
					osDelay(5000);
					moduleStatus = SET_BAUD_RATE;
					break;

				/* ���ò�������� */
				case SET_BAUD_RATE_FINISH:
					printf("���ò��������\r\n");
					moduleStatus = ENABLE_GPS;
					break;

				/* ʹ��GPS������� */
				case ENABLE_GPS_FINISH:
					printf("ʹ��GPS�������\r\n");
					moduleStatus = GET_GPS_GNRMC;
					break;

					/* ��ȡGNRMC��λֵ��� */
				case GET_GPS_GNRMC_FINISH:
					printf("��ȡGNRMC��λֵ���\r\n");
					/* ת����λ���� */
					GPS_GetLocation(GPRS_BufferStatus.recvBuffer, &location);
					printf("��λ������%50s\r\n",GPRS_BufferStatus.recvBuffer);
					/* ���ݶ�λ��Ϣ */
					osMessagePut(infoMessageQId, (uint32_t)&location, 100);
					osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPS_CONVERT_FINISH);
					/* GPS��λ�ɹ�����ģ��״̬������GPRS״̬ */
					moduleStatus = CHECK_SIM_STATUS;
					break;

					/* ���sim��״̬��� */
				case CHECK_SIM_STATUS_FINISH:
					printf("���sim��״̬���\r\n");
					moduleStatus = SEARCH_NET_STATUS;
					break;

					/* ��������״̬��� */
				case SEARCH_NET_STATUS_FINISH:
					printf("��������״̬���\r\n");
					moduleStatus = CHECK_GPRS_STATUS;
					break;

					/* ����GPRS״̬��� */
				case CHECK_GPRS_STATUS_FINISH:
					printf("����GPRS״̬���\r\n");
					moduleStatus = SET_SINGLE_LINK;
					break;

					/* ���õ�����ʽ��� */
				case SET_SINGLE_LINK_FINISH:
					printf("���õ�����ʽ���\r\n");
					moduleStatus = SET_SERIANET_MODE;
					break;

					/* ����͸��ģʽ��� */
				case SET_SERIANET_MODE_FINISH:
					printf("����͸��ģʽ���\r\n");
					moduleStatus = SET_APN_NAME;
					break;

					/* ����APN������� */
				case SET_APN_NAME_FINISH:
					printf("����APN�������\r\n");
					moduleStatus = ACTIVE_PDP;
					break;

					/* ����PDP������� */
				case ACTIVE_PDP_FINISH:
					printf("����PDP�������\r\n");
					moduleStatus = GET_SELF_IP_ADDR;
					break;

					/* ��ȡ����IP��ַ��� */
				case GET_SELF_IP_ADDR_FINISH:
					printf("��ȡ����IP��ַ���\r\n");
					moduleStatus = SET_SERVER_IP_ADDR;
					break;

					/* ���÷�������ַ��� */
				case SET_SERVER_IP_ADDR_FINISH:
					printf("���÷�������ַ���\r\n");
					moduleStatus = READY;
					break;

					/* ���ݷ������ */
				case DATA_SEND_FINISH:
					printf("���ݷ��ͳɹ�\r\n");
					printf("����������������%50s\r\n",GPRS_BufferStatus.recvBuffer);

					/* ������ʱ������ʱ��Աȣ�ʱ��У׼ */
//					RT_TimeAdjustWithCloud(GPRS_BufferStatus.recvBuffer, etime);

					moduleStatus = EXTI_SERIANET_MODE;

					/* GPRS������� */
					osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPRS_SEND_FINISHED);
					break;

					/* �˳�͸��ģʽ��� */
				case EXTI_SERIANET_MODE_FINISH:
					printf("�˳�͸��ģʽ���\r\n");
					moduleStatus = EXTI_LINK_MODE;
					break;

					/* �˳�����ģʽ��� */
				case EXTI_LINK_MODE_FINISH:
					printf("�˳�����ģʽ���\r\n");
					moduleStatus = SHUT_MODULE;
					break;

					/* �ر��ƶ�������� */
				case SHUT_MODULE_FINISH:
					printf("�ر��ƶ��������\r\n");
					/* ģ�鷢����ɣ���״̬���ó�ʹ��GPS��λ���´�����ֱ�����ӷ�������ַ���ɷ��� */
					moduleStatus = GET_GPS_GNRMC;
					/* ���Լ����� */
					osThreadSuspend(NULL);
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
				if (moduleErrorCnt >= 5)
				{
					printf("ģ����յ�����ָ���5��\r\n");
					moduleErrorCnt = 0;
					switch (moduleStatus)
					{
					case MODULE_VALID:
						break;

					/* ���ӷ�������ַ���֡�FAIL�����ߡ�ERROR�������������Ϸ����� */
					case SET_SERVER_IP_ADDR_FINISH:
						if (NULL != strstr((char*)GPRS_BufferStatus.recvBuffer, "FAIL ERROR"))
						{
							/* �������η��� */
							moduleStatus = GET_GPS_GNRMC;

							printf("���������Ϸ��������������η���\r\n");
							/* ���Լ����� */
							osThreadSuspend(NULL);
						}
						break;

					case DATA_SEND_FINISH:
						break;

					case ENABLE_GPS:
						break;

					case GET_GPS_GNRMC:
						break;

					/*  */
					default:
						moduleStatus = GET_GPS_GNRMC;
						gprsInited = FALSE;
						printf("ģ�����ô��󣬵ȴ��´���������\r\n");
						osThreadSuspend(NULL);
						break;
					}
				}
			}
			memset(GPRS_BufferStatus.recvBuffer, 0, GPRS_BufferStatus.bufferSize);
		}
	}
}



