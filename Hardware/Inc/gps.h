#ifndef __GPS_H
#define __GPS_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/******************************************************************************/
#define GPS_FLAG_VALID					('A')			/* ������Ч */
#define GPS_FLAG_INVALID				('V')			/* ������Ч */

#define GPS_OFFSET_FLAG					(18)			/* ������Ч��־λƫ�� */
#define GPS_OFFSET_LATITUDE				(20)			/* γ��ֵƫ�� */
#define GPS_OFFSET_LATITUDE_FLAG		(30)			/* γ��ֵ��־λN��S */
#define GPS_OFFSET_LONGITUDE			(32)			/* ����ֵƫ�� */
#define GPS_OFFSET_LONGITUDE_FLAG		(43)			/* ����ֵ��־λE��W */

/*******************************************************************************
 *
 */
typedef enum
{
	GPS_LATITUDE,
	GPS_LONGITUDE
} GPS_LocationTypeEnum;

typedef struct
{
	double latitude;
	double longitude;
} GPS_LocateTypedef;

/******************************************************************************/
void GPS_GetLocation(uint8_t* buf, GPS_LocateTypedef* info);

#endif
