#ifndef __ANALOG_H
#define __ANALOG_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "adc.h"
#include "public.h"
//#include "file.h"

/******************************************************************************/
#define ANALOG_ADC								(hadc1)
/* ��������Դ�������� */
#define ANALOG_PWR_ENABLE() \
	HAL_GPIO_WritePin(O_VSENSOR_GPIO_Port, O_VSENSOR_Pin, GPIO_PIN_RESET);
#define ANALOG_PWR_DISABLE() \
	HAL_GPIO_WritePin(O_VSENSOR_GPIO_Port, O_VSENSOR_Pin, GPIO_PIN_SET);

/* ﮵�ص�ѹ�ɼ���Դ�������� */
#define VBAT_PWR_CHECK_ENABLE() \
	HAL_GPIO_WritePin(O_VBAT_GPIO_Port, O_VBAT_Pin, GPIO_PIN_RESET);
#define VBAT_PWR_CHECK_DISABLE() \
	HAL_GPIO_WritePin(O_VBAT_GPIO_Port, O_VBAT_Pin, GPIO_PIN_SET);

#define ANALOG_SAMPLE_NUMB						(20)	/* ģ���������� */
#define ANALOG_CHANNEL_AD_VALUE_MIN				(20)	/* ͨ��AD���ֵ */
#define ANALOG_CHANNLE_INVALID_VALUE			(float)(-127)	/* ͨ����Чֵ��־ֵ */

#define ANALOG_INVALID_VALUE					(" NULL") /* ��Чֵ */

/******************************************************************************/
typedef enum
{
	ANALOG_MODE_NORMAL,				/* ����ģʽ */
	ANALOG_MODE_PERWARM,			/* Ԥ��ģʽ */
	ANALOG_MODE_ALARM				/* ����ģʽ */
} ANALOG_ModeEnum;					/* ģ����ģʽ */

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)						/* ���ֽڶ��� */

typedef struct
{
	uint16_t batVoltage;			/* ��ص�ѹ */
	uint16_t channel1;				/* channel1ת��ֵ */
	uint16_t channel2;				/* channel2ת��ֵ */
	uint16_t channel3;				/* channel3ת��ֵ */
	uint16_t channel4;				/* channel4ת��ֵ */
	uint16_t channel5;				/* channel5ת��ֵ */
	uint16_t channel6;				/* channel6ת��ֵ */
	uint16_t channel7;				/* channel7ת��ֵ */
	uint16_t channel8;				/* channel8ת��ֵ */
} ANALOG_ConvertValueTypedef;

typedef struct
{
	uint8_t batVoltage;				/* ��ص�ѹ */
	float channel1;					/* channel1ת��ֵ */
	float channel2;					/* channel2ת��ֵ */
	float channel3;					/* channel3ת��ֵ */
	float channel4;					/* channel4ת��ֵ */
	float channel5;					/* channel5ת��ֵ */
	float channel6;					/* channel6ת��ֵ */
	float channel7;					/* channel7ת��ֵ */
	float channel8;					/* channel8ת��ֵ */
} ANALOG_ValueTypedef;

typedef struct
{
	union
	{
		struct
		{
			uint8_t ch1:1;
			uint8_t ch2:1;
			uint8_t ch3:1;
			uint8_t ch4:1;
			uint8_t ch5:1;
			uint8_t ch6:1;
			uint8_t ch7:1;
			uint8_t ch8:1;
		} bit;
		uint8_t all;
	} status;
} ANALOG_AlarmStatusTypedef;

#pragma pack(pop)

/******************************************************************************/
extern ANALOG_ValueTypedef ANALOG_value;
extern ANALOG_AlarmStatusTypedef ANALOG_alarmStatus;
extern BOOL ANALOG_alarmOccur;

/******************************************************************************/
void ANALOG_Init(void);
void ANALOG_ConvertEnable(void);
void ANALOG_ConvertDisable(void);
void ANALOG_GetSensorValue(void);
//void ANALOG_Float2ASCII(FILE_SaveInfoAnalogTypedef* buffer, float value);

#endif
