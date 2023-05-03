#include "USART.h"

USART_StructureTypeDef USART1_struct;
USART_StructureTypeDef USART2_struct;
USARTx_HandleTypeDef USART1_Handle;
USARTx_HandleTypeDef USART2_Handle;

void USART_Init_Common(USARTx_HandleTypeDef *handle, uint32_t BaudRate, uint32_t WordLength, uint32_t StopBits, uint32_t Parity);
ErrorStatus USART_Send_Common(USARTx_HandleTypeDef *handle, uint8_t *pBuffer, uint16_t lengthTX);
uint16_t USART_Receive_Common(USARTx_HandleTypeDef *handle, uint8_t *pBuffer, uint16_t lengthRX);
void USART_IRQHandler_Common(USARTx_HandleTypeDef *handle);
void TIM_IRQHandler_Common(USARTx_HandleTypeDef *handle);


ErrorStatus USARTx_Init(USART_TypeDef *USARTx, uint32_t BaudRate, uint32_t WordLength, uint32_t StopBits, uint32_t Parity)
{
	static bool firstInit = true;
	if (firstInit)
	{
#ifdef RCV_TIM
		extern RCC_ClocksTypeDef rcc_clocks;
		SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
		TIM2->PSC = 1 - 1;
		if ((RCC->CFGR & RCC_CFGR_PPRE1) == RCC_CFGR_PPRE1_DIV1)
			TIM2->ARR = ((rcc_clocks.PCLK1_Frequency) / TIM2_Freq) - 1;
		else
			TIM2->ARR = ((rcc_clocks.PCLK1_Frequency) * 2 / TIM2_Freq) - 1;
		SET_BIT(TIM2->DIER, TIM_DIER_UIE);
		SET_BIT(TIM2->CR1, TIM_CR1_ARPE | TIM_CR1_CEN);
		NVIC_EnableIRQ(TIM2_IRQn);
#endif
		firstInit = false;
	}

	if (USARTx == USART1)
	{
#ifndef USART1_Enable
		assert(ERROR);
#endif
		USART1_Handle.instance = USARTx;
		USART1_Handle.structure = &USART1_struct;
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
		USART_Init_Common(&USART1_Handle, BaudRate, WordLength, StopBits, Parity);
		NVIC_EnableIRQ(USART1_IRQn);
	}
	else if (USARTx == USART2)
	{
#ifndef USART2_Enable
		assert(ERROR);
#endif
		USART2_Handle.instance = USARTx;
		USART2_Handle.structure = &USART2_struct;
		SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
		USART_Init_Common(&USART2_Handle, BaudRate, WordLength, StopBits, Parity);
		NVIC_EnableIRQ(USART2_IRQn);
	}
	else
		return ERROR;	// no match for USARTx

	return SUCCESS;
}

ErrorStatus USART_Send(USART_TypeDef *USARTx, uint8_t *pBuffer, uint16_t lengthTX)
{
	if (lengthTX > WriteBufferSize)
		return ERROR;

	ErrorStatus result = ERROR;
	if (USARTx == USART1)
	{
		result = USART_Send_Common(&USART1_Handle, pBuffer, lengthTX);
	}
	else if (USARTx == USART2)
	{
		result = USART_Send_Common(&USART2_Handle, pBuffer, lengthTX);
	}

	return result;
}

uint16_t USART_Receive(USART_TypeDef *USARTx, uint8_t *pBuffer, uint16_t lengthRX)
{
	if (lengthRX > ReadBufferSize)
		return ERROR;

	uint16_t length = 0;
	if (USARTx == USART1)
	{
		length = USART_Receive_Common(&USART1_Handle, pBuffer, lengthRX);
	}
	else if (USARTx == USART2)
	{
		length = USART_Receive_Common(&USART2_Handle, pBuffer, lengthRX);
	}

	return length;
}

uint16_t Modbus_CRC(uint8_t *pBuffer, uint16_t buffSize)
{
	uint16_t CRC16 = 0xffff;
	uint16_t i, j;
	for (i = 0; i < buffSize; i++)
	{
		CRC16 ^= pBuffer[i];
		for (j = 0; j < 8; j++)
		{
			if (CRC16 & 1)
				CRC16 = (CRC16 >> 1) ^ 0xA001;
			else
				CRC16 = (CRC16 >> 1);
		}
	}

	return CRC16;
}

