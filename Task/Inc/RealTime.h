#ifndef __REAL_TIME_H
#define __REAL_TIME_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "rt.h"

/******************************************************************************/
#define REALTIME_SAVE_INTERVAL					(1)

/******************************************************************************/
#define REALTIME_TASK_SIGNAL_UPDATE				(1 << 0)
#define REALTIME_SENSOR_CONVERT_START			(1 << 1)	/* ��������ʼת�� */
#define REALTIME_SENSOR_CONVERT_FINISH			(1 << 2)	/* ������ת����� */

/******************************************************************************/
void REALTIME_Task(void);

#endif

