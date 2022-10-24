#include "USART.h"

extern RCC_ClocksTypeDef rcc_clocks;
#ifdef UART1_Enable
USART_InitTypeDef USART1_InitStruct;
UART_StructureTypeDef UART1;
#endif
#ifdef UART1_Enable
USART_InitTypeDef USART2_InitStruct;
UART_StructureTypeDef UART2;
#endif
uint16_t ReadTimeout;


void USARTx_Init(USART_TypeDef *USARTx, uint32_t BaudRate, uint32_t WordLength, uint32_t StopBits, uint32_t Parity)
{
	if (USARTx == USART1)
	{
		USART_DeInit(USART1);
		USART_Cmd(USART1, DISABLE);
		// RCC
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
#ifdef STM32F10x
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
#endif
#ifdef STM32F4xx
		SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
#endif
		// Pin config
#ifdef UART1_RS485
		/*PA8 - CK*/
#ifdef STM32F10x
		SET_BIT(GPIOA->CRH, GPIO_CRH_MODE8);	// Output mode, max speed 50 MHz
		CLEAR_BIT(GPIOA->CRH, GPIO_CRH_CNF8);	// General purpose output push-pull
#endif
#ifdef STM32F4xx
		MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER8, GPIO_MODER_MODER8_0);	// 01: General purpose output mode
		CLEAR_BIT(GPIOA->OTYPER, GPIO_OTYPER_ODR_8);	// 0: Output push-pull (reset state)
		MODIFY_REG(GPIOA->OSPEEDR, GPIO_OSPEEDER_OSPEEDR8, GPIO_OSPEEDER_OSPEEDR8_1);	// 10: High speed
#endif
#endif /*_UART1_RS485_*/
		/*PA9 - TX*/
#ifdef STM32F10x
		SET_BIT(GPIOA->CRH, GPIO_CRH_MODE9);	// Output mode, max speed 50 MHz
		MODIFY_REG(GPIOA->CRH,GPIO_CRH_CNF9, GPIO_CRH_CNF9_1);	// Alternate function output Push-pull
#endif
#ifdef STM32F4xx
		MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER9, GPIO_MODER_MODER9_1);	// 10: Alternate function mode
		MODIFY_REG(GPIOA->AFR[1], 0xF0, 7 << 4);	// 0111: AF7
#endif
		/*PA10 - RX*/
#ifdef STM32F10x
		CLEAR_BIT(GPIOA->CRH, GPIO_CRH_MODE10);	// Input mode (reset state)
		MODIFY_REG(GPIOA->CRH,GPIO_CRH_CNF10, GPIO_CRH_CNF10_1);	// Input with pull-up / pull-down
		SET_BIT(GPIOA->ODR, GPIO_ODR_ODR10);	// pull-up
#endif
#ifdef STM32F4xx
		MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER10, GPIO_MODER_MODER10_1);	// 10: Alternate function mode
		MODIFY_REG(GPIOA->AFR[1], 0xF00, 7 << 8);	// 0111: AF7
#endif
		// USART Init Structure
		USART1_InitStruct.USART_BaudRate = BaudRate;
		USART1_InitStruct.USART_WordLength = WordLength;
		USART1_InitStruct.USART_StopBits = StopBits;
		USART1_InitStruct.USART_Parity = Parity;
		USART1_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART1_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		// Interrupts
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		NVIC_EnableIRQ(USART1_IRQn);
		// UART handling structure filling
		/* TX */
		UART1.DataTX.Data = (uint8_t*) malloc(sizeof(uint8_t) * WriteBufferSize);
		memset(UART1.DataTX.Data, NULL, WriteBufferSize);
		UART1.DataTX.Length = 0;
		UART1.DataTX.Counter = 0;
		UART1.DataTX.Status = Idle;
		/* RX */
		UART1.DataRX.Data = (uint8_t*) malloc(sizeof(uint8_t) * ReadBufferSize);
		memset(UART1.DataRX.Data, NULL, ReadBufferSize);
		UART1.DataRX.Length = 0;
		UART1.DataRX.Counter = 0;
		UART1.DataRX.Status = Idle;
#ifdef EngageTIM2
		// Read timeout timer (F = 20kHz, T = 50uS)
		ReadTimeout = (uint16_t)((double)((1.0/BaudRate) * (ReadBufferSize * OneByteLength)) * 1000000 / 50) + ReserveTime;
		uint32_t UART_CLK_APB1 = rcc_clocks.PCLK1_Frequency;
		SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
		TIM2->PSC = 1 - 1;
		if ((RCC->CFGR & RCC_CFGR_PPRE1) == RCC_CFGR_PPRE1_DIV1) TIM2->ARR = (UART_CLK_APB1 / 20000) - 1;
		else TIM2->ARR = (UART_CLK_APB1 * 2 / 20000) - 1;
		SET_BIT(TIM2->DIER, TIM_DIER_UIE);
		SET_BIT(TIM2->CR1, TIM_CR1_ARPE | TIM_CR1_CEN);
		NVIC_EnableIRQ(TIM2_IRQn);
