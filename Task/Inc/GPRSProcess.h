#ifndef __GPRS_PROCESS_H
#define __GPRS_PROCESS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "gprs.h"

/*******************************************************************************
 * ATָ��
 */
/* ģ���ʼ��ָ�� */
#define AT_CMD_CHECK_STATUS				  ("AT\r\n")
#define AT_CMD_CHECK_STATUS_RESPOND		  (AT_CMD_RESPOND_OK)
#define AT_CMD_POWER_ON_READY_RESPOND	  ("+CFUN: 1")
#define AT_CMD_MODULE_START_RESPOND	  	  ("SMS Ready")
#define AT_CMD_SET_BAUD_RATE			  ("AT+IPR=115200\r\n")/* ���ù̶������� */
#define AT_CMD_SET_BAUD_RATE_RESPOND	  (AT_CMD_RESPOND_OK)
#define AT_CMD_CHECK_SIM_STATUS			  ("AT+CPIN?\r\n")	  /* ��ѯSIM��״̬ */
#define AT_CMD_CHECK_SIM_STATUS_RESPOND	  ("+CPIN: READY")
#define AT_CMD_SEARCH_NET_STATUS		  ("AT+CREG?\r\n")	  /* ��ѯ������״̬ */
//#define AT_CMD_SEARCH_NET_STATUS_RESPOND  ("+CREG: 0,1")
#define AT_CMD_SEARCH_NET_STATUS_RESPOND  ("+CREG: 0,5")
#define AT_CMD_CHECK_GPRS_STATUS		  ("AT+CGATT?\r\n")	/* ��ѯGPRS�����Ƿ�ɹ� */
#define AT_CMD_CHECK_GPRS_STATUS_RESPOND  ("+CGATT: 1")

/* ����GPRSΪ͸��ģʽ */
#define AT_CMD_SET_SINGLE_LINK			  ("AT+CIPMUX=0\r\n")	 /* ����Ϊ��·ģʽ */
#define AT_CMD_SET_SINGLE_LINK_RESPOND	  (AT_CMD_RESPOND_OK)
#define AT_CMD_SET_SERIANET_MODE		  ("AT+CIPMODE=1\r\n")	 /* ����Ϊ͸��ģʽ */
#define AT_CMD_SET_SERIANET_MODE_RESPOND  (AT_CMD_RESPOND_OK)
#define AT_CMD_SET_APN_NAME				  ("AT+CSTT=\"UNINET\"\r\n")/* ����APN���� */
#define AT_CMD_SET_APN_NAME_RESPOND       (AT_CMD_RESPOND_OK)
#define AT_CMD_ACTIVE_PDP				  ("AT+CIICR\r\n")		 /* ����PDP���� */
#define AT_CMD_ACTIVE_PDP_RESPOND		  (AT_CMD_RESPOND_OK)
#define AT_CMD_GET_SELF_IP_ADDR			  ("AT+CIFSR\r\n")		 /* ��ȡ����IP��ַ */
#define AT_CMD_GET_SELF_IP_ADDR_RESPOND   (AT_CMD_RESPOND_OK)
#define AT_CMD_GPS_ENABLE				  ("AT+EGPSC=1\r\n")		/* ʹ��GPS���� */
#define AT_CMD_GPS_ENABLE_RESPOND		  ("+MGPSSTATUS:1,1,0")
#define AT_CMD_GPS_CHECK_STATUS			  ("AT+GETGPS=\"GNRMC\"\r\n")	/* ��ȡGNRMC״̬ */
#define AT_CMD_GPS_CHECK_STATUS_RESPOND	  (AT_CMD_RESPOND_OK)
#define AT_CMD_GPS_GET_GNRMC			  ("AT+GETGPS=\"GNRMC\"\r\n")	/* ��ȡGNRMC */
#define AT_CMD_GPS_GET_GNRMC_RESPOND	  ("$GNRMC,")
#define AT_CMD_GPS_DISABLE				  ("AT+EGPSC=0\r\n")		/* ʧ��GPS���� */
#define AT_CMD_GPS_DISABLE_RESPOND		  (AT_CMD_RESPOND_OK)
#define AT_CMD_SET_SERVER_IP_ADDR		  ("AT+CIPSTART=\"TCP\",\"112.124.106.188\",\"8090\"\r\n")/* ���ӷ����� */
#define AT_CMD_SET_SERVER_IP_ADDR_RESPOND ("CONNECT OK")
#define AT_CMD_DATA_SEND_SUCCESS_RESPOND  ("Save Data")
#define AT_CMD_DATA_SEND_ERROR_RESPOND    ("Error Data")
#define AT_CMD_EXIT_SERIANET_MODE	  	  ("+++")			 	/* �˳�͸������ģʽ */
#define AT_CMD_EXIT_SERIANET_MODE_RESPOND (AT_CMD_RESPOND_OK)
#define AT_CMD_RETURN_SERIANET		  	  ("ATO\r\n")			 /* ����͸��ģʽ */
#define AT_CMD_RETURN_SERIANET_RESPOND	  ("ATO\r\n")
#define AT_CMD_EXIT_LINK_MODE			  ("AT+CIPCLOSE=0\r\n")	 /* �رյ�·Socket���� */
#define AT_CMD_EXIT_LINK_MODE_RESPOND	  ("CLOSE OK")
#define AT_CMD_SHUT_MODELU	 		  	  ("AT+CIPSHUT\r\n")		 /* �ر��ƶ����� */
#define AT_CMD_SHUT_MODELU_RESPOND		  ("SHUT OK")

