#ifndef __REAL_TIME_H
#define __REAL_TIME_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "rt.h"

/******************************************************************************/
#define REALTIME_SAVE_INTERVAL					(1)

/******************************************************************************/
#define REALTIME_TASK_TIME_ANALOG_UPDATE		(1 << 0)	/* ʱ���ģ�������� */
#define REALTIME_TASK_ALRAM_RECORD				(1 << 1)	/* ���Ӵ�����¼ */
#define REALTIME_TASK_SENSOR_CONVERT_FINISH		(1 << 2)	/* ������ת�����  */

/******************************************************************************/
void REALTIME_Task(void);

#endif

