#ifndef __ANALOG_H
#define __ANALOG_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "adc.h"

/******************************************************************************/
#define ANALOG_ADC								(hadc1)
/* ��������Դ�������� */
#define ANALOG_PWR_ENABLE() \
	HAL_GPIO_WritePin(SENSOR_PWR_CTRL_GPIO_Port, SENSOR_PWR_CTRL_Pin, GPIO_PIN_RESET);
#define ANALOG_PWR_DISABLE() \
	HAL_GPIO_WritePin(SENSOR_PWR_CTRL_GPIO_Port, SENSOR_PWR_CTRL_Pin, GPIO_PIN_SET);

/* ﮵�ص�ѹ�ɼ���Դ�������� */
#define VBAT_PWR_CHECK_ENABLE() \
	HAL_GPIO_WritePin(VBAT_CTRL_GPIO_Port, VBAT_CTRL_Pin, GPIO_PIN_RESET);
#define VBAT_PWR_CHECK_DISABLE() \
	HAL_GPIO_WritePin(VBAT_CTRL_GPIO_Port, VBAT_CTRL_Pin, GPIO_PIN_SET);

#define ANALOG_CHANNEL_NUMB_MAX					(14)	/* ���֧��14ͨ����ģ���� */
#define ANALOG_CHANNEL_NUMB_TOTLE      			(9)		/* ģ����ͨ����(������ʪ�Ⱥ�﮵�ص�ѹ�ɼ�) */
#define ANALOG_CHANNEL_NUMB      				(8)		/* ģ����ͨ���� */
#define ANALOG_SAMPLE_NUMB						(20)	/* ģ���������� */

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)											/* ���ֽڶ��� */

typedef struct
{
	uint16_t temp1;				/* temp1ת��ֵ */
	uint16_t temp2;				/* temp2ת��ֵ */
	uint16_t temp3;				/* temp3ת��ֵ */
	uint16_t temp4;				/* temp4ת��ֵ */

	uint16_t humi1;				/* humi1ת��ֵ */
	uint16_t humi2;				/* humi2ת��ֵ */
	uint16_t humi3;				/* humi3ת��ֵ */
	uint16_t humi4;				/* humi4ת��ֵ */

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

#pragma pack(pop)
/******************************************************************************/
void ANALOG_Init(void);
void ANALOG_GetSensorValue(ANALOG_ValueTypedef* value);
void ANALOG_ConvertEnable(void);
void ANALOG_ConvertDisable(void);

#endif