/**
*	@section Private functions
**/
void USART_Init_Common(USARTx_HandleTypeDef *handle, uint32_t BaudRate, uint32_t WordLength, uint32_t StopBits, uint32_t Parity)
{
	USART_InitTypeDef USART_InitStruct;

	USART_DeInit(handle->instance);
	USART_Cmd(handle->instance, DISABLE);
	/* USART Init Structure */
	USART_InitStruct.USART_BaudRate = BaudRate;
	USART_InitStruct.USART_WordLength = WordLength;
	USART_InitStruct.USART_StopBits = StopBits;
	USART_InitStruct.USART_Parity = Parity;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(handle->instance, &USART_InitStruct);
	/* Interrupts */
	USART_ITConfig(handle->instance, USART_IT_TXE, DISABLE);
	USART_ITConfig(handle->instance, USART_IT_TC, DISABLE);
	USART_ITConfig(handle->instance, USART_IT_RXNE, DISABLE);
	USART_ITConfig(handle->instance, USART_IT_IDLE, DISABLE);
	/* Setting the USART handle structure members to its default state */
	// TX
	handle->structure->dataTX.data = (uint8_t *)calloc(WriteBufferSize, sizeof(uint8_t));
	handle->structure->dataTX.length = 0;
	handle->structure->dataTX.counter = 0;
	handle->structure->dataTX.status = USART_StatusIdle;
	handle->structure->dataTX.timeout = (uint16_t)((double)((1.0 / BaudRate) * (WriteBufferSize * OneByteLength)) * TIM2_Freq) + ReserveTime;
	// RX
	handle->structure->dataRX.data = (uint8_t *)calloc(ReadBufferSize, sizeof(uint8_t));
	handle->structure->dataRX.length = 0;
	handle->structure->dataRX.counter = 0;
	handle->structure->dataRX.status = USART_StatusIdle;
	handle->structure->dataRX.timeout = (uint16_t)((double)((1.0 / BaudRate) * (ReadBufferSize * OneByteLength)) * TIM2_Freq) + ReserveTime;

	USART_Cmd(handle->instance, ENABLE);
}

ErrorStatus USART_Send_Common(USARTx_HandleTypeDef *handle, uint8_t *pBuffer, uint16_t lengthTX)
{
	if (handle->structure->dataTX.status == USART_StatusIdle)
	{
		memcpy(handle->structure->dataTX.data, pBuffer, lengthTX);
		handle->structure->dataTX.length = lengthTX;
		handle->structure->dataTX.counter = 0;
		handle->structure->dataTX.status = USART_StatusProcessing;
		USART_ITConfig(handle->instance, USART_IT_TXE, ENABLE);
	}
	else
		return ERROR; // busy TX

	return SUCCESS;
}

uint16_t USART_Receive_Common(USARTx_HandleTypeDef *handle, uint8_t *pBuffer, uint16_t lengthRX)
{
	if (handle->structure->dataRX.status == USART_StatusIdle)
	{
		handle->structure->dataRX.length = lengthRX;
		handle->structure->dataRX.counter = 0;
		handle->structure->dataRX.status = USART_StatusProcessing;
		USART_ITConfig(handle->instance, USART_IT_RXNE, ENABLE);
	}
	if (handle->structure->dataRX.status == USART_StatusComplete)
	{
		memcpy(pBuffer, handle->structure->dataRX.data, lengthRX);
		memset(handle->structure->dataRX.data, NULL, ReadBufferSize);
		handle->structure->dataRX.status = USART_StatusIdle;

		return handle->structure->dataRX.counter;
	}

	return 0;
}

