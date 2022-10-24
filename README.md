# STM32 USART handling library
Current library uses SPL functions on the initialization stage
and CMSIS definitions in order to get access to MCU registers directly.
It provides the best performance and saves CPU free time.

## Data receiving process
How does the MCU know when a data frame sent from the transceiver is ready to be read?
At the moment, the project implements 3 methods for obtaining data:
* using a timeout which is calculated based on the ReadBufferSize defined by the user
* receiving until ReadBufferSize is filled 
* receiving until combination of \r + \n is reached

Take in your mind that 2 last methods are unsafe

## Integration with your project
1. Copy `USART.h` and `USART.c` from PROJ_DIR\Libraries\USART to your project
2. Write the line `#include "USART.h"` in your main header file 
3. In `USART.h` uncomment the definition of desired CPU you are going using to
4. Uncomment USARTx needed by your project
5. Determine the combination of `#define EngageTIM2` and `#define CR_LF_Break`
6. Define read and write buffer sizes
7. Comment `#define WipeBufferEachRead` if you want the bufferRX to be filled sequentially

## Example
It contains not just a library but the entire project.
If you have STM32F407VG based board and IAR EW for Arm 9.20.1 then just open UART SPL.eww.
Else you can analyse `main.c` and find out how to use the library in your case

## Compatibility
The library is tested on BluePill board and also on STM32F407VGT6(STM32F4XX_M board)
