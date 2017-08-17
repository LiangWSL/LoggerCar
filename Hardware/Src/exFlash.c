#include "exFlash.h"

#include "gps.h"

/*******************************************************************************
 * @brief  ����FLASH����
 * @param  SectorAddr��Ҫ������������ַ
 * @retval ��
 */
void SPI_FLASH_SectorErase(uint32_t SectorAddr)
{
	/* ����FLASHдʹ������ */
	SPI_FLASH_WriteEnable();
	SPI_FLASH_WaitForWriteEnd();

	/* �������� */
	/* ѡ��FLASH: CS�͵�ƽ */
	SPI_FLASH_CS_LOW();

	/* ������������ָ��*/
	SPI_FLASH_SendByte(W25X_SectorErase);

	/*���Ͳ���������ַ�ĸ�λ*/
	SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(SectorAddr & 0xFF);

	/* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
	SPI_FLASH_CS_HIGH();

	/* �ȴ��������*/
	SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
 * @brief  ����FLASH��������Ƭ����
 * @param  ��
 * @retval ��
 */
void SPI_FLASH_BulkErase(void)
{
	/* ����FLASHдʹ������ */
	SPI_FLASH_WriteEnable();

	/* ���� Erase */
	/* ѡ��FLASH: CS�͵�ƽ */
	SPI_FLASH_CS_LOW();

	/* �����������ָ��*/
	SPI_FLASH_SendByte(W25X_ChipErase);

	/* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
	SPI_FLASH_CS_HIGH();

	/* �ȴ��������*/
	SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
 * @brief  ��FLASH��ҳд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
 * @param	pBuffer��Ҫд�����ݵ�ָ��
 * @param WriteAddr��д���ַ
 * @param  NumByteToWrite��д�����ݳ��ȣ�����С�ڵ���SPI_FLASH_PerWritePageSize
 * @retval ��
 */
void SPI_FLASH_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	/* ����FLASHдʹ������ */
	SPI_FLASH_WriteEnable();

	/* ѡ��FLASH: CS�͵�ƽ */
	SPI_FLASH_CS_LOW();

	/* дҳдָ��*/
	SPI_FLASH_SendByte(W25X_PageProgram);

	/*����д��ַ*/
	SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(WriteAddr & 0xFF);

	if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
	{
		NumByteToWrite = SPI_FLASH_PerWritePageSize;
	}

	/* д������*/
	while (NumByteToWrite--)
	{
		/* ���͵�ǰҪд����ֽ����� */
		SPI_FLASH_SendByte(*pBuffer);
		/* ָ����һ�ֽ����� */
		pBuffer++;
	}

	/* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
	SPI_FLASH_CS_HIGH();

	/* �ȴ�д�����*/
	SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
 * @brief  ��FLASHд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
 * @param	pBuffer��Ҫд�����ݵ�ָ��
 * @param  WriteAddr��д���ַ
 * @param  NumByteToWrite��д�����ݳ���
 * @retval ��
 */
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	/*mod�������࣬��writeAddr��SPI_FLASH_PageSize��������������AddrֵΪ0*/
	Addr = WriteAddr % SPI_FLASH_PageSize;

	/*��count������ֵ���պÿ��Զ��뵽ҳ��ַ*/
	count = SPI_FLASH_PageSize - Addr;
	/*�����Ҫд��������ҳ*/
	NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	/*mod�������࣬�����ʣ�಻��һҳ���ֽ���*/
	NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

	/* Addr=0,��WriteAddr �պð�ҳ���� aligned  */
	if (Addr == 0)
	{
		/* NumByteToWrite < SPI_FLASH_PageSize */
		if (NumOfPage == 0)
		{
			SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
		}
		else /* NumByteToWrite > SPI_FLASH_PageSize */
		{
			/*�Ȱ�����ҳ��д��*/
			while (NumOfPage--)
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
			SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
		}
	}
	/* ����ַ�� SPI_FLASH_PageSize ������  */
	else
	{
		/* NumByteToWrite < SPI_FLASH_PageSize */
		if (NumOfPage == 0)
		{
			/*��ǰҳʣ���count��λ�ñ�NumOfSingleС��һҳд����*/
			if (NumOfSingle > count)
			{
				temp = NumOfSingle - count;
				/*��д����ǰҳ*/
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);

				WriteAddr +=  count;
				pBuffer += count;
				/*��дʣ�������*/
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
			}
			else /*��ǰҳʣ���count��λ����д��NumOfSingle������*/
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
			}
		}
		else /* NumByteToWrite > SPI_FLASH_PageSize */
		{
			/*��ַ����������count�ֿ������������������*/
			NumByteToWrite -= count;
			NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
			NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

			/* ��д��count�����ݣ�Ϊ��������һ��Ҫд�ĵ�ַ���� */
			SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);

			/* ���������ظ���ַ�������� */
			WriteAddr +=  count;
			pBuffer += count;
			/*������ҳ��д��*/
			while (NumOfPage--)
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
			if (NumOfSingle != 0)
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
			}
		}
	}
}

/*******************************************************************************
 * @brief  ��ȡFLASH����
 * @param 	pBuffer���洢�������ݵ�ָ��
 * @param   ReadAddr����ȡ��ַ
 * @param   NumByteToRead����ȡ���ݳ���
 * @retval ��
 */
