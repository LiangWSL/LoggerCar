#ifndef __EX_FLASH_H
#define __EX_FLASH_H

/******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "common.h"

#include "spi.h"
#include "eeprom.h"
#include "input.h"
#include "gps.h"

#include "RealTime.h"


/******************************************************************************/
#define exFLASH_CS_ENABLE() \
		HAL_GPIO_WritePin(SPI_CS_FLASH_GPIO_Port, SPI_CS_FLASH_Pin, GPIO_PIN_RESET);
#define exFLASH_CS_DISABLE() \
		HAL_GPIO_WritePin(SPI_CS_FLASH_GPIO_Port, SPI_CS_FLASH_Pin, GPIO_PIN_SET);

#define exFLASH_SPI		(hspi1)

/*******************************************************************************
*W25Q����ָ��
*/
#define exFLASH_CMD_WRITE_ENABLE					(uint8_t)(0X06)
#define exFLASH_CMD_WRITE_DISABLE					(uint8_t)(0X04)
#define exFLASH_CMD_READ_STATUS_REG					(uint8_t)(0X05)
#define exFLASH_CMD_WRITE_STATUS_REG				(uint8_t)(0X01)
#define exFLASH_CMD_READ_DATA						(uint8_t)(0X03)
#define exFLASH_CMD_FAST_READ_DATA					(uint8_t)(0X0B)
#define exFLASH_CMD_FAST_READ_DUAL					(uint8_t)(0X3B)
#define exFLASH_CMD_PAGE_PROGRAM					(uint8_t)(0X02)
#define exFLASH_CMD_BLOCK_ERASE						(uint8_t)(0XD8)
#define exFLASH_CMD_SECTOR_ERASE					(uint8_t)(0X20)
#define exFLASH_CMD_CHIP_ERASE						(uint8_t)(0XC7)
#define exFLASH_CMD_POWER_DOWN						(uint8_t)(0XB9)
#define exFLASH_CMD_RELEASE_POWER_DOWN				(uint8_t)(0XAB)
#define exFLASH_CMD_DEVICE_ID						(uint8_t)(0XAB)
#define exFLASH_CMD_MANUFACE_DIVICE_ID				(uint8_t)(0X90)
#define exFLASH_CMD_JEDEC_DIVICE_ID					(uint8_t)(0X9F)

#define exFLASH_ID									(uint32_t)(0X00EF4017)
#define DUMMY_BYTE									(uint8_t)(0xFF)

#define exFLASH_PAGE_SIZE_BYTES						(256)
#define exFLASH_SECTOR_SIZE_BYTES					(4096)
#define exFLASH_SECTOR_COUNT						(1024)
//#define exFLASH_BLOCK_SIZE							(65536)
//#define exFLASH_BLOCK_COUNT							(128)
#define exFLASH_BLOCK_SIZE							(1)		/* ������ */

/******************************************************************************/
#pragma pack(push)
#pragma pack(1)									/* ���ֽڶ��� */

typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
} exFLASH_RealTime;

typedef struct
{
	uint16_t temp1;
	uint16_t humi1;

	uint16_t temp2;
	uint16_t humi2;

	uint16_t temp3;
	uint16_t humi3;

	uint16_t temp4;
	uint16_t humi4;
} exFLASH_AnalogValueTypedef;

#define ANALOG_VALUE_FORMAT						(FORMAT_ONE_DECIMAL)

typedef struct
{
	exFLASH_RealTime realTime;					/* ʱ�� */
	uint8_t  batteryLevel;						/* ��ص��� */
	uint8_t  externalPowerStatus;				/* �ⲿ���״̬ */
	uint32_t longitude;							/* ���� */
	uint32_t latitude;							/* γ�� */
	uint8_t  resever;							/* ���� */
	exFLASH_AnalogValueTypedef analogValue;		/* ģ����ֵ */
} exFLASH_InfoTypedef;

#pragma pack(pop)

/******************************************************************************/
void exFLASH_Init(void);
void exFLASH_WriteBuffer(uint32_t writeAddr, uint8_t* pBuffer, uint16_t dataLength);
void exFLASH_ReadBuffer(uint32_t readAddr, uint8_t* pBuffer, uint16_t dataLength);

void exFLASH_SectorErase(uint32_t sectorAddr);
void exFLASH_ChipErase(void);
void exFLASH_ModePwrDown(void);
void exFLASH_ModeWakeUp(void);


uint32_t exFLASH_ReadDeviceID(void);
void exFLASH_ReadStructInfo(exFLASH_InfoTypedef* info);
void exFLASH_SaveStructInfo(exFLASH_InfoTypedef* saveInfo,
							RT_TimeTypedef*      realTime,
							ANALOG_ValueTypedef* analogValue,
							GPS_LocateTypedef*   location);

#endif