#define AT_CMD_RESPOND_OK				  ("OK")

/******************************************************************************/
#define GPRSPROCESS_GPS_ENABLE				(1 << 0)
#define GPRSPROCESS_SEND_DATA_ENABLE		(1 << 1)

#define GPRS_PROCESS_TASK_RECV_ENABLE		(1 << 2)

/******************************************************************************/
typedef enum
{
	MODULE_INVALID,									/* ģ����Ч */
	MODULE_VALID,									/* ģ����Ч */
	SET_BAUD_RATE,									/* ���ò����� */
	SET_BAUD_RATE_FINISH,							/* ������������� */
	ENABLE_GPS,										/* ʹ��GPS���� */
	ENABLE_GPS_FINISH,								/* ʹ��GPS������� */
	GPS_CHECK_STATUS,								/* ���GPS״̬ */
	GPS_CHECK_STATUS_FINISH,						/* ���GPS״̬��� */
	GET_GPS_GNRMC,									/* ��ȡGNRMC��λֵ */
	GET_GPS_GNRMC_FINISH,							/* ��ȡGNRMC��λֵ��� */
	CHECK_SIM_STATUS,								/* ��ѯSIM��״̬ */
	CHECK_SIM_STATUS_FINISH,						/* ��ѯSIM��״̬��� */
	SEARCH_NET_STATUS,								/* ��������״̬ */
	SEARCH_NET_STATUS_FINISH,						/* ��������״̬��� */
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
	SET_SERVER_IP_ADDR,								/* ���÷�������ַ */
	SET_SERVER_IP_ADDR_FINISH,						/* ���÷�������ַ��� */
	READY,											/* ģ��׼���� */
	DATA_SEND_FINISH,								/* ���ݷ������ */
	EXTI_SERIANET_MODE,								/* �˳�͸��ģʽ */
	EXTI_SERIANET_MODE_FINISH,						/* �˳�͸��ģʽ��� */
	RETURN_SERIANET_MODE,							/* ����͸��ģʽ */
	RETURN_SERIANET_MODE_FINISH,					/* ����͸��ģʽ��� */
	EXTI_LINK_MODE,									/* �˳�����ģʽ */
	EXTI_LINK_MODE_FINISH,							/* �˳�����ģʽ��� */
	SHUT_MODULE,									/* �ر��ƶ����� */
	SHUT_MODULE_FINISH,								/* �ر��ƶ�������� */
} GPRS_ModuleStatusEnum;

/******************************************************************************/
void GPRSPROCESS_Task(void);


#endif
