#include "GPRSProcess.h"
#include "gprs.h"
#include "gps.h"
#include "rt.h"
#include "MainProcess.h"

/******************************************************************************/
extern GPS_LocateTypedef  GPS_Locate;				/* ��λ��Ϣ */
extern osMessageQId gprsTaskMessageQid;
extern osThreadId mainprocessTaskHandle;
extern GPRS_SendbufferTyepdef GPRS_NewSendbuffer;
extern const char Message[];
extern uint16_t GPRS_SendPackSize;							/* GPRS���Ͱ���С */
extern char     ICCID[20];									/* ICCID */
extern char	 	IMSI[15];									/* IMSI */
extern char     IMEI[15];									/* IMEI */

/******************************************************************************/
static osEvent signal;
static GPRS_ModuleStatusEnum moduleStatus = SET_BAUD_RATE;			/* GPRSģ��״̬ */
static char* expectString;											/* Ԥ���յ����ַ��� */
static GPRS_TaskStatusEnum taskStatus = START_TASK_INVALID;
static uint8_t moduleTimeoutCnt;									/* ģ�鳬ʱ���� */
static uint8_t moduleErrorCnt;										/* ģ����մ���ָ����� */
static BOOL gprsInited = FALSE;									/* gprs���ܳ�ʼ����־λ */

/******************************************************************************/
void SendProcess(void);
void TimeoutProcess(void);
void RecvProcess(void);
void RecvErrorProcess(void);


/*******************************************************************************
 * @note ģ���ظ������޷��������������
 * 		 �����ȷ������ò�����ָ�����ʱ�����ģ��δ������ִ��������������յ�ָ���λģ�飬��������ִ��
 */
void GPRSPROCESS_Task(void)
{
	while(1)
	{
		/* ���Ͳ��� */
		SendProcess();

		/* �ȴ����� */
		signal = osSignalWait(GPRS_PROCESS_TASK_RECV_ENABLE, 3000);
		/* ���ճ�ʱ */
		if (signal.status == osEventTimeout)
		{
			TimeoutProcess();
		}
		/* ���յ����� */
		else if ((signal.value.signals & GPRS_PROCESS_TASK_RECV_ENABLE)
				== GPRS_PROCESS_TASK_RECV_ENABLE)
		{
			/* ���յ��������ݣ��򽫳�ʱ������� */
			moduleTimeoutCnt = 0;

			/* ���ݻ��߶��ŷ�����ɺ��յ�ƽ̨�Ļ��� */
			if ((moduleStatus == DATA_SEND_FINISH) || (moduleStatus == MESSAGE_SEND_FINISH))
			{
				if (GPRS_RecvBuffer.buffer.GPRS_RecvPack.head == GPRS_SERVER_PACK_HEAD)
				{
					GPRS_TimeAdjustWithCloud(&GPRS_RecvBuffer.buffer.GPRS_RecvPack);
				}


				/* ����Ƿ������ݵģ����־������� */
				if (moduleStatus == DATA_SEND_FINISH)
				{
					/* GPRS������� */
					osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPRS_SEND_FINISHED);
				}

				moduleStatus = EXTI_SERIANET_MODE;
			}
			/* Ѱ��Ԥ�ڽ��յ��ַ����Ƿ��ڽ��յ������� */
			else if (NULL != strstr((char*)GPRS_RecvBuffer.buffer.recvBuffer, expectString))
			{
				RecvProcess();
			}
			/* ģ�鷵�ص�ָ���ȷ */
			else
			{
				RecvErrorProcess();
			}
			memset(GPRS_RecvBuffer.buffer.recvBuffer, 0, GPRS_RecvBuffer.bufferSize);
		}
	}
}

/*******************************************************************************
 *
 */
