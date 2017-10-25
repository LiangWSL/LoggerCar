#include "../Inc/lowPwr.h"
#include "gprs.h"

/******************************************************************************/
static FunctionalState LOWPWR_KeyStatusCheck(void);
static void LOWPWR_EnterStandbyMode(void);

/*******************************************************************************
 *
 */
void LOWPWR_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* ����ǵ͹���ģʽ����������Ҫ���������� */
	if (__HAL_PWR_GET_FLAG(PWR_FLAG_WU) == TRUE)
	{
		/* ������Ч�����½������ģʽ */
		if (DISABLE == LOWPWR_KeyStatusCheck())
		{
			/* �������ģʽ */
			LOWPWR_EnterStandbyMode();
		}
	}

	/* ������и�λ��־λ */
	__HAL_RCC_CLEAR_RESET_FLAGS();
}

/*******************************************************************************
 *
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_PIN_0 == GPIO_Pin)
	{
		/* ������������³���3s */
		if (ENABLE == LOWPWR_KeyStatusCheck())
		{
			/* �������ģʽ */
			LOWPWR_EnterStandbyMode();
		}
	}
}

/*******************************************************************************
 * function:��ⰴ��״̬
 * return��enable = ����������
 *        disable = ������Ч
 */
static FunctionalState LOWPWR_KeyStatusCheck(void)
{
	uint8_t pressCnt = 0, releaseCnt = 0;

	/* ���PA0�����ϵĵ�ƽΪ����50 * 100ms = 5s�ĸߵ�ƽ */
	while (1)
	{
		if (HAL_GPIO_ReadPin(WKUP_GPIO_Port, WKUP_Pin) == GPIO_PIN_SET)
		{
			pressCnt++;
			releaseCnt = 0;

			GPRS_PWR_CTRL_ENABLE();
			if (pressCnt >= 50)
			{
				return ENABLE;
			}
		}
		else
		{
			pressCnt = 0;
			releaseCnt++;
			/* ��⵽�����ͷ�5�� */
			if (releaseCnt > 5)
			{
				GPRS_PWR_CTRL_DISABLE();
				return DISABLE;
			}
		}
		osDelay(100);
	}
}

/*******************************************************************************
 *
 */
static void LOWPWR_EnterStandbyMode(void)
{
	/* ��λ��Դ */
//	__HAL_RCC_PWR_FORCE_RESET();

	/* ʹ�ܵ�Դ�ӿ�ʱ�� */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* ������ѱ�־λ */
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

	HAL_PWR_EnterSTANDBYMode();
}
