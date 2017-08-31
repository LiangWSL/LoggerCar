#include "analog.h"

#include "ntc.h"
#include "hih5030.h"

/******************************************************************************/
static uint16_t convertValueBuffer[ANALOG_SAMPLE_NUMB][ANALOG_CHANNEL_NUMB_TOTLE];
ANALOG_ValueTypedef ANALOG_value;

/******************************************************************************/
static void ANALOG_GetAverageValue(ANALOG_ConvertValueTypedef* convertValue);
static uint8_t ANALOG_GetBatVoltage(uint16_t value);

/*******************************************************************************
 * function:ģ������ʼ��
 * ע�⣺û�о�����ѹУ׼��ADC�������нϴ�ƫ��
 */
void ANALOG_Init(void)
{
	/* У׼ADC */
	HAL_ADCEx_Calibration_Start(&ANALOG_ADC);
}

/*******************************************************************************
 * function��ʹ��ADCת����ת��֮ǰ�����ģ������Դ���أ������ʵ�����ʱ
 */
void ANALOG_ConvertEnable(void)
{
	/* ��������Դ���Ƶ��ɼ��������ʵ�����ʱ */
	ANALOG_PWR_ENABLE();
	VBAT_PWR_CHECK_ENABLE();
	osDelay(10);
//	HAL_ADCEx_Calibration_Start(&ANALOG_ADC);
	HAL_ADC_Start_DMA(&ANALOG_ADC, (uint32_t*)convertValueBuffer,
								sizeof(convertValueBuffer));
}

/*******************************************************************************
 * function��ģ����ֹͣת�������ر�ģ������Դ����
 */
void ANALOG_ConvertDisable(void)
{
	HAL_ADC_Stop_DMA(&ANALOG_ADC);
	ANALOG_PWR_DISABLE();
	VBAT_PWR_CHECK_DISABLE();
}

/*******************************************************************************
 * function����ȡģ������ֵ
 */
void ANALOG_GetSensorValue(void)
{
	ANALOG_ConvertValueTypedef ANALOG_convertValue;

	/* ��ȡADת��ֵ */
	ANALOG_GetAverageValue(&ANALOG_convertValue);

	/* ��ȡ�¶� */
	ANALOG_value.temp1 = NTC_GetTemp(ANALOG_convertValue.temp1);
	ANALOG_value.temp2 = NTC_GetTemp(ANALOG_convertValue.temp2);
	ANALOG_value.temp3 = NTC_GetTemp(ANALOG_convertValue.temp3);
	ANALOG_value.temp4 = NTC_GetTemp(ANALOG_convertValue.temp4);

	/* ��ȡʪ�ȣ������� */
	ANALOG_value.humi1 = HIH5030_GetHumi(ANALOG_convertValue.humi1, ANALOG_value.temp1);
	ANALOG_value.humi2 = HIH5030_GetHumi(ANALOG_convertValue.humi2, ANALOG_value.temp2);
	ANALOG_value.humi3 = HIH5030_GetHumi(ANALOG_convertValue.humi3, ANALOG_value.temp3);
	ANALOG_value.humi4 = HIH5030_GetHumi(ANALOG_convertValue.humi4, ANALOG_value.temp4);

	/* ��ȡ��ص�ѹ */
	ANALOG_value.batVoltage = ANALOG_GetBatVoltage(ANALOG_convertValue.batVoltage + 70);
}

/*******************************************************************************
 * function����ADֵ�Ӵ�С����ȡ�м����ֵ
 */
static void ANALOG_GetAverageValue(ANALOG_ConvertValueTypedef* convertValue)
{
	uint8_t i, j, k;
	uint16_t value;
	uint32_t average;

	/* ͨ���� */
	for (i = RESET; i < ANALOG_CHANNEL_NUMB_TOTLE; i++)
	{
		/* ����,���������� */
		for (j = RESET; j < ANALOG_SAMPLE_NUMB - 1; j++)
		{
			for (k = j; k < ANALOG_SAMPLE_NUMB; k++)
			{
				if (convertValueBuffer[k][i] > convertValueBuffer[j][i])
				{
					value = convertValueBuffer[j][i];
					convertValueBuffer[j][i] = convertValueBuffer[k][i];
					convertValueBuffer[k][i] = value;
				}
			}
		}

		/* ���ƽ��ֵ */
		average = RESET;

		/* ȡ�м����ֵ��ƽ���� */
		for(j = ANALOG_SAMPLE_NUMB / 2 - 5; j < ANALOG_SAMPLE_NUMB / 2 + 5; j++)
		{
			average += convertValueBuffer[j][i];
		}
		*(&convertValue->temp1 + i) = (uint16_t)(average / 10);
	}
}

/*******************************************************************************
 * function:��ȡ﮵�ص�ѹ
 * value��﮵�ط�ѹ���ADֵ
 * ע�⣺����﮵�صĵ�ѹ��ΧΪ6~8.4V
 */
static uint8_t ANALOG_GetBatVoltage(uint16_t value)
{
	uint32_t voltage;
	uint8_t  percent;

	voltage = (uint32_t)((value * 3300) / 4096);

	voltage = ((voltage * 85) / 10) - 6000;

	percent = (voltage * 100) / 2400;

	if (percent > 100)
	{
		percent = 100;
	}

	return percent;
}



