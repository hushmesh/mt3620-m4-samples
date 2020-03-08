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

#include "ColorStream.h"
#include "debug.h"


static const int buttonAGpio = 12;
static const int buttonPressCheckPeriodMs = 10;
static void HandleButtonTimerIrq(GPT *);
static void HandleButtonTimerIrqDeferred(void);

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
            //DEBUG("RTCore: Hello world!\r\n\r\n");
            DEBUG("RTCore: OH FDF UEAH Hello world!\r\n\r\n");
        }

        prevState = newState;
    }
}

static void HandleUartIsu0RxIrqDeferred(void)
{
    /*
    uintptr_t avail = UART_ReadAvailable(debug);
    if (avail == 0) {
        DEBUG("ERROR: UART received interrupt for zero bytes.\r\n");
        return;
    }

    if (avail >= 65536) {
        // Avoid handling large amounts of data as this could cause stack issues.
        DEBUG("ERROR: UART received too many bytes.\r\n");
        return;
    }

    uint8_t buffer[avail];
    if (UART_Read(debug, buffer, avail) != ERROR_NONE) {
        DEBUG("ERROR: Failed to read ");
        UART_PrintUInt(debug, avail);
        DEBUG(" bytes from UART.\r\n");
        return;
    }

    DEBUG("UART received ");
    UART_PrintUInt(debug, avail);
    DEBUG(" bytes: \'");
    UART_Write(debug, buffer, avail);
    DEBUG("\'.\r\n");
    */
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

        if (node) {
            (*node->cb)();
        }
    } while (node);
}

_Noreturn void RTCoreMain(void)
{
    VectorTableInit();
    CPUFreq_Set(26000000);

    debug_init(MT3620_UNIT_ISU1);
    
    DEBUG("--------------AAA---------------\r\n");
    DEBUG("UART_RTApp_MT3620_BareMetal\r\n");
    DEBUG("App built on: " __DATE__ " " __TIME__ "\r\n");

    CS_Init();

    DEBUG("Install a loopback header on ISU0, and press button A to send a message.\r\n");

    GPIO_ConfigurePinForInput(buttonAGpio);

    // Setup GPT1 to poll for button press
    if (!(buttonTimeout = GPT_Open(MT3620_UNIT_GPT1, 1000, GPT_MODE_REPEAT))) {
        DEBUG("ERROR: Opening timer\r\n");
    }
    int32_t error;

    if ((error = GPT_StartTimeout(buttonTimeout, buttonPressCheckPeriodMs,
                                  GPT_UNITS_MILLISEC, &HandleButtonTimerIrq)) != ERROR_NONE) {
        DEBUG("ERROR: Starting timer (%ld)\r\n", error);
    }

    for (;;) {
        __asm__("wfi");
        InvokeCallbacks();
    }
}
