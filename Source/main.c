#include "main.h"

uint16_t lengthRX1, lengthRX2;
uint8_t buffRX1[ReadBufferSize], buffRX2[ReadBufferSize];


void main()
{
	asm("CPSID i");
	Sys_Init();
	USARTx_Init(USART1, 115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No);
	USARTx_Init(USART2, 115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No);
	IO_Init();
	asm("CPSIE i");
	USART_Send(USART1, "Hello1\r\n", sizeof("Hello1\r\n")-1);
	USART_Send(USART2, "Hello2\r\n", sizeof("Hello2\r\n")-1);

	while(1)
	{
		lengthRX1 = USART_Receive(USART1, buffRX1, ReadBufferSize);
		if (lengthRX1 > 0)
		{
			printf("buffRX1: %s\r\n", buffRX1);
		}
		lengthRX2 = USART_Receive(USART2, buffRX2, ReadBufferSize);
		if (lengthRX2 > 0)
		{
			printf("buffRX2: %s\r\n", buffRX2);
		}
	}
}