void SPI_FLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	/* ѡ��FLASH: CS�͵�ƽ */
	SPI_FLASH_CS_LOW();

	/* ���� �� ָ�� */
	SPI_FLASH_SendByte(W25X_ReadData);

	/* ���� �� ��ַ��λ */
	SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
	SPI_FLASH_SendByte(ReadAddr & 0xFF);

	/* ��ȡ���� */
	while (NumByteToRead--) /* while there is data to be read */
	{
		/* ��ȡһ���ֽ�*/
		*pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
		/* ָ����һ���ֽڻ����� */
		pBuffer++;
	}

	/* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * @brief  ��ȡFLASH ID
 * @param 	��
 * @retval FLASH ID
 */
uint32_t SPI_FLASH_ReadID(void)
{
	uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

	/* ��ʼͨѶ��CS�͵�ƽ */
	SPI_FLASH_CS_LOW();

	/* ����JEDECָ���ȡID */
	SPI_FLASH_SendByte(W25X_JedecDeviceID);

	/* ��ȡһ���ֽ����� */
	Temp0 = SPI_FLASH_SendByte(Dummy_Byte);
	Temp1 = SPI_FLASH_SendByte(Dummy_Byte);
	Temp2 = SPI_FLASH_SendByte(Dummy_Byte);

	/* ֹͣͨѶ��CS�ߵ�ƽ */
	SPI_FLASH_CS_HIGH();

	/*�����������������Ϊ�����ķ���ֵ*/
	Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

	return Temp;
}

/*******************************************************************************
 * @brief  ��ȡFLASH Device ID
 * @param 	��
 * @retval FLASH Device ID
 */
uint32_t SPI_FLASH_ReadDeviceID(void)
{
	uint32_t Temp = 0;

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "RDID " instruction */
	SPI_FLASH_SendByte(W25X_DeviceID);
	SPI_FLASH_SendByte(Dummy_Byte);
	SPI_FLASH_SendByte(Dummy_Byte);
	SPI_FLASH_SendByte(Dummy_Byte);

	/* Read a byte from the FLASH */
	Temp = SPI_FLASH_SendByte(Dummy_Byte);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	return Temp;
}

/*******************************************************************************
* Function Name  : SPI_FLASH_StartReadSequence
* Description    : Initiates a read data byte (READ) sequence from the Flash.
*                  This is done by driving the /CS line low to select the device,
*                  then the READ instruction is transmitted followed by 3 bytes
*                  address. This function exit and keep the /CS line low, so the
*                  Flash still being selected. With this technique the whole
*                  content of the Flash is read with a single READ instruction.
* Input          : - ReadAddr : FLASH's internal address to read from.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_StartReadSequence(uint32_t ReadAddr)
{
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Read from Memory " instruction */
	SPI_FLASH_SendByte(W25X_ReadData);

	/* Send the 24-bit address of the address to read from -------------------*/
	/* Send ReadAddr high nibble address byte */
	SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	/* Send ReadAddr medium nibble address byte */
	SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
	/* Send ReadAddr low nibble address byte */
	SPI_FLASH_SendByte(ReadAddr & 0xFF);
}


/*******************************************************************************
 * @brief  ʹ��SPI��ȡһ���ֽڵ�����
 * @param  ��
 * @retval ���ؽ��յ�������
 */
uint8_t SPI_FLASH_ReadByte(void)
{
	return (SPI_FLASH_SendByte(Dummy_Byte));
}

/*******************************************************************************
 * @brief  ʹ��SPI����һ���ֽڵ�����
 * @param  byte��Ҫ���͵�����
 * @retval ���ؽ��յ�������
 */
uint8_t SPI_FLASH_SendByte(uint8_t byte)
{
	while(!__HAL_SPI_GET_FLAG(&exFLASH_SPI, SPI_FLAG_TXE));
	exFLASH_SPI.Instance->DR = byte;

	while(!__HAL_SPI_GET_FLAG(&exFLASH_SPI, SPI_FLAG_RXNE));
	return exFLASH_SPI.Instance->DR;
}

/*******************************************************************************
 * @brief  ��FLASH���� дʹ�� ����
 * @param  none
 * @retval none
 */
void SPI_FLASH_WriteEnable(void)
{
	/* ͨѶ��ʼ��CS�� */
	SPI_FLASH_CS_LOW();

	/* ����дʹ������*/
	SPI_FLASH_SendByte(W25X_WriteEnable);

	/*ͨѶ������CS�� */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * @brief  �ȴ�WIP(BUSY)��־����0�����ȴ���FLASH�ڲ�����д�����
 * @param  none
 * @retval none
 */
void SPI_FLASH_WaitForWriteEnd(void)
{
	uint8_t FLASH_Status = 0;

	/* ѡ�� FLASH: CS �� */
	SPI_FLASH_CS_LOW();

	/* ���� ��״̬�Ĵ��� ���� */
	SPI_FLASH_SendByte(W25X_ReadStatusReg);

	/* ��FLASHæµ����ȴ� */
	do
	{
		/* ��ȡFLASHоƬ��״̬�Ĵ��� */
		FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);
	}
	while ((FLASH_Status & WIP_Flag) == SET);  /* ����д���־ */

	/* ֹͣ�ź�  FLASH: CS �� */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * �������ģʽ
 */
void SPI_Flash_PowerDown(void)
{
	/* ͨѶ��ʼ��CS�� */
	SPI_FLASH_CS_LOW();

	/* ���� ���� ���� */
	SPI_FLASH_SendByte(W25X_PowerDown);

	/*ͨѶ������CS�� */
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
 * ����
 */
void SPI_Flash_WAKEUP(void)
{
	/*ѡ�� FLASH: CS �� */
	SPI_FLASH_CS_LOW();

	/* ���� �ϵ� ���� */
	SPI_FLASH_SendByte(W25X_ReleasePowerDown);

	/* ֹͣ�ź� FLASH: CS �� */
	SPI_FLASH_CS_HIGH();
}
