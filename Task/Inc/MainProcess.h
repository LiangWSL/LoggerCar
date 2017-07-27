#ifndef _MAIN_TASK_H
#define _MAIN_TASK_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define MAINPROCESS_TICKS_TO_TIMEOUT			(20000)		/* �ź����ȴ���ʱ */
#define MAINPROCESS_SENSOR_CONVERT_FINISH		(1 << 0)	/* ������ת����� */
#define MAINPROCESS_GPS_CONVERT_FINISH			(1 << 1)	/* GPSת����� */
#define MAINPROCESS_GPRS_SEND_FINISHED			(1 << 2)	/* GPRS���ݷ������ */



/******************************************************************************/
void MAINPROCESS_Task(void);

#endif
