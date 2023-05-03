# STM32 USART handling library
Current library uses SPL functions on the initialization stage
and CMSIS definitions inside main code in order to get access to MCU registers directly.
This approach provides the best performance and saves CPU free time.

## Data receiving process
How does the MCU know when a data frame sent from the transceiver is ready to be read?
At the moment, the project implements 4 methods for obtaining data:
> * Receive until line idle status is detected
> * Receive until RX timeout for ReadBufferSize is elapsed
> * Receive until "\r\n" escape sequence is detected
> * Receive until RX buffer is full

## Integration with your project
1. Copy `USART.h` and `USART.c` from PROJ_DIR\Libraries\USART to your project
2. Write the line `#include "USART.h"` in your main header file 
3. In `USART.h` uncomment USARTx_Enable needed by your project
4. Choose only one way to receive the data
5. Define maximum read and write buffer sizes

## Compatibility
The library is tested on:
* BluePill board
* STM32F407VGT6 (STM32F4XX_M board)
* STM32F107RCT6