#endif
#ifdef UART1_RS485
		CLEAR_BIT(GPIOA->ODR, GPIO_ODR_ODR8);	// Receiver enable
#endif
		USART_Init(USART1, &USART1_InitStruct);
		USART_Cmd(USART1, ENABLE);
	}

	if (USARTx == USART2)
	{
		USART_DeInit(USART2);
		USART_Cmd(USART2, DISABLE);
		// RCC
		SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
#ifdef STM32F10x
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
#endif
#ifdef STM32F4xx
		SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
#endif
		// Pin config
#ifdef UART2_RS485
		/*PA4 - CK*/
#ifdef STM32F10x
		SET_BIT(GPIOA->CRL, GPIO_CRL_MODE4);	// Output mode, max speed 50 MHz
		CLEAR_BIT(GPIOA->CRL, GPIO_CRL_CNF4);	// General purpose output push-pull
#endif
#ifdef STM32F4xx
		MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER4, GPIO_MODER_MODER4_0);	// 01: General purpose output mode
		CLEAR_BIT(GPIOA->OTYPER, GPIO_OTYPER_ODR_4);	// 0: Output push-pull (reset state)
		MODIFY_REG(GPIOA->OSPEEDR, GPIO_OSPEEDER_OSPEEDR4, GPIO_OSPEEDER_OSPEEDR4_1);	// 10: High speed
#endif
#endif /*_UART2_RS485_*/
		/*PA2 - TX*/
#ifdef STM32F10x
		SET_BIT(GPIOA->CRL, GPIO_CRL_MODE2);	// Output mode, max speed 50 MHz
		MODIFY_REG(GPIOA->CRL,GPIO_CRL_CNF2, GPIO_CRL_CNF2_1);	// Alternate function output Push-pull
#endif
#ifdef STM32F4xx
		MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER2, GPIO_MODER_MODER2_1);	// 10: Alternate function mode
		MODIFY_REG(GPIOA->AFR[0], 0xF00, 7 << 8);	// 0111: AF7
#endif
		/*PA3 - RX*/
#ifdef STM32F10x
		CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE3);	// Input mode (reset state)
		MODIFY_REG(GPIOA->CRL,GPIO_CRL_CNF3, GPIO_CRL_CNF3_1);	// Input with pull-up / pull-down
		SET_BIT(GPIOA->ODR, GPIO_ODR_ODR3);	// pull-up
#endif
#ifdef STM32F4xx
		MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER3, GPIO_MODER_MODER3_1);	// 10: Alternate function mode
		MODIFY_REG(GPIOA->AFR[0], 0xF000, 7 << 12);	// 0111: AF7
#endif
		// USART Init Structure
		USART2_InitStruct.USART_BaudRate = BaudRate;
		USART2_InitStruct.USART_WordLength = WordLength;
		USART2_InitStruct.USART_StopBits = StopBits;
		USART2_InitStruct.USART_Parity = Parity ;
		USART2_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART2_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		// Interrupts
		USART_ITConfig(USART2, USART_IT_TC, ENABLE);
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
		NVIC_EnableIRQ(USART2_IRQn);
		// UART handling structure filling
		/* TX */
		UART2.DataTX.Data = (uint8_t*) malloc(sizeof(uint8_t) * WriteBufferSize);
		memset(UART2.DataTX.Data, NULL, WriteBufferSize);
		UART2.DataTX.Length = 0;
		UART2.DataTX.Counter = 0;
		UART2.DataTX.Status = Idle;
		/* RX */
		UART2.DataRX.Data = (uint8_t*) malloc(sizeof(uint8_t) * ReadBufferSize);
		memset(UART2.DataRX.Data, NULL, ReadBufferSize);
		UART2.DataRX.Length = 0;
		UART2.DataRX.Counter = 0;
		UART2.DataRX.Status = Idle;
