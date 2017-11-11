#include "../Inc/public.h"

/*******************************************************************************
 * @brief �ַ���ת��������
 */
void str2numb(uint8_t* pNumb, uint8_t* pStr, uint8_t size)
{
	while(size--)
	{
		*pNumb++ = (*pStr++) - '0';
	}
}

/*******************************************************************************
 * @brief ��BCD��ת����ASCII��,һ��BCD�����ת��2��ASCII��
 * @pASCII:ת����ascii����ָ��
 * @pBCD��BCD��ָ��
 * @size��Ҫת����BCD�ֽ���
 */
void BCD2ASCII(char* pASCII, uint8_t* pBCD, uint8_t size)
{
	uint8_t i;

	for (i = 0; i < size; i++)
	{
		*(pASCII + (i * 2))     = (*(pBCD + i) / 16) + '0';
		*(pASCII + (i * 2) + 1) = (*(pBCD + i) % 16) + '0';
	}
}

/*******************************************************************************
 * @brief ��ASCII��ת����BCD�룬����ASCII��ת����һ��BCD��
 * @param pBCD:����BCD���ָ��
 * @param pASCII��ASCII��ָ��
 * @param size��ASCII�������2��ASCII��ת����һ��BCD��
 */
void ASCII2BCD(uint8_t* pBCD, char* pASCII, uint8_t size)
{
	uint8_t i;

	for (i = 0; i < size;)
	{
		*(pBCD + (i / 2)) = (*(pASCII + i) - '0') * 16 + (*(pASCII + i + 1) - '0');
		i += 2;
	}
}

/*******************************************************************************
 * function:����ֵת����BCD��
 * @pHEX:��ֵָ��
 * @pBCD��BCD���ָ��
 * @size��Ҫת������ֵ����
 */
void HEX2BCD(uint8_t* pBCD, uint8_t* pHEX, uint8_t size)
{
	uint8_t i;

	for (i = 0; i < size; i++)
	{
		*(pBCD + i) = (*(pHEX + i) / 10 * 16 + *(pHEX + i) % 10);
	}
}

/*******************************************************************************
 * @brief ��16���Ƶ���ֵת����ASCII��һ��16������ת����2��ASCII��
 * @param pASCII��ASCIIָ��
 * @param pHEX��16������ֵָ��
 * @param size��hex�ֽ���
 */
void HEX2ASCII(char* pASCII, uint8_t* pHEX, uint8_t size)
{
	uint8_t i;

	for (i = 0; i < size; i++)
	{
		*(pASCII + (i * 2))     = (*(pHEX + i) / 10) + '0';
		*(pASCII + (i * 2) + 1) = (*(pHEX + i) % 10) + '0';
	}
}

/*******************************************************************************
 * @brief ASCII��ת����HEX
 * 			ע�⣺ֻ֧��ת��1��2��3���ַ�
 */
void ASCII2HEX(uint8_t* pHEX, uint8_t* pASCII, uint8_t size)
{
	switch (size)
	{
	case 1:
		*pHEX = (*pASCII) - '0';
		break;
	case 2:
		*pHEX = ((*pASCII) - '0') * 10 + (*(pASCII + 1) - '0');
		break;
	case 3:
		*pHEX = ((*pASCII) - '0') * 100 + (*(pASCII + 1) - '0') * 10 + (*(pASCII + 2) - '0');
		break;
	default:
		break;
	}
}

/******************************************************************************/
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&DEBUG_UART, (uint8_t *)&ch, 1, 0xffff);
	return ch;
}

int fgetc(FILE * f)
{
	uint8_t ch = 0;
	HAL_UART_Receive(&DEBUG_UART, &ch, 1, 0xffff);
	return ch;
}

/******************************************************************************/
void DebugPrintf(char* str)
{
	printf(str);
}

/*******************************************************************************
 *
 */
HAL_StatusTypeDef UART_DMAIdleConfig(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
	uint32_t *tmp;

	if((pData == NULL ) || (Size == 0))
	{
	  return HAL_ERROR;
	}

	huart->pRxBuffPtr = pData;
	huart->RxXferSize = Size;

	huart->ErrorCode = HAL_UART_ERROR_NONE;

	/* Enable the DMA channel */
	tmp = (uint32_t*)&pData;
	HAL_DMA_Start(huart->hdmarx, (uint32_t)&huart->Instance->DR, *(uint32_t*)tmp, Size);

	/* Enable the DMA transfer for the receiver request by setting the DMAR bit
	   in the UART CR3 register */
	SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);

	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);

	return HAL_OK;
}


