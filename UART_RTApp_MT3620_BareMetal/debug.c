#include "debug.h"

UART* DEBUG_UART = 0;

void DEBUG_Init(Platform_Unit unit) {
    if (DEBUG_UART != 0) {
        /* already initialized, do nothing */
        return;
    }
    DEBUG_UART = UART_Open(unit, 115200, UART_PARITY_NONE, 1, 0);
}
