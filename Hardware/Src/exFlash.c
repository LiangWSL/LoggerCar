#include "exFlash.h"

#include "gps.h"


/*******************************************************************************
 * 读写一个字节数据
 */
static uint8_t exFLASH_WriteReadByte(uint8_t byte)
{
	while(!__HAL_SPI_GET_FLAG(&exFLASH_SPI, SPI_FLAG_TXE));
	exFLASH_SPI.Instance->DR = byte;

	while(!__HAL_SPI_GET_FLAG(&exFLASH_SPI, SPI_FLAG_RXNE));
	return exFLASH_SPI.Instance->DR;
}

/******************************************************************************/
uint32_t exFLASH_ReadDeviceID(void)
{
	uint32_t temp = RESET;
	uint8_t temp1 = RESET, temp2 = RESET, temp3 = RESET;

	exFLASH_CS_ENABLE();

	exFLASH_WriteReadByte(exFLASH_CMD_JEDEC_DIVICE_ID);

	/* 读数据 */
	temp1 = exFLASH_WriteReadByte(DUMMY_BYTE);
	temp2 = exFLASH_WriteReadByte(DUMMY_BYTE);
	temp3 = exFLASH_WriteReadByte(DUMMY_BYTE);

	exFLASH_CS_DISABLE();

	temp = (temp1 << 16) | (temp2 << 8) | temp3;

	return temp;
}

/*******************************************************************************
 * 发送flash写使能指令
 */