void SendProcess(void)
{
	/* ���Ͳ��� */
	switch (moduleStatus)
	{
	/* ���ģ����Ч������ִ�п��� */
	case MODULE_INVALID:
		/* ���� */
		GPRS_PWR_CTRL_ENABLE();
		expectString = AT_CMD_POWER_ON_READY_RESPOND;
		moduleStatus = MODULE_VALID;
		break;

	/* ���ò����� */
	case SET_BAUD_RATE:
		GPRS_RST_CTRL_ENABLE();
		osDelay(1);
		GPRS_RST_CTRL_DISABLE();
		osDelay(1);
		GPRS_SendCmd(AT_CMD_SET_BAUD_RATE);
		expectString = AT_CMD_SET_BAUD_RATE_RESPOND;
		moduleStatus = SET_BAUD_RATE_FINISH;
		break;

	/* �رջ���ģʽ */
	case ECHO_DISABLE:
		GPRS_SendCmd(AT_CMD_ECHO_DISABLE);
		expectString = AT_CMD_ECHO_DISABLE_RESPOND;
		moduleStatus = ECHO_DISABLE_FINISH;
		break;

	/* ��ȡICCID */
	case GET_ICCID:
		GPRS_SendCmd(AT_CMD_GET_ICCID);
		expectString = AT_CMD_GET_ICCID_RESPOND;
		moduleStatus = GET_ICCID_FINISH;
		break;

	/* ��ȡIMSI */
	case GET_IMSI:
		GPRS_SendCmd(AT_CMD_GET_IMSI);
		expectString = AT_CMD_GET_IMSI_RESPOND;
		moduleStatus = GET_IMSI_FINISH;
		break;

	/* ��ȡIMEI */
	case GET_IMEI:
		GPRS_SendCmd(AT_CMD_GET_IMEI);
		expectString = AT_CMD_GET_IMEI_RESPOND;
		moduleStatus = GET_IMEI_FINISH;
		break;

	/* ʹ��GPS���� */
	case ENABLE_GPS:
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
		case START_TASK_DATA:
			/* ���ģ���Ѿ���ʼ����� */
			if (gprsInited == TRUE)
				moduleStatus = GET_SIGNAL_QUALITY;
			else
				moduleStatus = CHECK_SIM_STATUS;
			break;

		/* �������ŷ��� */
		case START_TASK_MESSAGE:
			/* ���ģ���Ѿ���ʼ����ɣ����������Ͷ��ţ�������ŷ���ʧ�ܣ��ȴ��´δ������� */
			if (gprsInited == TRUE)
				moduleStatus = SET_MESSAGE_SERVER_IP_ADDR;
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
		/* GPS����ʹ�ܱȽ�������Ҫ����ʱһ��ʱ�� */
		GPRS_SendCmd(AT_CMD_GPS_GET_GNRMC);
		expectString = AT_CMD_GPS_GET_GNRMC_RESPOND;
		moduleStatus = GET_GPS_GNRMC_FINISH;
		break;

	/* ��ѯSIM��״̬ */
	case CHECK_SIM_STATUS:
		GPRS_SendCmd(AT_CMD_CHECK_SIM_STATUS);
		expectString = AT_CMD_CHECK_SIM_STATUS_RESPOND;
		moduleStatus = CHECK_SIM_STATUS_FINISH;
		break;

	/* ��������״̬ */
	case SEARCH_NET_STATUS:
		GPRS_SendCmd(AT_CMD_SEARCH_NET_STATUS);
		expectString = AT_CMD_SEARCH_NET_STATUS_RESPOND;
		moduleStatus = SEARCH_NET_STATUS_FINISH;
		break;

	/* ����GPRS״̬ */
	case CHECK_GPRS_STATUS:
		GPRS_SendCmd(AT_CMD_CHECK_GPRS_STATUS);
		expectString = AT_CMD_CHECK_GPRS_STATUS_RESPOND;
		moduleStatus = CHECK_GPRS_STATUS_FINISH;
		break;

	/* ���õ����ӷ�ʽ */
	case SET_SINGLE_LINK:
		GPRS_SendCmd(AT_CMD_SET_SINGLE_LINK);
		expectString = AT_CMD_SET_SINGLE_LINK_RESPOND;
		moduleStatus = SET_SINGLE_LINK_FINISH;
		break;

	/* ����Ϊ͸��ģʽ */
	case SET_SERIANET_MODE:
		GPRS_SendCmd(AT_CMD_SET_SERIANET_MODE);
		expectString = AT_CMD_SET_SERIANET_MODE_RESPOND;
		moduleStatus = SET_SERIANET_MODE_FINISH;
		break;

	/* ����APN���� */
	case SET_APN_NAME:
		GPRS_SendCmd(AT_CMD_SET_APN_NAME);
		expectString = AT_CMD_SET_APN_NAME_RESPOND;
		moduleStatus = SET_APN_NAME_FINISH;
		break;

	/* ����PDP���� */
	case ACTIVE_PDP:
		GPRS_SendCmd(AT_CMD_ACTIVE_PDP);
		expectString = AT_CMD_ACTIVE_PDP_RESPOND;
		moduleStatus = ACTIVE_PDP_FINISH;
		break;

	/* ��ȡ����IP��ַ */
	case GET_SELF_IP_ADDR:
		GPRS_SendCmd(AT_CMD_GET_SELF_IP_ADDR);
		expectString = AT_CMD_GET_SELF_IP_ADDR_RESPOND;
		moduleStatus = GET_SELF_IP_ADDR_FINISH;
		break;

	/* ��ȡ�ź����� */
	case GET_SIGNAL_QUALITY:
		GPRS_SendCmd(AT_CMD_GET_SIGNAL_QUALITY);
		expectString = AT_CMD_GET_SIGNAL_QUALITY_RESPOND;
		moduleStatus = GET_SIGNAL_QUALITY_FINISH;
		break;

	/* ���÷�������ַ */
	case SET_SERVER_IP_ADDR:
		GPRS_SendCmd(AT_CMD_SET_SERVER_IP_ADDR);
		expectString = AT_CMD_SET_SERVER_IP_ADDR_RESPOND;
		moduleStatus = SET_SERVER_IP_ADDR_FINISH;
		break;

	/* ģ��׼������ */
	case READY:
		GPRS_SendData(GPRS_SendPackSize);
		moduleStatus = DATA_SEND_FINISH;
		break;

	/* ���ö��ŷ�������ַ */
	case SET_MESSAGE_SERVER_IP_ADDR:
		GPRS_SendCmd(AT_CMD_SET_MESSAGE_SERVER_IP_ADDR);
		expectString = AT_CMD_SET_MESSAGE_SERVER_IP_ADDR_RESPOND;
		moduleStatus = SET_MESSAGE_SERVER_IP_ADDR_FINISH;
		break;

	/* ����׼���� */
	case MESSAGE_READY:
		GPRS_SendMessagePack(&GPRS_NewSendbuffer, RT_RealTime, (char*)Message, 12);
		expectString = AT_CMD_MESSAGE_SEND_SUCCESS_RESPOND;
		moduleStatus = MESSAGE_SEND_FINISH;
		break;

	/* �˳�͸��ģʽ */
	case EXTI_SERIANET_MODE:
		GPRS_SendCmd(AT_CMD_EXIT_SERIANET_MODE);
		expectString = AT_CMD_EXIT_SERIANET_MODE_RESPOND;
		moduleStatus = EXTI_SERIANET_MODE_FINISH;
		break;

	/* �˳�����ģʽ */
	case EXTI_LINK_MODE:
		GPRS_SendCmd(AT_CMD_EXIT_LINK_MODE);
		expectString = AT_CMD_EXIT_LINK_MODE_RESPOND;
		moduleStatus = EXTI_LINK_MODE_FINISH;
		break;

	/* �ر��ƶ����� */
	case SHUT_MODULE:
		GPRS_SendCmd(AT_CMD_SHUT_MODELU);
		expectString = AT_CMD_SHUT_MODELU_RESPOND;
		moduleStatus = SHUT_MODULE_FINISH;
		break;

	default:
		break;
	}
}

