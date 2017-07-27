#include "GPRSProcess.h"
#include "gprs.h"
#include "gps.h"

#include "osConfig.h"
#include "MainProcess.h"

/*******************************************************************************
 *
 */
void GPRSPROCESS_Task(void)
{
	osEvent signal;
	GPRS_ModuleStatusEnum moduleStatus = SET_BAUD_RATE;		/* GPRSģ��״̬ */
	char* expectString;						/* Ԥ���յ����ַ��� */
	char* str;
	GPRS_StructTypedef sendStruct;			/* ���ͽṹ */
	GPS_LocationTypedef location;
	BOOL isLoop = FALSE;

	GPRS_Init();
	/* ��ʼ�����ͽṹ�� */
	GPRS_StructInit(&sendStruct);

	while(1)
	{
		/* ��ȡģ������Ϣ */
		signal = osMessageGet(infoMessageQId, 100);
		memcpy(&sendStruct.dataPack, (uint32_t*)signal.value.v, sizeof(exFLASH_InfoTypedef));
		isLoop = TRUE;

		while(isLoop)
		{
			/* ���Ͳ��� */
			switch (moduleStatus)
			{
			/* ���ģ����Ч������ִ�п��� */
			case MODULE_INVALID:
				printf("ģ�鿪��\r\n");
				/* ���� */
				GPRS_PWR_CTRL_ENABLE();
				osDelay(5000);
				GPRS_PWR_CTRL_DISABLE();
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

			/* ��ѯSIM��״̬ */
			case CHECK_SIM_STATUS:
				printf("��ѯsim��״̬\r\n");
				GPRS_SendCmd(AT_CMD_CHECK_SIM_STATUS);
				expectString = AT_CMD_CHECK_SIM_STATUS_RESPOND;
				moduleStatus = CHECK_SIM_STATUS_FINISH;
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
				printf("��ȡGNRMC��λֵ\r\n");
				/* GPS����ʹ�ܱȽ�������Ҫ����ʱһ��ʱ�� */
				GPRS_SendCmd(AT_CMD_GPS_GET_GNRMC);
				expectString = AT_CMD_GPS_GET_GNRMC_RESPOND;
				moduleStatus = GET_GPS_GNRMC_FINISH;
				break;

			/* ʧ��GPS���� */
			case DISABLE_GPS:
				printf("ʧ��GPS����\r\n");
				GPRS_SendCmd(AT_CMD_GPS_DISABLE);
				expectString = AT_CMD_GPS_DISABLE_RESPOND;
				moduleStatus = DISABLE_GPS_FINISH;
				break;

			/* ���÷�������ַ */
			case SET_SERVER_IP_ADDR:
				printf("��ȡ��������ַ\r\n");
				GPRS_SendCmd(AT_CMD_SET_SERVER_IP_ADDR);
				expectString = AT_CMD_SET_SERVER_IP_ADDR_RESPOND;
				moduleStatus = SET_SERVER_IP_ADDR_FINISH;
				break;

			/* ģ��׼������ */
			case READY:
				printf("ģ��׼�����ˣ���������\r\n");
				/* �������ݵ�ƽ̨ */
				GPRS_SendProtocol(&sendStruct);
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

			signal = osSignalWait(GPRS_PROCESS_TASK_RECV_ENABLE, 10000);
			/* ���ͳ�ʱ */
			if (signal.status == osEventTimeout)
			{
				printf("�ȴ����ճ�ʱ\r\n");
				/* ���͵�ƽ̨������û���յ��� */
				if (DATA_SEND_FINISH == moduleStatus)
				{
					/* �����������ݷ��ͣ���ģʽ�л����˳�͸��ģʽ */
					moduleStatus = EXTI_SERIANET_MODE;

					/* ���ݷ���ʧ�ܣ���¼���ȴ��´η��� */
					/* todo */
				}
				/* ��������û���յ�Ԥ�ڴ𸴣����ظ����� */
				else
				{
					/* ģʽ�л���ǰһ���ٴδ������� */
					moduleStatus--;
				}
			}
			else if ((signal.value.signals & GPRS_PROCESS_TASK_RECV_ENABLE)
					== GPRS_PROCESS_TASK_RECV_ENABLE)
			{
				/* Ѱ��Ԥ�ڽ��յ��ַ����Ƿ��ڽ��յ������� */
				str = strstr((char*)GPRS_BufferStatus.recvBuffer, expectString);
				if (NULL != str)
				{
					switch (moduleStatus)
					{
					/* ģ����� */
					case MODULE_VALID:
						printf("ģ�����\r\n");
						/* ������ɣ��Ͽ�power�������� */
						GPRS_PWR_CTRL_DISABLE();
						expectString = AT_CMD_MODULE_START_RESPOND;
						moduleStatus = MODULE_START;
						break;

						/* ģ������ */
					case MODULE_START:
						printf("ģ������\r\n");
						/* ��ʼ����ģ����� */
						moduleStatus = SET_BAUD_RATE;
						break;

						/* ���ò�������� */
					case SET_BAUD_RATE_FINISH:
						printf("���ò��������\r\n");
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
						GPS_GetLocation(GPRS_BufferStatus.recvBuffer, &location);
						/* ��ָ�봫�ݻ�ԭ���� */
						/* todo */
						moduleStatus = DISABLE_GPS;
						
						break;

						/* ʧ��GPS������� */
					case DISABLE_GPS_FINISH:
						printf("ʧ��GPS�������\r\n");
						moduleStatus = SET_SERVER_IP_ADDR;

						isLoop = FALSE;
						/* ���ݶ�λ��Ϣ */
						osMessagePut(infoMessageQId, (uint32_t)&location, 100);
						osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPS_CONVERT_FINISH);
						/* GPS��λ�ɹ������Լ����� */
						osThreadSuspend(NULL);
						break;

						/* ���÷�������ַ��� */
					case SET_SERVER_IP_ADDR_FINISH:
						printf("���÷�������ַ���\r\n");
						moduleStatus = READY;
						break;

						/* ���ݷ������ */
					case DATA_SEND_FINISH:
						printf("���ݷ��ͳɹ�\r\n");
						moduleStatus = EXTI_SERIANET_MODE;
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
						moduleStatus = ENABLE_GPS;

						/* GPRS������� */
						osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPRS_SEND_FINISHED);
						printf("���ݷ������\r\n");
						isLoop = FALSE;
						/* ���Լ����� */
						osThreadSuspend(NULL);
						break;

					default:
						break;
					}
				}
				/* ���ǽ������ݣ����Ҳ��ǻ�ȡGPS��GNRMC */
				else if (moduleStatus != DATA_SEND_FINISH)
				{
					/* �жϽ��յ������Ƿ���� */
					str = strstr((char*)GPRS_BufferStatus.recvBuffer, "Error");

					/* ���ݴ��󣬱������³�ʼ��ģ�� */
					if (str != NULL)
					{
						moduleStatus = SET_BAUD_RATE;
						printf("���ݷ��ʹ���\r\n");
					}
				}
				memset(GPRS_BufferStatus.recvBuffer, 0, GPRS_BufferStatus.bufferSize);
				osDelay(1000);
			}
		}
	}
}