static void exFLASH_WriteEnable(void)
{
	exFLASH_CS_ENABLE();

	exFLASH_WriteReadByte(exFLASH_CMD_WRITE_ENABLE);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * FLASH 芯片向内部存储矩阵写入数据需要消耗一定的时间，并不是在总线通讯结束的一瞬间完成的，所以在写操作
 * 后需要确认FLASH芯片“空闲”时才能再次写入
 */
static void exFLASH_WaitForIdle(void)
{
	exFLASH_CS_ENABLE();

	exFLASH_WriteReadByte(exFLASH_CMD_READ_STATUS_REG);

	/* flash正忙，等待，寄存器的最后一位为BUSY位 */
	while(exFLASH_WriteReadByte(DUMMY_BYTE) & 0x01);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * 注意：地址对齐4KB
 */
void exFLASH_SectorErase(uint32_t sectorAddr)
{
	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* 发送扇区擦除命令 */
	exFLASH_WriteReadByte(exFLASH_CMD_SECTOR_ERASE);

	/* 发送扇区地址，高位在前 */
	exFLASH_WriteReadByte(sectorAddr & 0xFF0000 >> 16);
	exFLASH_WriteReadByte(sectorAddr & 0x00FF00 >> 8);
	exFLASH_WriteReadByte(sectorAddr & 0x0000FF);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 * 注意：地址对齐64KB
 */
void exFLASH_BlockErase(uint32_t blockAddr)
{
	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* 发送扇区擦除命令 */
	exFLASH_WriteReadByte(exFLASH_CMD_BLOCK_ERASE);

	/* 发送扇区地址，高位在前 */
	exFLASH_WriteReadByte(blockAddr & 0xFF0000 >> 16);
	exFLASH_WriteReadByte(blockAddr & 0x00FF00 >> 8);
	exFLASH_WriteReadByte(blockAddr & 0x0000FF);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 *
 */
void exFLASH_ChipErase(void)
{
	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* 发送扇区擦除命令 */
	exFLASH_WriteReadByte(exFLASH_CMD_CHIP_ERASE);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 *
 */
static void exFLASH_WritePageBytes(uint32_t writeAddr, uint8_t* pBuffer,
								 uint16_t dataLength)
{
	/* 写数据的长度不能大于flash的页字节 */
	if (dataLength > exFLASH_PAGE_SIZE_BYTES)
		dataLength = exFLASH_PAGE_SIZE_BYTES;

	exFLASH_WriteEnable();
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* 发送扇区写命令 */
	exFLASH_WriteReadByte(exFLASH_CMD_PAGE_PROGRAM);

	/* 发送扇区地址，高位在前 */
	exFLASH_WriteReadByte((writeAddr & 0xFF0000) >> 16);
	exFLASH_WriteReadByte((writeAddr & 0xFF00) >> 8);
	exFLASH_WriteReadByte(writeAddr & 0xFF);

	while (dataLength--)
		exFLASH_WriteReadByte(*pBuffer++);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}

/*******************************************************************************
 *
 */
void exFLASH_WriteBuffer(uint32_t writeAddr, uint8_t* pBuffer, uint16_t dataLength)
{
	uint8_t pageBytesRemainder;				/* 该页剩余可写字节数 */
	uint8_t pageWriteNumb;					/* 需要写页数 */
	uint8_t dataBytesRemainder;				/* 最后一页还剩字节数 */

	/* 写地址与flash页地址对齐 */
	if (0 == (writeAddr % exFLASH_PAGE_SIZE_BYTES))
	{
		/* 字节长度<=页字节长度 */
		if (dataLength <=  exFLASH_PAGE_SIZE_BYTES)
			exFLASH_WritePageBytes(writeAddr, pBuffer, dataLength);
		else
		{
			/* 需要写的页数 */
			pageWriteNumb = dataLength / exFLASH_PAGE_SIZE_BYTES;
			/* 写完页，还需写的字节数 */
			dataBytesRemainder = dataLength % exFLASH_PAGE_SIZE_BYTES;

			while (pageWriteNumb--)
			{
				exFLASH_WritePageBytes(writeAddr, pBuffer, exFLASH_PAGE_SIZE_BYTES);
				writeAddr += exFLASH_PAGE_SIZE_BYTES;
				pBuffer   += exFLASH_PAGE_SIZE_BYTES;
			}
			exFLASH_WritePageBytes(writeAddr, pBuffer, dataBytesRemainder);
		}
	}
	else
	{
		/* 当前页可写入字节数 */
		pageBytesRemainder = exFLASH_PAGE_SIZE_BYTES - (writeAddr % exFLASH_PAGE_SIZE_BYTES);

		/* 当前页可以写完 */
		if (dataLength <= pageBytesRemainder)
			exFLASH_WritePageBytes(writeAddr, pBuffer, dataLength);
		else
		{
			/* 先把当前页写满 */
			exFLASH_WritePageBytes(writeAddr, pBuffer, pageBytesRemainder);
			writeAddr  += pageBytesRemainder;
			pBuffer    += pageBytesRemainder;
			dataLength -= pageBytesRemainder;

			/* 需要写的页数 */
			pageWriteNumb = dataLength / exFLASH_PAGE_SIZE_BYTES;
			while (pageWriteNumb--)
			{
				exFLASH_WritePageBytes(writeAddr, pBuffer, exFLASH_PAGE_SIZE_BYTES);
				writeAddr += exFLASH_PAGE_SIZE_BYTES;
				pBuffer += exFLASH_PAGE_SIZE_BYTES;
			}

			/* 写完页，还需写的字节数 */
			dataBytesRemainder = dataLength % exFLASH_PAGE_SIZE_BYTES;
			if (dataBytesRemainder > 0)
				exFLASH_WritePageBytes(writeAddr, pBuffer, dataBytesRemainder);
		}
	}
}


/*******************************************************************************
 *
 */
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
 uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	/*mod运算求余，若writeAddr是SPI_FLASH_PageSize整数倍，运算结果Addr值为0*/
 Addr = WriteAddr % SPI_FLASH_PageSize;

	/*差count个数据值，刚好可以对齐到页地址*/
 count = SPI_FLASH_PageSize - Addr;
	/*计算出要写多少整数页*/
 NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	/*mod运算求余，计算出剩余不满一页的字节数*/
 NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

	/* Addr=0,则WriteAddr 刚好按页对齐 aligned  */
 if (Addr == 0)
 {
		/* NumByteToWrite < SPI_FLASH_PageSize */
   if (NumOfPage == 0)
   {
	   exFLASH_WritePageBytes(WriteAddr,pBuffer,  NumByteToWrite);
   }
   else /* NumByteToWrite > SPI_FLASH_PageSize */
   {
			/*先把整数页都写了*/
     while (NumOfPage--)
     {
    	 exFLASH_WritePageBytes(WriteAddr,pBuffer,  SPI_FLASH_PageSize);
       WriteAddr +=  SPI_FLASH_PageSize;
       pBuffer += SPI_FLASH_PageSize;
     }
			/*若有多余的不满一页的数据，把它写完*/
     exFLASH_WritePageBytes(WriteAddr,pBuffer,  NumOfSingle);
   }
 }
	/* 若地址与 SPI_FLASH_PageSize 不对齐  */
 else
 {
		/* NumByteToWrite < SPI_FLASH_PageSize */
   if (NumOfPage == 0)
   {
			/*当前页剩余的count个位置比NumOfSingle小，一页写不完*/
     if (NumOfSingle > count)
     {
       temp = NumOfSingle - count;
				/*先写满当前页*/
       exFLASH_WritePageBytes(WriteAddr,pBuffer,  count);

       WriteAddr +=  count;
       pBuffer += count;
				/*再写剩余的数据*/
       exFLASH_WritePageBytes(WriteAddr,pBuffer,  temp);
     }
     else /*当前页剩余的count个位置能写完NumOfSingle个数据*/
     {
    	 exFLASH_WritePageBytes(WriteAddr,pBuffer,  NumByteToWrite);
     }
   }
   else /* NumByteToWrite > SPI_FLASH_PageSize */
   {
			/*地址不对齐多出的count分开处理，不加入这个运算*/
     NumByteToWrite -= count;
     NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
     NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

			/* 先写完count个数据，为的是让下一次要写的地址对齐 */
     exFLASH_WritePageBytes(WriteAddr,pBuffer,  count);

			/* 接下来就重复地址对齐的情况 */
     WriteAddr +=  count;
     pBuffer += count;
			/*把整数页都写了*/
     while (NumOfPage--)
     {
    	 exFLASH_WritePageBytes(WriteAddr,pBuffer,  SPI_FLASH_PageSize);
       WriteAddr +=  SPI_FLASH_PageSize;
       pBuffer += SPI_FLASH_PageSize;
     }
			/*若有多余的不满一页的数据，把它写完*/
     if (NumOfSingle != 0)
     {
    	 exFLASH_WritePageBytes(WriteAddr,pBuffer,  NumOfSingle);
     }
   }
 }
}

/*******************************************************************************
 *
 */
void exFLASH_ReadBuffer(uint32_t readAddr, uint8_t* pBuffer, uint16_t dataLength)
{
	exFLASH_WaitForIdle();

	exFLASH_CS_ENABLE();

	/* 发送扇区读命令 */
	exFLASH_WriteReadByte(exFLASH_CMD_READ_DATA);

	/* 发送扇区地址，高位在前 */
	exFLASH_WriteReadByte((readAddr & 0xFF0000) >> 16);
	exFLASH_WriteReadByte((readAddr & 0xFF00) >> 8);
	exFLASH_WriteReadByte(readAddr & 0xFF);

	while (dataLength--)
		*pBuffer++ = exFLASH_WriteReadByte(DUMMY_BYTE);

	exFLASH_CS_DISABLE();

	exFLASH_WaitForIdle();
}



/*******************************************************************************
 * 发送flash 停机
 */
void exFLASH_ModePwrDown(void)
{
	exFLASH_CS_ENABLE();

	exFLASH_WriteReadByte(exFLASH_CMD_POWER_DOWN);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * 发送flash唤醒
 */
void exFLASH_ModeWakeUp(void)
{
	exFLASH_CS_ENABLE();

	exFLASH_WriteReadByte(exFLASH_CMD_RELEASE_POWER_DOWN);

	exFLASH_CS_DISABLE();
}

/*******************************************************************************
 * 模拟量数据格式转换
 */
static void exFLASH_DataFormatConvert(float value, EE_DataFormatEnum format,
							uint8_t* pBuffer)
{
	BOOL negative = FALSE;
	uint16_t temp = 0;

	/* 判断是否为负数 */
	if (value < 0)
		negative = TRUE;

	switch (format)
	{
	case FORMAT_INT:
		temp = (uint16_t)abs((int)(value));
		break;

	case FORMAT_ONE_DECIMAL:
		temp = (uint16_t)abs((int)(value * 10));
		break;

	case FORMAT_TWO_DECIMAL:
		temp = (uint16_t)abs((int)(value * 100));
		break;
	default:
		break;
	}

	*pBuffer = HalfWord_GetHighByte(temp);
	*(pBuffer + 1) = HalfWord_GetLowByte(temp);

	/* 负数则最高位置一 */
	if (negative)
		*pBuffer |= 0x8000;
}

/*******************************************************************************
 *
 */
static void exFLASH_LocationFormatConvert(double value, uint8_t* pBuffer)
{
	BOOL negative = FALSE;
	uint32_t temp;

	if (value < 0)
		negative = TRUE;

	/* 获取整数部分 */
	*pBuffer = abs((int)value);

	temp = (uint32_t)((value - (*pBuffer)) * 1000000);

	if (negative)
		temp |= 0x800000;

	*(pBuffer + 1) = (uint8_t)((temp & 0x00FF0000) >> 16);
	*(pBuffer + 2) = (uint8_t)((temp & 0x0000FF00) >> 8);
	*(pBuffer + 3) = (uint8_t)(temp & 0x000000FF);
}


/*******************************************************************************
 *
 */
void exFLASH_SaveStructInfo(exFLASH_InfoTypedef* saveInfo,
							RT_TimeTypedef*      realTime,
							ANALOG_ValueTypedef* analogValue,
							GPS_LocateTypedef* location)
{
	/* 结构体复位，避免数据出错 */
	memset(saveInfo, 0, sizeof(exFLASH_InfoTypedef));

	/* 获取时钟 */
	saveInfo->realTime.year  = realTime->date.Year;
	saveInfo->realTime.month = realTime->date.Month;
	saveInfo->realTime.day   = realTime->date.Date;
	saveInfo->realTime.hour  = realTime->time.Hours;
	saveInfo->realTime.min   = realTime->time.Minutes;
	/* 为了数据的整齐，将秒置位0 */
	saveInfo->realTime.sec = 0;

	/* 储存电池电量 */
	saveInfo->batteryLevel = analogValue->batVoltage;

	/* 外部电池状态 */
	saveInfo->externalPowerStatus = INPUT_CheckPwrOnStatus();

	exFLASH_LocationFormatConvert(location->latitude,  (uint8_t*)&saveInfo->latitude);
	exFLASH_LocationFormatConvert(location->longitude, (uint8_t*)&saveInfo->longitude);

	/* 模拟数据格式转换 */
	exFLASH_DataFormatConvert(analogValue->temp1, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp1);
	exFLASH_DataFormatConvert(analogValue->humi1, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi1);
	exFLASH_DataFormatConvert(analogValue->temp2, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp2);
	exFLASH_DataFormatConvert(analogValue->humi2, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi2);
	exFLASH_DataFormatConvert(analogValue->temp3, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp3);
	exFLASH_DataFormatConvert(analogValue->humi3, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi3);
	exFLASH_DataFormatConvert(analogValue->temp4, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.temp4);
	exFLASH_DataFormatConvert(analogValue->humi4, ANALOG_VALUE_FORMAT,
			(uint8_t*)&saveInfo->analogValue.humi4);

//	exFLASH_WriteBuffer(EE_FlashInfoSaveAddr, (uint8_t*)&exFLASH_SaveInfo,
//			sizeof(exFLASH_InfoTypedef));
//
//	EE_FlashInfoSaveAddr += sizeof(exFLASH_InfoTypedef);
//	EEPROM_WriteBytes(EE_ADDR_FLASH_INFO_SAVE_ADDR, &EE_FlashInfoSaveAddr,
//			sizeof(EE_FlashInfoSaveAddr));
}

/*******************************************************************************
 *
 */
void exFLASH_ReadStructInfo(exFLASH_InfoTypedef* info)
{
	exFLASH_ReadBuffer(EE_FlashInfoReadAddr, (uint8_t*)info,
			sizeof(exFLASH_InfoTypedef));

	EE_FlashInfoReadAddr += sizeof(exFLASH_InfoTypedef);
	EEPROM_WriteBytes(EE_ADDR_FLASH_INFO_READ_ADDR, &EE_FlashInfoReadAddr,
			sizeof(exFLASH_InfoTypedef));
}
































