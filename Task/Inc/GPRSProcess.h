#ifndef __GPRS_PROCESS_H
#define __GPRS_PROCESS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "gprs.h"

/*******************************************************************************
 * ATָ��
 */
#define AT_CMD_RESPOND_OK				  		("OK")
/* ģ���ʼ��ָ�� */
#define AT_CMD_CHECK_STATUS				  		("AT\r\n")
#define AT_CMD_CHECK_STATUS_RESPOND		  		(AT_CMD_RESPOND_OK)
#define AT_CMD_POWER_ON_READY_RESPOND	  		("+CFUN: 1")
#define AT_CMD_MODULE_START_RESPOND	  	  		("SMS Ready")
#define AT_CMD_SET_BAUD_RATE			  		("AT+IPR=115200\r\n")			/* ���ù̶������� */
#define AT_CMD_SET_BAUD_RATE_RESPOND	  		(AT_CMD_RESPOND_OK)
#define AT_CMD_CHECK_SIM_STATUS			  		("AT+CPIN?\r\n")	  			/* ��ѯSIM��״̬ */
#define AT_CMD_CHECK_SIM_STATUS_RESPOND	  		("+CPIN: READY")
#define AT_CMD_SEARCH_NET_STATUS		  		("AT+CREG?\r\n")	  			/* ��ѯ������״̬ */
#define AT_CMD_SEARCH_NET_STATUS_RESPOND	  	("+CREG: 0")
#define AT_CMD_SEARCH_NET_LOCATION_RESPOND  	("+CREG: 0,1")					/* ע�ᵽ�������� */
#define AT_CMD_SEARCH_NET_ROAM_RESPOND  		("+CREG: 0,5")					/* ע�ᵽ�������� */
#define AT_CMD_CHECK_GPRS_STATUS		  		("AT+CGATT?\r\n")				/* ��ѯGPRS�����Ƿ�ɹ� */
#define AT_CMD_CHECK_GPRS_STATUS_RESPOND  		("+CGATT: 1")

/* ����GPRSΪ͸��ģʽ */
#define AT_CMD_SET_SINGLE_LINK			  		("AT+CIPMUX=0\r\n")	 			/* ����Ϊ��·ģʽ */
#define AT_CMD_SET_SINGLE_LINK_RESPOND	  		(AT_CMD_RESPOND_OK)
#define AT_CMD_SET_SERIANET_MODE		  		("AT+CIPMODE=1\r\n")	 		/* ����Ϊ͸��ģʽ */
#define AT_CMD_SET_SERIANET_MODE_RESPOND  		(AT_CMD_RESPOND_OK)
#define AT_CMD_SET_APN_NAME				  		("AT+CSTT=\"UNINET\"\r\n")		/* ����APN���� */
#define AT_CMD_SET_APN_NAME_RESPOND       		(AT_CMD_RESPOND_OK)
#define AT_CMD_ACTIVE_PDP				  		("AT+CIICR\r\n")		 		/* ����PDP���� */
#define AT_CMD_ACTIVE_PDP_RESPOND		  		(AT_CMD_RESPOND_OK)
#define AT_CMD_GET_SELF_IP_ADDR			  		("AT+CIFSR\r\n")		 		/* ��ȡ����IP��ַ */
#define AT_CMD_GET_SELF_IP_ADDR_RESPOND   		(AT_CMD_RESPOND_OK)
#define AT_CMD_GPS_ENABLE				  		("AT+EGPSC=1\r\n")				/* ʹ��GPS���� */
//#define AT_CMD_GPS_ENABLE_RESPOND		  		("+MGPSSTATUS:1,1,0")
#define AT_CMD_GPS_ENABLE_RESPOND		  		(AT_CMD_RESPOND_OK)
#define AT_CMD_GPS_CHECK_STATUS			  		("AT+GETGPS=\"GNRMC\"\r\n")		/* ��ȡGNRMC״̬ */
#define AT_CMD_GPS_CHECK_STATUS_RESPOND	  		(AT_CMD_RESPOND_OK)
#define AT_CMD_GPS_GET_GNRMC			  		("AT+GETGPS=\"GNRMC\"\r\n")		/* ��ȡGNRMC */
#define AT_CMD_GPS_GET_GNRMC_RESPOND	  		("$GNRMC,")
#define AT_CMD_GPS_DISABLE				  		("AT+EGPSC=0\r\n")				/* ʧ��GPS���� */
#define AT_CMD_GPS_DISABLE_RESPOND		  		(AT_CMD_RESPOND_OK)
#define AT_CMD_SET_SERVER_IP_ADDR		  		("AT+CIPSTART=\"TCP\",\"112.124.106.188\",\"8090\"\r\n")/* ���ӷ����� */
#define AT_CMD_SET_SERVER_IP_ADDR_RESPOND 		("CONNECT OK")
#define AT_CMD_DATA_SEND_SUCCESS_RESPOND  		("Save Data")
#define AT_CMD_DATA_SEND_ERROR_RESPOND    		("Error Data")
#define AT_CMD_EXIT_SERIANET_MODE	  	  		("+++")			 				/* �˳�͸������ģʽ */
#define AT_CMD_EXIT_SERIANET_MODE_RESPOND 		(AT_CMD_RESPOND_OK)
#define AT_CMD_RETURN_SERIANET		  	  		("ATO\r\n")			 			/* ����͸��ģʽ */
#define AT_CMD_RETURN_SERIANET_RESPOND	  		("ATO\r\n")
#define AT_CMD_EXIT_LINK_MODE			  		("AT+CIPCLOSE=0\r\n")	 		/* �رյ�·Socket���� */
#define AT_CMD_EXIT_LINK_MODE_RESPOND	  		("CLOSE OK")
#define AT_CMD_SHUT_MODELU	 		  	  		("AT+CIPSHUT\r\n")		 		/* �ر��ƶ����� */
#define AT_CMD_SHUT_MODELU_RESPOND		  		("SHUT OK")

