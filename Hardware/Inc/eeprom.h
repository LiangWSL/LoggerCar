#ifndef __EEPROM_H
#define __EEPROM_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "stm32f105xc.h"
#include "i2c.h"

#include "analog.h"



/******************************************************************************/
#define EEPROM_I2C					(hi2c2)
#define EEPROM_DEVICE_ADDR			(0xA0)		/* EEPROM�豸��ַ */

#define EE_ADDR_BASE							(0X000)
#define EE_ADDR_DEVICE_INIT						(0x000)
#define EE_ADDR_DEVICE_SN						(0x001)
#define EE_ADDR_FIRMWARE_VERSION				(0x00B)
#define EE_ADDR_RECORD_INTERVAL					(0x00C)
#define EE_ADDR_OVER_LIMIT_RECORD_INTERVAL		(0x00E)
#define EE_ADDR_EXIT_ANALOG_CHANNEL_NUMB		(0x00E)
#define EE_ADDR_PARAM_1							(0x00F)
#define EE_ADDR_PARAM_2							(0x012)
#define EE_ADDR_PARAM_3							(0x015)
#define EE_ADDR_PARAM_4							(0x018)
#define EE_ADDR_PARAM_5							(0x01B)
#define EE_ADDR_PARAM_6							(0x02E)
#define EE_ADDR_PARAM_7							(0x021)
#define EE_ADDR_PARAM_8							(0x024)
#define EE_ADDR_PARAM_9							(0x027)
#define EE_ADDR_PARAM_10						(0x02A)
#define EE_ADDR_PARAM_11						(0x02D)
#define EE_ADDR_PARAM_12						(0x030)
#define EE_ADDR_PARAM_13						(0x033)
#define EE_ADDR_PARAM_14						(0x036)

#define EE_ADDR_FLASH_INFO_SAVE_ADDR			(0x039)


/******************************************************************************/
typedef enum
{
	LOCATION_STATION,								/* ��վ��λ */
	LOCATION_GPS,									/* GPS��λ */
} EE_LocationTypeEnum;

typedef enum
{
	TYPE_TEMP = 1,									/* �¶� */
	TYPE_HUMI,										/* ʪ�� */
	TYPE_RAINFALL,									/* ���� */
	TYPE_SOIL_MOISTURE,								/* ����ˮ�� */
	TYPE_VOLTAGE,									/* ��ѹ */
	TYPE_ELECTRIC_CURRENT,							/* ���� */
	TYPE_POWER,										/* ���� */
	TYPE_ILLUMINATION_INTENSITY,					/* ����ǿ�� */
	TYPE_WIND_SPEED,								/* ���� */
	TYPE_WIND_DIR,									/* ���� */
	TYPE_CO2,										/* CO2 */
	TYPE_BAROMETRIC_PRESSURE,						/* ����ѹ */
	TYPE_O2,										/* O2 */
	TYPE_NUMB_OF_TIMES,								/* ���� */
	TYPE_WATER_LEVEL,								/* ˮλ */
	TYPE_PRESSURE,									/* ѹ�� */
	TYPE_PH_VALUE,									/* PHֵ */
} EE_ChannelTypeEnum;

typedef enum
{
	UNIT_TEMP = 1,									/* �¶� */
	UNIT_HUMI,										/* ʪ�� */
	UNIT_RAINFALL,									/* ���� */
	UNIT_SOIL_MOISTURE,								/* ����ˮ�� */
	UNIT_VOLTAGE,									/* ��ѹ */
	UNIT_ELECTRIC_CURRENT,							/* ���� */
	UNIT_POWER,										/* ���� */
	UNIT_ILLUMINATION_INTENSITY,					/* ����ǿ�� */
	UNIT_WIND_SPEED,								/* ���� */
	UNIT_WIND_DIR,									/* ���� */
	UNIT_CO2,										/* CO2 */
	UNIT_BAROMETRIC_PRESSURE,						/* ����ѹ */
	UNIT_O2,										/* O2 */
	UNIT_NUMB_OF_TIMES,								/* ���� */
	UNIT_WATER_LEVEL,								/* ˮλ */
	UNIT_PRESSURE,									/* ѹ�� */
	UNIT_PH_VALUE,									/* PHֵ */
} EE_ChannelUnitEnum;

typedef enum
{
	FORMAT_INT = 1,									/* ���ݸ�ʽ���� */
	FORMAT_ONE_DECIMAL,								/* ���ݸ�ʽ1λС�� */
	FORMAT_TWO_DECIMAL,								/* ���ݸ�ʽ2λС�� */
} EE_DataFormatEnum;

/******************************************************************************/
typedef struct
{
	EE_ChannelTypeEnum channelType;					/* ͨ������ */
	EE_ChannelUnitEnum channelUnit;					/* ͨ����λ */
	EE_DataFormatEnum  dataFormat;					/* ������ʽ */
} EE_ParamTypedef;


/******************************************************************************/
extern uint8_t EE_DeviceInit;
extern char 	EE_deviceSN[10];
extern uint8_t EE_firmwareVersion;
extern uint8_t EE_recordInterval;
extern uint8_t EE_overLimitRecordInterval;
extern uint8_t EE_exitAnalogChannelNumb;
extern EE_ParamTypedef EE_Param[ANALOG_CHANNEL_NUMB_MAX];
extern uint32_t EE_FlashInfoSaveAddr;

/******************************************************************************/
void EEPROM_WriteBytes(uint16_t addr, void* pBuffer, uint8_t dataLength);
void EEPROM_ReadBytes(uint16_t addr, uint8_t* pBuffer, uint8_t dataLength);
void DEVICE_Init(void);
#endif
