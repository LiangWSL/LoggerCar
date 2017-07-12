#include "hih5030.h"

/*******************************************************************************
 * HIH5030�¶Ȳ�����
 * @param currentHumi����ǰʪ��
 * @param currentTemp����ǰ�¶�
 * return �¶Ȳ������ֵ��
 *
 * HIH5030�¶Ȳ�����ʽ����ʵRH = ��������RH��/ ��1.0546 - 0.00216 * ��ǰ�¶ȣ���
 */
static float HIH5030_Adjust(float currentHumi, float currentTemp)
{
	return (float)(currentHumi / (1.0546 - 0.00216 * currentTemp));
}

/*******************************************************************************
 * function��HIH5030��ȡʪ��
 * @param analogValue:ADת��ֵ
 * @param currentTemp����ǰ�¶�ֵ������ʪ�ȴ��������¶Ȳ���
 */
float HIH5030_GetHumi(uint16_t analogValue, float currentTemp)
{
	float humiValue, voltage;

	/* ��ȡ��ѹֵ */
	voltage = (float)((3.300 * analogValue) / 4096);

	/* ��ȡʪ��ֵ */
	humiValue = (float)(((float)(voltage / 3.300) - 0.1515) / 0.00636);

	/* �¶Ȳ���ֵ */
	humiValue = HIH5030_Adjust(humiValue, currentTemp);
	
	return humiValue;
}