/*******************************************************************************
 *
 */
void TimeoutProcess(void)
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

	case SET_BAUD_RATE_FINISH:
		moduleStatus = MODULE_INVALID;
		break;

	/* ���͵�ƽ̨������û���յ���,�����������ݷ��ͣ���ģʽ�л����˳�͸��ģʽ */
	case DATA_SEND_FINISH:
		if (moduleTimeoutCnt > 2)
		{
			moduleTimeoutCnt = 0;
			moduleStatus = EXTI_SERIANET_MODE;
		}
		break;

	case MESSAGE_SEND_FINISH:
		if (moduleTimeoutCnt > 2)
		{
			moduleTimeoutCnt = 0;
			moduleStatus = EXTI_SERIANET_MODE;
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
			/* �ص���λ״̬ */
			moduleStatus = SET_BAUD_RATE;
			DebugPrintf("ģ��ָ����ճ�ʱ3��,�������η���\r\n");
		}
		break;
	}
}

/*******************************************************************************
 *
 */
void RecvProcess(void)
{
	/* ��ȷ���գ�����մ������ */
	moduleErrorCnt = 0;

	switch (moduleStatus)
	{
	/* ģ����� */
	case MODULE_VALID:
		/* ������ɣ��Ͽ�power�������� */
		GPRS_PWR_CTRL_DISABLE();
		/* ģ�鿪���ʵ���ʱ */
		osDelay(5000);
		moduleStatus = SET_BAUD_RATE;
		break;

	/* ���ò�������� */
	case SET_BAUD_RATE_FINISH:
		/* ��λģ�� */
		GPRS_RST_CTRL_ENABLE();
		osDelay(50);
		GPRS_RST_CTRL_DISABLE();
		osDelay(50);
		moduleStatus = ECHO_DISABLE;
		break;

	/* �رջ���ģʽ��� */
	case ECHO_DISABLE_FINISH:
		moduleStatus = GET_ICCID;
		break;

	/* ��ȡICCID��� */
	case GET_ICCID_FINISH:
		memcpy(ICCID, &GPRS_RecvBuffer.buffer.recvBuffer[10], 20);
		moduleStatus = GET_IMSI;
		break;

	/* ��ȡIMSI��� */
	case GET_IMSI_FINISH:
		memcpy(IMSI, &GPRS_RecvBuffer.buffer.recvBuffer[2], 15);
		moduleStatus = GET_IMEI;
		break;

	/* ��ȡIMEI��� */
	case GET_IMEI_FINISH:
		memcpy(IMEI, &GPRS_RecvBuffer.buffer.recvBuffer[10], 15);
		moduleStatus = ENABLE_GPS;
		break;

	/* ʹ��GPS������� */
	case ENABLE_GPS_FINISH:
		/* ʹ��GPS���ܺ󣬿����Ѿ���ɣ��ص�Initģʽ */
		moduleStatus = INIT;
//		/* ����GPS��5s�ٻ�ȡ��λ���� */
//		osDelay(5000);
		break;

	/* ��ȡGNRMC��λֵ��� */
	case GET_GPS_GNRMC_FINISH:
		/* ת����λ���� */
		GPS_GetLocation(GPRS_RecvBuffer.buffer.recvBuffer, &GPS_Locate);
		printf("��λ������%50s\r\n",GPRS_RecvBuffer.buffer.recvBuffer);

		osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPS_CONVERT_FINISH);
		/* GPS��λ�ɹ����ص�init״̬ */
		moduleStatus = INIT;
		break;

	/* ���sim��״̬��� */
	case CHECK_SIM_STATUS_FINISH:
		moduleStatus = SEARCH_NET_STATUS;
		break;

	/* ��������״̬��� */
	case SEARCH_NET_STATUS_FINISH:
		/* ע�ᵽ�������磬����ע�ᵽ�������� */
		if ((GPRS_RecvBuffer.buffer.recvBuffer[11] == '1') ||
				(GPRS_RecvBuffer.buffer.recvBuffer[11] == '5'))
			moduleStatus = CHECK_GPRS_STATUS;
		else
			moduleStatus = INIT;
		break;

	/* ����GPRS״̬��� */
	case CHECK_GPRS_STATUS_FINISH:
		moduleStatus = SET_SINGLE_LINK;
		break;

	/* ���õ�����ʽ��� */
	case SET_SINGLE_LINK_FINISH:
		moduleStatus = SET_SERIANET_MODE;
		break;

	/* ����͸��ģʽ��� */
	case SET_SERIANET_MODE_FINISH:
		moduleStatus = SET_APN_NAME;
		break;

	/* ����APN������� */
	case SET_APN_NAME_FINISH:
		moduleStatus = ACTIVE_PDP;
		break;

	/* ����PDP������� */
	case ACTIVE_PDP_FINISH:
		moduleStatus = GET_SELF_IP_ADDR;
		break;

	/* ��ȡ����IP��ַ��� */
	case GET_SELF_IP_ADDR_FINISH:
		moduleStatus = GET_SIGNAL_QUALITY;
		/* ���GPRS���ܳ�ʼ����� */
		gprsInited = TRUE;
		break;

	/* ��ȡ�ź�������� */
	case GET_SIGNAL_QUALITY_FINISH:
		GPRS_signalQuality = GPRS_GetSignalQuality(GPRS_RecvBuffer.buffer.recvBuffer);
		printf("�ź�ǿ��=%d\r\n", GPRS_signalQuality);
		moduleStatus = SET_SERVER_IP_ADDR;
		break;

	/* ���÷�������ַ��� */
	case SET_SERVER_IP_ADDR_FINISH:
		moduleStatus = READY;
		break;

	/* ���ö��ŷ�������ַ��� */
	case SET_MESSAGE_SERVER_IP_ADDR_FINISH:
		moduleStatus = MESSAGE_READY;
		break;

	/* �˳�͸��ģʽ��� */
	case EXTI_SERIANET_MODE_FINISH:
		moduleStatus = EXTI_LINK_MODE;
		break;

	/* �˳�����ģʽ��� */
	case EXTI_LINK_MODE_FINISH:
		moduleStatus = SHUT_MODULE;
		break;

	/* �ر��ƶ�������� */
	case SHUT_MODULE_FINISH:
		/* ģ�鷢����ɣ���״̬���ó�ʹ��GPS��λ���´�����ֱ�����ӷ�������ַ���ɷ��� */
		moduleStatus = INIT;
		break;

	default:
		break;
	}
}

/*******************************************************************************
 *
 */
void RecvErrorProcess(void)
{
	/* ������� */
	moduleErrorCnt++;
	if (moduleErrorCnt >= 5)
	{
		DebugPrintf("ģ����յ�����ָ���5��\r\n");
		moduleErrorCnt = 0;
		switch (moduleStatus)
		{
		case MODULE_VALID:
			break;

		/* ���ӷ�������ַ���֡�FAIL�����ߡ�ERROR�������������Ϸ����� */
//		case SET_SERVER_IP_ADDR_FINISH:
//			if (NULL != strstr((char*)GPRS_RecvBuffer.recvBuffer, "FAIL"))
//			{
//				/* �������η��� */
//				moduleStatus = INIT;
//
//				DebugPrintf("���������Ϸ��������������η���\r\n");
//			}
//			break;

		case DATA_SEND_FINISH:
			break;

		case ENABLE_GPS:
			break;

		case GET_GPS_GNRMC:
			moduleStatus = INIT;
			break;

		default:
			moduleStatus = SET_BAUD_RATE;
			gprsInited = FALSE;
			DebugPrintf("ģ�����ô��󣬵ȴ��´���������\r\n");
			break;
		}
	}
}
