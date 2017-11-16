#ifndef _MAIN_TASK_H
#define _MAIN_TASK_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define MAINPROCESS_TICKS_TO_TIMEOUT			(20000)		/* �ź����ȴ���ʱ */
#define MAINPROCESS_START_TASK					(1 << 1)	/* �������� */
#define MAINPROCESS_GPS_CONVERT_FINISH			(1 << 2)	/* GPSת����� */
#define MAINPROCESS_GPRS_SEND_FINISHED			(1 << 3)	/* GPRS���ݷ������ */
#define MAINPROCESS_GPRS_SEND_ERROR				(1 << 4)	/* GPRS���ݷ��ʹ��� */


/******************************************************************************/
void MAINPROCESS_Task(void);

#endif