#ifdef EngageTIM2
		// Read timeout timer (F = 20kHz, T = 50uS)
		ReadTimeout = (uint16_t)((double)((1.0/BaudRate) * (ReadBufferSize * 10)) * 1000000 / 50) + ReserveTime;
		uint32_t UART_CLK_APB1 = rcc_clocks.PCLK1_Frequency;
		SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
		TIM2->PSC = 1 - 1;
		if ((RCC->CFGR & RCC_CFGR_PPRE1) == RCC_CFGR_PPRE1_DIV1) TIM2->ARR = (UART_CLK_APB1 / 20000) - 1;
		else TIM2->ARR = (UART_CLK_APB1 * 2 / 20000) - 1;
		SET_BIT(TIM2->DIER, TIM_DIER_UIE);
		SET_BIT(TIM2->CR1, TIM_CR1_ARPE | TIM_CR1_CEN);
		NVIC_EnableIRQ(TIM2_IRQn);
#endif
#ifdef UART2_RS485
		CLEAR_BIT(GPIOA->ODR, GPIO_ODR_ODR4);	// Receiver enable
#endif
		USART_Init(USART2, &USART2_InitStruct);
		USART_Cmd(USART2, ENABLE);
	}
}

bool USART_Send(USART_TypeDef *USARTx, uint8_t *pBuffer, uint16_t LengthTX)
{
	if (USARTx == USART1)
	{
		if (UART1.DataTX.Status == Idle)
		{
#ifdef UART1_RS485
			SET_BIT(GPIOA->ODR, GPIO_ODR_ODR8);	// Driver enable
#endif
			UART1.DataTX.Status = Processing;
			UART1.DataTX.Counter = 0;
			UART1.DataTX.Length = LengthTX;
			if (LengthTX <= WriteBufferSize)
			{
				memcpy(UART1.DataTX.Data, pBuffer, LengthTX);
				USARTx->DR = UART1.DataTX.Data[UART1.DataTX.Counter++];
			}
			else return ERROR;
		}
	}

	else if (USARTx == USART2)
	{
		if (UART2.DataTX.Status == Idle)
		{
#ifdef UART2_RS485
			SET_BIT(GPIOA->ODR, GPIO_ODR_ODR4);	// Driver enable
#endif
			UART2.DataTX.Status = Processing;
			UART2.DataTX.Counter = 0;
			UART2.DataTX.Length = LengthTX;
			if (LengthTX <= WriteBufferSize)
			{
				memcpy(UART2.DataTX.Data, pBuffer, LengthTX);
				USARTx->DR = UART2.DataTX.Data[UART2.DataTX.Counter++];
			}
			else return ERROR;
		}
	}
	return SUCCESS;
}

uint16_t USART_Receive(USART_TypeDef *USARTx, uint8_t *pBuffer, uint16_t LengthRX)
{
	if (USARTx == USART1)
	{
		if (UART1.DataRX.Status == Complete)
		{
			uint16_t DataLength = UART1.DataRX.Length;
			if (DataLength > LengthRX) DataLength = LengthRX;
#ifndef WipeBufferEachRead
			memcpy(pBuffer, UART1.DataRX.Data, DataLength);
#else
			memcpy(pBuffer, UART1.DataRX.Data, LengthRX);
#endif
			memset(UART1.DataRX.Data, NULL, ReadBufferSize);
			UART1.DataRX.Counter = 0;
			UART1.DataRX.Status = Idle;
			return DataLength;
		}
	}

	else if (USARTx == USART2)
	{
		if (UART2.DataRX.Status == Complete)
		{
			uint16_t DataLength = UART2.DataRX.Length;
			if (DataLength > LengthRX) DataLength = LengthRX;
#ifndef WipeBufferEachRead
			memcpy(pBuffer, UART2.DataRX.Data, DataLength);
#else
			memcpy(pBuffer, UART2.DataRX.Data, LengthRX);
#endif
			memset(UART2.DataRX.Data, NULL, ReadBufferSize);
			UART2.DataRX.Counter = 0;
			UART2.DataRX.Status = Idle;
			return DataLength;
		}
	}
	return 0;
}

uint16_t Modbus_CRC(uint8_t *pBuffer, uint16_t BuffSize)
{
	uint16_t CRC16 = 0xffff;
	uint16_t i, j;
	for(i = 0; i < BuffSize; i++)
	{
		CRC16 ^= pBuffer[i];
		for (j = 0; j < 8; j++)
		{
			if (CRC16 & 1) CRC16 = (CRC16 >> 1) ^ 0xA001;
			else CRC16 = (CRC16 >> 1);
		}
	}
	return CRC16;
}