/* �豸���� */
#define AT_CMD_GET_SIGNAL_QUALITY		  		("AT+CSQ\r\n")					/* ��ȡ�ź����� */
#define AT_CMD_GET_SIGNAL_QUALITY_RESPOND 		("+CSQ:")
#define AT_CMD_GET_OPERATOR				  		("AT+COPS?\r\n")				/* ��ȡ��Ӫ�� */
#define AT_CMD_GET_OPERATOR_RESPOND		  		("+COPS:")
#define AT_CMD_SET_TEXT_FORMAT_GSM				("AT+CSCS=\"GSM\"\r\n")				/* �����ַ���ΪGSM */
#define AT_CMD_SET_TEXT_FORMAT_GSM_RESPOND		(AT_CMD_RESPOND_OK)
#define AT_CMD_GET_SUBSCRIBER_NUMB		  		("AT+CNUM\r\n")					/* ��ȡ�������� */
#define AT_CMD_GET_SUBSCRIBER_NUMB_RESPOND 		("+CNUM:")
#define AT_CMD_ECHO_ENABLE				  		("ATE1\r\n")					/* �����������ģʽ */
#define AT_CMD_ECHO_ENABLE_RESPOND		  		(AT_CMD_RESPOND_OK)
#define AT_CMD_ECHO_DISABLE				  		("ATE0\r\n")					/* �ر��������ģʽ */
#define AT_CMD_ECHO_DISABLE_RESPOND		  		(AT_CMD_RESPOND_OK)

/* ���Ͷ��� */
#define AT_CMD_SET_MESSAGE_MODE					("AT+CMGF=1\r\n")
#define AT_CMD_SET_MESSAGE_MODE_RESPOND			(AT_CMD_RESPOND_OK)
#define AT_CMD_SET_TEXT_PARAM					("AT+CSMP=17,167,2,25\r\n")
#define AT_CMD_SET_TEXT_PARAM_RESPOND			(AT_CMD_RESPOND_OK)
#define AT_CMD_SET_MESSAGE_TEXT_FORMAT			("AT+CSCS=\"UCS2\"\r\n")
#define AT_CMD_SET_MESSAGE_TEXT_FORMAT_RESPOND	(AT_CMD_RESPOND_OK)
#define AT_CMD_SEND_MESSAGE_NUMB				("AT+CMGS=\"18367053909\"\r\n")
#define AT_CMD_SEND_MESSAGE_NUMB_RESPOND		(">")
#define AT_CMD_SEND_MESSAGE_SUCCESS_RESPOND		(AT_CMD_RESPOND_OK)

/* �����������Ͷ��� */
#define AT_CMD_GET_ICCID						("AT+ICCID\r\n")
#define AT_CMD_GET_ICCID_RESPOND				("ICCID:")
#define AT_CMD_GET_IMSI							("AT+CIMI\r\n")
#define AT_CMD_GET_IMSI_RESPOND					(AT_CMD_RESPOND_OK)
#define AT_CMD_GET_IMEI							("AT+EGMR=0,7\r\n")
#define AT_CMD_GET_IMEI_RESPOND					("+EGMR:")
#define AT_CMD_SET_MESSAGE_SERVER_IP_ADDR		("AT+CIPSTART=\"TCP\",\"sms.hzjly.cn\",\"10001\"\r\n")/* ���ӷ����� */
#define AT_CMD_SET_MESSAGE_SERVER_IP_ADDR_RESPOND ("CONNECT OK")
#define AT_CMD_MESSAGE_SEND_SUCCESS_RESPOND  	("18367053909")
/******************************************************************************/
#define GPRSPROCESS_WAIT_FOR_PARAM_INIT			(1 << 0)
//#define GPRSPROCESS_SEND_DATA_ENABLE			(1 << 1)
#define GPRS_PROCESS_TASK_RECV_ENABLE			(1 << 2)

