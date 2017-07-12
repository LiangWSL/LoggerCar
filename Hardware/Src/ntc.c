#include "ntc.h"

/*******************************************************************************
 * ���ݵ���ֵ���ó��¶�ֵ
 * @ param��analogValue:����ADֵ
 * @ output��float����¶ȸ�������ֵ
 */
static float NTC_CheckTable(volatile uint32_t resValue)
{
	volatile uint8_t vIndex, hIndex;
	float temp;

	/* ����ֵ��Ч */
	if ((resValue > NTC_Table[0][0])
			|| (resValue < NTC_Table[NTC_TABLE_LAYER - 1][NTC_TABLE_INDEX - 1]))
	{
		return temp = NULL;
	}

	/* ������ */
	for (vIndex = RESET; vIndex < NTC_TABLE_LAYER; vIndex++)
	{
		if (resValue > NTC_Table[vIndex][1])
		{
			vIndex--;
			break;
		}
	}

	/* ������ */
	for (hIndex = 1; hIndex < NTC_TABLE_INDEX; hIndex++)
	{
		if (resValue > NTC_Table[vIndex][hIndex])
		{
			break;
		}
	}

	/* ��ά����֮���ֵ */
	if (NTC_TABLE_INDEX == hIndex)
	{
		temp = (float)(NTC_Table[vIndex][hIndex - 1] - resValue)
			/ (float)(NTC_Table[vIndex][hIndex - 1] - NTC_Table[vIndex + 1][0]);

		temp += (float)(((vIndex * 10) + (hIndex - 1)) - NTC_TABLE_TEMP_OFFSET);
	}
	else /* �����б��е�ֵ */
	{
		temp = (float)(NTC_Table[vIndex][hIndex - 1] - resValue)
		   / (float)(NTC_Table[vIndex][hIndex - 1] - NTC_Table[vIndex][hIndex]);

		temp += (float)(((vIndex * 10) + (hIndex - 1)) - NTC_TABLE_TEMP_OFFSET);
	}

	return temp;
}

/*******************************************************************************
 * @analogValue:ADֵ
 */
float NTC_GetTemp(uint16_t analogValue)
{
	uint32_t resValue;

	/* ��ADֵת���ɵ���ֵ */
	resValue = (uint32_t)(((uint32_t)(STM32_AD_FULL_VALUE * NTC_REF_RES)
					/ analogValue) - NTC_REF_RES);

	/* �����¶Ȳ����� */
	return NTC_CheckTable(resValue);
}