/**
*	@section IRQ Handlers
**/
#ifdef UART1_Enable
void USART1_IRQHandler(void)
{
	if (READ_BIT(USART1->SR, USART_SR_RXNE))	// Receive
	{
		if (UART1.DataRX.Status != Complete)
		{
			if (UART1.DataRX.Counter < ReadBufferSize)
			{
				UART1.DataRX.Status = Processing;
#ifdef EngageTIM2
				/* Read timeout timer startup */
				UART1.TIM.CNT = 0;
				UART1.TIM.CEN = ENABLE;
#endif
				UART1.DataRX.Data[UART1.DataRX.Counter++] = (uint8_t)USART1->DR;
#if !defined EngageTIM2 && !defined CR_LF_Break
				if (UART1.DataRX.Counter >= ReadBufferSize) UART1.DataRX.Status = Complete, UART1.DataRX.Length = UART1.DataRX.Counter;
#endif
#ifdef CR_LF_Break
				if (UART1.DataRX.Data[UART1.DataRX.Counter-1] == *("\n"))
				{
					if (UART1.DataRX.Data[UART1.DataRX.Counter-2] == *("\r"))
					{
						UART1.DataRX.Status = Complete;
						UART1.DataRX.Length = UART1.DataRX.Counter-2;
						UART1.DataRX.Data[UART1.DataRX.Counter-1] = *("\0");
						UART1.DataRX.Data[UART1.DataRX.Counter-2] = *("\0");
					}
				}
#endif
			}
			else CLEAR_BIT(USART1->SR, USART_SR_RXNE);
		}
		else CLEAR_BIT(USART1->SR, USART_SR_RXNE);
	}

	if (READ_BIT(USART1->SR, USART_SR_TC))	// Transmit
	{
		CLEAR_BIT(USART1->SR, USART_SR_TC);
		if (UART1.DataTX.Counter < UART1.DataTX.Length) USART1->DR = UART1.DataTX.Data[UART1.DataTX.Counter++];
		else
		{
			UART1.DataTX.Status = Idle;
#ifdef UART1_RS485
			CLEAR_BIT(GPIOA->ODR, GPIO_ODR_ODR8);	// Receiver enable
#endif
		}
	}
}
#endif

#ifdef UART2_Enable
void USART2_IRQHandler(void)
{
	if (READ_BIT(USART2->SR, USART_SR_RXNE))	// Receive
	{
		if (UART2.DataRX.Status != Complete)
		{
			if (UART2.DataRX.Counter < ReadBufferSize)
			{
				UART2.DataRX.Status = Processing;
#ifdef EngageTIM2
				/* Read timeout timer startup */
				UART2.TIM.CNT = 0;
				UART2.TIM.CEN = ENABLE;
#endif
				UART2.DataRX.Data[UART2.DataRX.Counter++] = (uint8_t)USART2->DR;
#if !defined EngageTIM2 && !defined CR_LF_Break
				if (UART2.DataRX.Counter >= ReadBufferSize) UART2.DataRX.Status = Complete, UART2.DataRX.Length = UART2.DataRX.Counter;
#endif
#ifdef CR_LF_Break
				if (UART2.DataRX.Data[UART2.DataRX.Counter-1] == *("\n"))
				{
					if (UART2.DataRX.Data[UART2.DataRX.Counter-2] == *("\r"))
					{
						UART2.DataRX.Status = Complete;
						UART2.DataRX.Length = UART2.DataRX.Counter-2;
						UART2.DataRX.Data[UART2.DataRX.Counter-1] = *("\0");
						UART2.DataRX.Data[UART2.DataRX.Counter-2] = *("\0");
					}
				}
#endif
			}
			else CLEAR_BIT(USART2->SR, USART_SR_RXNE);
		}
		else CLEAR_BIT(USART2->SR, USART_SR_RXNE);
	}

	if (READ_BIT(USART2->SR, USART_SR_TC))	// Transmit
	{
		CLEAR_BIT(USART2->SR, USART_SR_TC);
		if (UART2.DataTX.Counter < UART2.DataTX.Length) USART2->DR = UART2.DataTX.Data[UART2.DataTX.Counter++];
		else
		{
			UART2.DataTX.Status = Idle;
#ifdef UART2_RS485
			CLEAR_BIT(GPIOA->ODR, GPIO_ODR_ODR4);	// Receiver enable
#endif
		}
	}
}
#endif

#ifdef EngageTIM2
void TIM2_IRQHandler(void)
{
	TIM2->SR = 0x0000;

	if (UART1.TIM.CEN == ENABLE)
	{
		if (UART1.TIM.CNT++ > ReadTimeout)
		{
			UART1.TIM.CEN = DISABLE;
			UART1.DataRX.Status = Complete;
			UART1.DataRX.Length = UART1.DataRX.Counter;
		}
	}

	if (UART2.TIM.CEN == ENABLE)
	{
		if (UART2.TIM.CNT++ > ReadTimeout)
		{
			UART2.TIM.CEN = DISABLE;
			UART2.DataRX.Status = Complete;
			UART2.DataRX.Length = UART2.DataRX.Counter;
		}
	}
}
#endif