/******************************************************************************/
typedef enum
{
	MODULE_INVALID,									/* ģ����Ч */
	MODULE_VALID,									/* ģ����Ч */
	SET_BAUD_RATE,									/* ���ò����� */
	SET_BAUD_RATE_FINISH,							/* ������������� */
	ECHO_DISABLE,									/* �رջ���ģʽ */
	ECHO_DISABLE_FINISH,							/* �رջ���ģʽ��� */
	GET_ICCID,										/* ��ȡICCID */
	GET_ICCID_FINISH,								/* ��ȡICCID��� */
	GET_IMSI,										/* ��ȡIMSI */
	GET_IMSI_FINISH,								/* ��ȡIMSI��� */
	GET_IMEI,										/* ��ȡIMEI */
	GET_IMEI_FINISH,								/* ��ȡIMEI��� */
	ENABLE_GPS,										/* ʹ��GPS���� */
	ENABLE_GPS_FINISH,								/* ʹ��GPS������� */
	INIT,											/* ��ʼ״̬ */
	GET_GPS_GNRMC,									/* ��ȡGNRMC��λֵ */
	GET_GPS_GNRMC_FINISH,							/* ��ȡGNRMC��λֵ��� */
	CHECK_SIM_STATUS,								/* ��ѯSIM��״̬ */
	CHECK_SIM_STATUS_FINISH,						/* ��ѯSIM��״̬��� */
	SEARCH_NET_STATUS,								/* ��������״̬ */
	SEARCH_NET_STATUS_FINISH,						/* ��������״̬��� */
//	GET_OPERATOR,									/* ��ȡ��Ӫ����Ϣ */
//	GET_OPERATOR_FINISH,							/* ��ȡ��Ӫ����Ϣ��� */
//	SET_TEXT_FORMAT_GSM,							/* �����ַ�����ʽΪGSM */
//	SET_TEXT_FORMAT_GSM_FINISH,						/* �����ַ�����ʽΪGSM��� */
//	GET_SUBSCRIBER_NUMB,							/* ��ȡ�������� */
//	GET_SUBSCRIBER_NUMB_FINISH,						/* ��ȡ����������� */
	CHECK_GPRS_STATUS,								/* ����GPRS״̬ */
	CHECK_GPRS_STATUS_FINISH,						/* ����GPRS״̬��� */
	SET_SINGLE_LINK,								/* ���õ����ӷ�ʽ */
	SET_SINGLE_LINK_FINISH,							/* ���õ����ӷ�ʽ��� */
	SET_SERIANET_MODE,								/* ����Ϊ͸��ģʽ */
	SET_SERIANET_MODE_FINISH,						/* ����Ϊ͸��ģʽ��� */
	SET_APN_NAME,									/* ����APN���� */
	SET_APN_NAME_FINISH,							/* ����APN������� */
	ACTIVE_PDP,										/* ����PDP���� */
	ACTIVE_PDP_FINISH,								/* ����PDP������� */
	GET_SELF_IP_ADDR,								/* ��ȡ����IP��ַ */
	GET_SELF_IP_ADDR_FINISH,						/* ��ȡ����IP��ַ��� */
	GET_SIGNAL_QUALITY,								/* ��ȡ�ź����� */
	GET_SIGNAL_QUALITY_FINISH,						/* ��ȡ�ź�������� */
	SET_SERVER_IP_ADDR,								/* ���÷�������ַ */
	SET_SERVER_IP_ADDR_FINISH,						/* ���÷�������ַ��� */
	READY,											/* ģ��׼���� */
	DATA_SEND_FINISH,								/* ���ݷ������ */

	SET_MESSAGE_SERVER_IP_ADDR,						/* ���ö��ŷ�������ַ */
	SET_MESSAGE_SERVER_IP_ADDR_FINISH,				/* ���ö��ŷ�������ַ��� */
	MESSAGE_READY,									/* ����׼���� */
	MESSAGE_SEND_FINISH,							/* ���ŷ������ */

	EXTI_SERIANET_MODE,								/* �˳�͸��ģʽ */
	EXTI_SERIANET_MODE_FINISH,						/* �˳�͸��ģʽ��� */
	RETURN_SERIANET_MODE,							/* ����͸��ģʽ */
	RETURN_SERIANET_MODE_FINISH,					/* ����͸��ģʽ��� */
	EXTI_LINK_MODE,									/* �˳�����ģʽ */
	EXTI_LINK_MODE_FINISH,							/* �˳�����ģʽ��� */
	SHUT_MODULE,									/* �ر��ƶ����� */
	SHUT_MODULE_FINISH,								/* �ر��ƶ�������� */
} GPRS_ModuleStatusEnum;

typedef enum
{
	START_TASK_INVALID,								/* ��Ч״̬ */
	START_TASK_GPS,									/* ��ȡGPS��λ���� */
	START_TASK_DATA,								/* �������� */
	START_TASK_MESSAGE,								/* ���Ͷ��� */
} GPRS_TaskStatusEnum;

/******************************************************************************/
void GPRSPROCESS_Task(void);


#endif
