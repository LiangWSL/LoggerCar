#include "gps.h"
#include "exFlash.h"

/*******************************************************************************
 *
 */
static void str2numb(uint8_t* buf, uint8_t* info, uint8_t size)
{
	while(size--)
	{
		*info++ = (*buf++) - '0';
	}
}

/*******************************************************************************
 *
 */
static float GPS_ValueConvert(uint8_t* buf, GPS_LocationTypeEnum loc)
{
	/* ��ʽ��abcde.fghi
	 * ���㷽ʽ�� abc + (de / 60) + (fghi / 600000) */

	uint16_t abc  = 0;
	float 	 de   = 0;
	float    fghi = 0;

	switch(loc)
	{
	case GPS_LATITUDE:
		abc  = (*buf * 10) + *(buf + 1);
		de   = (float)((*(buf + 2) * 10) + *(buf + 3)) / 60;
		fghi = (float)((*(buf + 4) * 1000) + (*(buf + 5) * 100) + (*(buf + 6) * 10) + *(buf + 7)) / 600000;
		break;
	case GPS_LONGITUDE:
		abc  = (*buf * 100) + (*(buf + 1) * 10) + *(buf + 2);
		de   = (float)((*(buf + 3) * 10) + *(buf + 4)) / 60;
		fghi = (float)((*(buf + 5) * 1000) + (*(buf + 6) * 100) + (*(buf + 7) * 10) + *(buf + 8)) / 600000;
		break;
	default:
		break;
	}

	return abc + de + fghi;
}

/*******************************************************************************
 * ��ȡ��λ����
 */
void GPS_GetLocation(uint8_t* buf, GPS_LocationTypedef* info)
{
	uint8_t latitude[8]  = {0};
	uint8_t longitude[9] = {0};

	/* �ж�������Ч */
	if (GPS_FLAG_VALID == *(buf + GPS_OFFSET_FLAG))
	{
		str2numb((buf + GPS_OFFSET_LATITUDE),     &latitude[0], 4);
		str2numb((buf + GPS_OFFSET_LATITUDE + 5), &latitude[4], 4);

		str2numb(buf + GPS_OFFSET_LONGITUDE,     &longitude[0], 5);
		str2numb(buf + GPS_OFFSET_LONGITUDE + 6, &longitude[5], 4);

		/* ��γ�Ȼ�ȡ */
		info->longitude = GPS_ValueConvert(longitude, GPS_LONGITUDE);
		info->latitude  = GPS_ValueConvert(latitude,  GPS_LATITUDE);

		if (*(buf + GPS_OFFSET_LATITUDE_FLAG) == 'S')
			info->latitude = -(info->latitude);
		if (*(buf + GPS_OFFSET_LATITUDE_FLAG) == 'W')
			info->longitude = -(info->longitude);
	}
}


