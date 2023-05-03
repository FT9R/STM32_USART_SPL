#ifndef USART_H
#define USART_H

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "main.h"	// target MCU header + SPL def

/**
*	@def User definitions
*/
/* USARTx EN */
#define USART1_Enable
#define USART2_Enable
/* The only one way to receive */
#define RCV_IDLE	// Receive until line idle status is detected
//#define RCV_TIM	// Receive until RX timeout for ReadBufferSize is elapsed
//#define RCV_CRLF	// Receive until "\r\n" escape sequence is detected
//#define RCV_FULL	// Receive until RX buffer is full
/* RX/TX buf size trim */
#define ReadBufferSize	20
#define WriteBufferSize	20

/**
*	@def Maintenance definitions
*/
#define OneByteLength	10	// StartBit + 8xDataBit + StopBit
#define ReserveTime	2	// Extra time = x*50uS
#define TIM2_Freq	20000
typedef enum
{
	USART_StatusIdle,
	USART_StatusProcessing,
	USART_StatusComplete
} USART_StatusDef;

typedef struct
{
	uint8_t *data;
	uint16_t length;
	uint16_t counter;
	uint8_t status;
	uint16_t timeout;
} USART_Instance_TypeDef;

typedef struct
{
	bool CEN;
	uint16_t CNT;
} USART_TimerTypeDef;

typedef struct
{
	USART_Instance_TypeDef dataTX;
	USART_Instance_TypeDef dataRX;
	USART_TimerTypeDef timer;
} USART_StructureTypeDef;

typedef struct
{
	USART_TypeDef *instance;
	USART_StructureTypeDef *structure;
} USARTx_HandleTypeDef;

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
*	@return	Initialization result status
*/
ErrorStatus USARTx_Init(USART_TypeDef *USARTx, uint32_t BaudRate, uint32_t WordLength, uint32_t StopBits, uint32_t Parity);

/**
*	@brief	USART common TX routine
*	@param	USARTx: Determines which interface will be used
*					@arg	USART1
*					@arg	USART2
*	@param	pBuffer: pointer to a buffer that contains the data to be sent
*	@param	lengthTX: bytes amount to be transmitted from the source buffer
*	@return	TX start status
*/
ErrorStatus USART_Send(USART_TypeDef *USARTx, uint8_t *pBuffer, uint16_t lengthTX);

/**
*	@brief	USART common RX routine
*	@param	USARTx: Determines which interface will be used
*					@arg	USART1
*					@arg	USART2
*	@param	pBuffer: pointer to a buffer that will contain the received data
*	@param	lengthRX: bytes amount to be writed to the destination Buffer
*	@return	Received data size in bytes
*/
uint16_t USART_Receive(USART_TypeDef *USARTx, uint8_t *pBuffer, uint16_t lengthRX);

/**
*	@brief	CheckSum calculating routine
*	@param	pBuffer: pointer to a buffer which contains the data
*	@param	buffSize: buffer size in bytes
*	@return	CRC-16
*/
uint16_t Modbus_CRC(uint8_t *pBuffer, uint16_t buffSize);

#endif