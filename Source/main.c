#include "main.h"

uint16_t LengthRX1, LengthRX2;
uint8_t BuffRX1[10], BuffRX2[10];


void main()
{
	asm("CPSID i");
	Sys_Init();
	USARTx_Init(USART1, 9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No);
	USARTx_Init(USART2, 9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No);
	asm("CPSIE i");
	USART_Send(USART1, "Hello1", sizeof("Hello1")-1);
	USART_Send(USART2, "Hello2", sizeof("Hello1")-1);

	while(1)
	{
		LengthRX1 = USART_Receive(USART1, BuffRX1, 10);
		LengthRX2 = USART_Receive(USART2, BuffRX2, 10);
	}
}