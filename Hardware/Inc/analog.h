#ifndef __ANALOG_H
#define __ANALOG_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "adc.h"
#include "public.h"
#include "file.h"

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
#pragma pack(1)											/* ���ֽڶ��� */

typedef struct
{
	uint16_t temp1;					/* temp1ת��ֵ */
	uint16_t humi1;					/* humi1ת��ֵ */
	uint16_t temp2;					/* temp2ת��ֵ */
	uint16_t humi2;					/* humi2ת��ֵ */
	uint16_t temp3;					/* temp3ת��ֵ */
	uint16_t humi3;					/* humi3ת��ֵ */
	uint16_t temp4;					/* temp4ת��ֵ */
	uint16_t humi4;					/* humi4ת��ֵ */
	uint16_t batVoltage;			/* ��ص�ѹ */
} ANALOG_ConvertValueTypedef;

typedef struct
{
	float temp1;				/* temp1ת��ֵ */
	float temp2;				/* temp2ת��ֵ */
	float temp3;				/* temp3ת��ֵ */
	float temp4;				/* temp4ת��ֵ */

	float humi1;				/* humi1ת��ֵ */
	float humi2;				/* humi2ת��ֵ */
	float humi3;				/* humi3ת��ֵ */
	float humi4;				/* humi4ת��ֵ */

	uint8_t batVoltage;				/* ��ص�ѹ */
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
void ANALOG_Float2ASCII(FILE_SaveInfoAnalogTypedef* buffer, float value);

#endif
