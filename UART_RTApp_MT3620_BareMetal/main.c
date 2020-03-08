/* ColorStream_RTApp_MT3620_BareMetal
   Continually monitors light sensor for ColorStream input. When found, decodes it and
   sends result to high-level app.

   TODO
   1. Write TCS34725 library
   2. Implement timer callbacks from main loop
   3. Implement ColorStream algorithm
   4. Connect to partner_hlapp */

/* Modified from: UART_RTApp_MT3620_BareMetal 
   Copyright (c) Codethink Ltd. All rights reserved.
   Licensed under the MIT License. */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "lib/CPUFreq.h"
#include "lib/VectorTable.h"
#include "lib/NVIC.h"
#include "lib/GPIO.h"
#include "lib/GPT.h"
#include "lib/UART.h"
#include "lib/Print.h"
#include "lib/I2CMaster.h"

#include "TCS34725.h"

static const int buttonAGpio = 12;
static const int buttonPressCheckPeriodMs = 10;
static void HandleButtonTimerIrq(GPT *);
static void HandleButtonTimerIrqDeferred(void);

static I2CMaster *driver = NULL;
UART      *debug  = NULL;
static GPT       *buttonTimeout = NULL;

typedef struct CallbackNode {
    bool enqueued;
    struct CallbackNode *next;
    void (*cb)(void);
} CallbackNode;

static void EnqueueCallback(CallbackNode *node);

static void HandleButtonTimerIrq(GPT *handle)
{
    (void)handle;
    static CallbackNode cbn = {.enqueued = false, .cb = HandleButtonTimerIrqDeferred};
    EnqueueCallback(&cbn);
}

static void HandleButtonTimerIrqDeferred(void)
{
    // Assume initial state is high, i.e. button not pressed.
    static bool prevState = true;
    bool newState;
    GPIO_Read(buttonAGpio, &newState);

    if (newState != prevState) {
        bool pressed = !newState;
        if (pressed) {
            UART_Print(debug, "RTCore: Hello world!");
        }

        prevState = newState;
    }
}

static void HandleUartIsu0RxIrqDeferred(void)
{
    uintptr_t avail = UART_ReadAvailable(debug);
    if (avail == 0) {
        UART_Print(debug, "ERROR: UART received interrupt for zero bytes.\r\n");
        return;
    }

    if (avail >= 65536) {
        // Avoid handling large amounts of data as this could cause stack issues.
        UART_Print(debug, "ERROR: UART received too many bytes.\r\n");
        return;
    }

    uint8_t buffer[avail];
    if (UART_Read(debug, buffer, avail) != ERROR_NONE) {
        UART_Print(debug, "ERROR: Failed to read ");
        UART_PrintUInt(debug, avail);
        UART_Print(debug, " bytes from UART.\r\n");
        return;
    }

    UART_Print(debug, "UART received ");
    UART_PrintUInt(debug, avail);
    UART_Print(debug, " bytes: \'");
    UART_Write(debug, buffer, avail);
    UART_Print(debug, "\'.\r\n");
}


static void HandleUartIsu0RxIrq(void) {
    static CallbackNode cbn = { .enqueued = false, .cb = HandleUartIsu0RxIrqDeferred };
    EnqueueCallback(&cbn);
}

static CallbackNode *volatile callbacks = NULL;

static void EnqueueCallback(CallbackNode *node)
{
    uint32_t prevBasePri = NVIC_BlockIRQs();
    if (!node->enqueued) {
        CallbackNode *prevHead = callbacks;
        node->enqueued = true;
        callbacks = node;
        node->next = prevHead;
    }
    NVIC_RestoreIRQs(prevBasePri);
}

static void InvokeCallbacks(void)
{
    CallbackNode *node;
    do {
        uint32_t prevBasePri = NVIC_BlockIRQs();
        node = callbacks;
        if (node) {
            node->enqueued = false;
            callbacks = node->next;
        }
        NVIC_RestoreIRQs(prevBasePri);

        if (node) {
            (*node->cb)();
        }
    } while (node);
}

_Noreturn void RTCoreMain(void)
{
    VectorTableInit();
    CPUFreq_Set(26000000);

    //debug = UART_Open(MT3620_UNIT_UART_DEBUG, 115200, UART_PARITY_NONE, 1, NULL);
    debug = UART_Open(MT3620_UNIT_ISU1, 115200, UART_PARITY_NONE, 1, NULL);
    UART_Print(debug, "--------------------------------\r\n");
    UART_Print(debug, "UART_RTApp_MT3620_BareMetal\r\n");
    UART_Print(debug, "App built on: " __DATE__ " " __TIME__ "\r\n");

    driver = I2CMaster_Open(MT3620_UNIT_ISU0);
    if (!driver) {
        UART_Print(debug, "ERROR: I2C initialisation failed\r\n");
    }

    I2CMaster_SetBusSpeed(driver, I2C_BUS_SPEED_FAST);

    if (!TCS_CheckWhoAmI(driver)) {
        UART_Print(debug, "Who Am I Failed");
    }

    if (!TCS_Reset(driver)) {
        UART_Print(debug, "Reset Failed");
    }

    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;

    if (!TCS_ReadColorData(driver, &red, &green, &blue)) {
        UART_Print(debug, "Failed to read colors");
    }

    UART_Printf(debug, "Red: %d\r\nGreen: %d\r\nBlue: %d\r\n", red, green, blue);

    UART_Print(debug,
        "Install a loopback header on ISU0, and press button A to send a message.\r\n");

    GPIO_ConfigurePinForInput(buttonAGpio);

    // Setup GPT1 to poll for button press
    if (!(buttonTimeout = GPT_Open(MT3620_UNIT_GPT1, 1000, GPT_MODE_REPEAT))) {
        UART_Print(debug, "ERROR: Opening timer\r\n");
    }
    int32_t error;

    if ((error = GPT_StartTimeout(buttonTimeout, buttonPressCheckPeriodMs,
                                  GPT_UNITS_MILLISEC, &HandleButtonTimerIrq)) != ERROR_NONE) {
        UART_Printf(debug, "ERROR: Starting timer (%ld)\r\n", error);
    }

    for (;;) {
        __asm__("wfi");
        InvokeCallbacks();
    }
}
