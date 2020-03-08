#pragma once

/* debug.h creates a macro, "DEBUG" which can be used to send a printf-like
   message to the given ISU using the UART API.

   You need to hook up a special USB cable. Connect the cable's RX to the dev board's
   TX. (For ISU1, pin 13. For ISU0, pin 5). Connect the cable's GND to the board's
   GND. Then plug the USB into a linux computer and run:

   $ sudo minicom -D /dev/ttyUSB0 -b 115200 
   
   The tty device might be different. */

#include "lib/UART.h"
#include "lib/Print.h"
#include "lib/Platform.h"

extern UART* DEBUG_UART;
void DEBUG_Init(Platform_Unit unit);

#define DEBUG(...) UART_Printf(DEBUG_UART, __VA_ARGS__)