void USART_IRQHandler_Common(USARTx_HandleTypeDef *handle)
{
	/* Overrun error during debug */
	if (READ_BIT(handle->instance->SR, USART_SR_ORE))
	{
		handle->instance->SR;
		handle->instance->DR;
	}

	/* Transmit */
	if (handle->structure->dataTX.status == USART_StatusProcessing)
	{
		if (READ_BIT(handle->instance->SR, USART_SR_TXE))
		{
			if (handle->structure->dataTX.counter < handle->structure->dataTX.length)
			{
				handle->instance->DR = handle->structure->dataTX.data[handle->structure->dataTX.counter++];
			}
			else
			{
				USART_ITConfig(handle->instance, USART_IT_TXE, DISABLE);
				USART_ITConfig(handle->instance, USART_IT_TC, ENABLE);
			}
		}
		if (READ_BIT(handle->instance->SR, USART_SR_TC))
		{
			CLEAR_BIT(handle->instance->SR, USART_SR_TC);
			USART_ITConfig(handle->instance, USART_IT_TC, DISABLE);
			handle->structure->dataTX.status = USART_StatusIdle;
		}
	}

	/* Receive */
	if (handle->structure->dataRX.status == USART_StatusProcessing)
	{
		if (READ_BIT(handle->instance->SR, USART_SR_RXNE))
		{
			handle->structure->dataRX.data[handle->structure->dataRX.counter++] = (uint8_t)handle->instance->DR;
#ifdef RCV_IDLE
			USART_ITConfig(handle->instance, USART_IT_IDLE, ENABLE);
#elif defined RCV_TIM
			handle->structure->timer.CEN = ENABLE;
#elif defined RCV_CRLF
			if (handle->structure->dataRX.data[handle->structure->dataRX.counter - 1] == *("\n"))
			{
				if (handle->structure->dataRX.data[handle->structure->dataRX.counter - 2] == *("\r"))
				{
					handle->structure->dataRX.status = USART_StatusComplete;
					handle->structure->dataRX.length = handle->structure->dataRX.counter - 2;
					handle->structure->dataRX.data[handle->structure->dataRX.counter - 1] = *("\0");
					handle->structure->dataRX.data[handle->structure->dataRX.counter - 2] = *("\0");
					USART_ITConfig(handle->instance, USART_IT_RXNE, DISABLE);
				}
			}
#endif
#if !defined (RCV_TIM) && !defined (RCV_CRLF)
			if (handle->structure->dataRX.counter >= handle->structure->dataRX.length)
			{
				handle->structure->dataRX.status = USART_StatusComplete;
				handle->structure->dataRX.length = handle->structure->dataRX.counter;
				USART_ITConfig(handle->instance, USART_IT_RXNE, DISABLE);
				USART_ITConfig(handle->instance, USART_IT_IDLE, DISABLE);
			}
#endif
		}
		if (READ_BIT(handle->instance->SR, USART_SR_IDLE))
		{
			handle->instance->SR;
			handle->instance->DR;
			handle->structure->dataRX.status = USART_StatusComplete;
			handle->structure->dataRX.length = handle->structure->dataRX.counter;
			USART_ITConfig(handle->instance, USART_IT_RXNE, DISABLE);
			USART_ITConfig(handle->instance, USART_IT_IDLE, DISABLE);
		}
	}
}

void TIM_IRQHandler_Common(USARTx_HandleTypeDef *handle)
{
	if (handle->structure->timer.CEN == ENABLE)
	{
		if (handle->structure->timer.CNT++ > handle->structure->dataRX.timeout)
		{
			handle->structure->dataRX.status = USART_StatusComplete;
			handle->structure->dataRX.length = handle->structure->dataRX.counter;
			handle->structure->timer.CNT = 0;
			handle->structure->timer.CEN = DISABLE;
		}
	}
}

/**
*	@section IRQ Handlers
**/
#ifdef USART1_Enable
void USART1_IRQHandler(void)
{
	USART_IRQHandler_Common(&USART1_Handle);
}
#endif	/* USART1_Enable */

#ifdef USART2_Enable
void USART2_IRQHandler(void)
{
	USART_IRQHandler_Common(&USART2_Handle);
}
#endif	/* USART2_Enable */

#ifdef RCV_TIM
void TIM2_IRQHandler(void)
{
	TIM2->SR = 0x0000;

#ifdef USART1_Enable
	TIM_IRQHandler_Common(&USART1_Handle);
#endif

#ifdef USART2_Enable
	TIM_IRQHandler_Common(&USART2_Handle);
#endif
}
#endif	/* RCV_TIM */