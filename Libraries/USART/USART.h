/**
******************************************************************************
* @file    USART.h
* @author  FT9R
* @version V0.1
* @date    21.10.2022
* @brief   This is the library, aimed to handle with USART interface
******************************************************************************
*/
#ifndef USART_H
#define USART_H

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
*	@def User definitions
*/
/* MCU */
// #define STM32F10x
#define STM32F4xx
/* UARTx */
#define UART1_Enable
#define UART2_Enable
/* RS485 */
// #define UART1_RS485
// #define UART2_RS485
#define EngageTIM2
//#define CR_LF_Break
#define WipeBufferEachRead
#define ReadBufferSize            10
#define WriteBufferSize           10
/**
*	@def Maintenance definitions
*/
#define OneByteLength							10	// StartBit + Data8Bit + StopBit
#define ReserveTime								2		// Extra time for reading process = x*50uS
#define Idle											0
#define Processing								1
#define Complete									2
#if !defined EngageTIM2 && !defined CR_LF_Break
#warning "Unsafe way to receive the data!"
#warning "TIM2 is not engaged so ensure that FrameLength = ReadBufferSize = Const to complete receiving process"
#endif
#if !defined EngageTIM2 && defined CR_LF_Break
#warning "Unsafe way to receive the data!"
#warning "You have to end the frame with \r + \n to complete receiving process. Checksum is inserted before \r if needed"
#endif
#if defined EngageTIM2 && defined CR_LF_Break
#error "Incompatible defines "EngageTIM2 + CR_LF_Break", choose only desired one"
#endif
#if defined STM32F10x && !defined STM32F4xx
#include "stm32f10x_usart.h"
#elif defined STM32F4xx && !defined STM32F10x
#include "stm32f4xx_usart.h"
#else
#error "Choose the only one target MCU first"
#endif

typedef struct UART_Instance
{
	uint8_t *Data;
	uint16_t Length;
	uint16_t Counter;
	uint8_t Status;
} UART_Instance_TypeDef;

typedef struct
{
	bool CEN;
	uint16_t CNT;
} UART_TimerTypeDef;

typedef struct
{
	UART_Instance_TypeDef DataTX;
	UART_Instance_TypeDef DataRX;
	UART_TimerTypeDef TIM;
} UART_StructureTypeDef;

/**
*	@brief	Initialization function
*	@param	USARTx: Determines which interface will be initialized
*					@arg	USART1
*					@arg	USART2
*	@param	BaudRate: configures the USART communication baud rate
*	@param	WordLength: Specifies the number of data bits transmitted or received in a frame
*					@arg	USART_WordLength_8b
*					@arg	USART_WordLength_9b
*	@param	StopBits: Specifies the number of stop bits transmitted
*					@arg	USART_StopBits_1
*					@arg	USART_StopBits_0_5
*					@arg	USART_StopBits_2
*					@arg	USART_StopBits_1_5
*	@param	Parity: Specifies the parity mode
*					@arg	USART_Parity_No
*					@arg	USART_Parity_Even
*					@arg	USART_Parity_Odd
*	@return	None
*/
void USARTx_Init(USART_TypeDef *USARTx, uint32_t BaudRate, uint32_t WordLength, uint32_t StopBits, uint32_t Parity);

/**
*	@brief	USART common TX routine
*	@param	USARTx: Determines which interface will be used
*					@arg	USART1
*					@arg	USART2
*	@param	pBuffer: pointer to a buffer that contains the data to be sent
*	@param	LengthTX: bytes amount to be transmitted from the source buffer
*	@return	FrameSize & WriteBufferSize comparison status
*/
bool USART_Send(USART_TypeDef *USARTx, uint8_t *pBuffer, uint16_t LengthTX);

/**
*	@brief	USART common RX routine
*	@param	USARTx: Determines which interface will be used
*					@arg	USART1
*					@arg	USART2
*	@param	pBuffer: pointer to a buffer that will contain the received data
*	@param	LengthRX: bytes amount to be writed to the destination Buffer
*	@return	Received data size in bytes
*/
uint16_t USART_Receive(USART_TypeDef *USARTx, uint8_t *pBuffer, uint16_t LengthRX);

/**
*	@brief	CheckSum calculating routine
*	@param	pBuffer: pointer to a buffer which contains the data
*	@param	BuffSize: buffer size in bytes
*	@return	CRC-16
*/
uint16_t Modbus_CRC(uint8_t *pBuffer, uint16_t BuffSize);

#endif