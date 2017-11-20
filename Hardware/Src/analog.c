#include "analog.h"

#include "ntc.h"
#include "hih5030.h"
#include "param.h"

/******************************************************************************/
static uint16_t convertValueBuffer[ANALOG_SAMPLE_NUMB][ANALOG_CHANNEL_NUMB_TOTLE];
ANALOG_ValueTypedef ANALOG_value;
ANALOG_AlarmStatusTypedef ANALOG_alarmStatus;
BOOL ANALOG_alarmOccur = FALSE;				/* ģ������������ */
ANALOG_ModeEnum ANALOG_Mode;				/* ģ��������ģʽ */

/******************************************************************************/
static void ANALOG_GetAverageValue(ANALOG_ConvertValueTypedef* convertValue);
static uint8_t ANALOG_GetBatVoltage(uint16_t value);

/*******************************************************************************
 * @brief ģ������ʼ��
 * ע�⣺û�о�����ѹУ׼��ADC�������нϴ�ƫ��
 */
void ANALOG_Init(void)
{
	/* У׼ADC */
	HAL_ADCEx_Calibration_Start(&ANALOG_ADC);
}

/*******************************************************************************
 * @brief ʹ��ADCת����ת��֮ǰ�����ģ������Դ���أ������ʵ�����ʱ
 */
void ANALOG_ConvertEnable(void)
{
	/* ��������Դ���Ƶ��ɼ��������ʵ�����ʱ */
	ANALOG_PWR_ENABLE();
	VBAT_PWR_CHECK_ENABLE();
	osDelay(50);
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
 * @brief ��ȡģ������ֵ,���ADֵ<ͨ��AD���ֵANALOG_CHANNEL_AD_VALUE_MIN��
 * 		  ��֤����ͨ��������δ���ӻ�������,���ͨ����ֵ��־λANALOG_CHANNLE_INVALID_VALUE
 */
void ANALOG_GetSensorValue(void)
{
	ANALOG_ConvertValueTypedef ANALOG_convertValue;

	/* ��ȡADת��ֵ */
	ANALOG_GetAverageValue(&ANALOG_convertValue);

	/* ��ȡ��ص�ѹ */
	ANALOG_value.batVoltage = ANALOG_GetBatVoltage(ANALOG_convertValue.batVoltage);
	/* ��ȡ�¶� */
	ANALOG_value.channel1 = NTC_GetTemp(ANALOG_convertValue.channel1);
	ANALOG_value.channel2 = HIH5030_GetHumi(ANALOG_convertValue.channel2, ANALOG_value.channel1);
	ANALOG_value.channel3 = NTC_GetTemp(ANALOG_convertValue.channel3);
	ANALOG_value.channel4 = HIH5030_GetHumi(ANALOG_convertValue.channel4, ANALOG_value.channel3);
	ANALOG_value.channel5 = NTC_GetTemp(ANALOG_convertValue.channel5);
	ANALOG_value.channel6 = HIH5030_GetHumi(ANALOG_convertValue.channel6, ANALOG_value.channel5);
	ANALOG_value.channel7 = NTC_GetTemp(ANALOG_convertValue.channel7);
	ANALOG_value.channel8 = HIH5030_GetHumi(ANALOG_convertValue.channel8, ANALOG_value.channel7);
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
		*(&convertValue->batVoltage + i) = (uint16_t)(average / 10);
	}
}

/*******************************************************************************
 * @brief ��ȡ﮵�ص�ѹ������3�ŵ�ֵ�����ѹ
 * value��﮵�ط�ѹ���ADֵ
 * ע�⣺����﮵�صĵ�ѹ��ΧΪ6.4~8.4V
 */
static uint8_t ANALOG_GetBatVoltage(uint16_t value)
{
	uint16_t voltage;
	uint8_t  percent;

	/* ��ȡ��ѹֵ */
	voltage = (uint16_t)((((uint32_t)value * 3300) / 4096) * 3 - 6000);

	percent = (voltage * 100) / 2400;

	if (percent > 100)
	{
		percent = 100;
	}

	return percent;
}




