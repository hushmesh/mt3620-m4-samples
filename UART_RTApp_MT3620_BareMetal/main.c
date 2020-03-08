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
#include "lib/GPT.h"

#include "ColorStream.h"
#include "Debug.h"

static const int COLORSTREAM_PERIOD_IN_MS = 1000;
static const int COLORSTREAM_TIMER_SPEED_IN_HZ = 10000;

// isTimeToPollSensor is modified in the timer thread, so we mark
// as volatile to make sure the main thread checks it every time
static volatile int isTimeToPollSensor = 0;

// cbTimeoutInTimerThread flips a flag to show the main thread that 
// it's time to read the sensor. Minimizing the work done in the timer
// thread helps keep the timer accurate.
static void cbTimeoutInTimerThread(GPT *handle)
{
    uint32_t prevBasePriority = NVIC_BlockIRQs(); // make sure no other interrupts occur
    isTimeToPollSensor = 1;
    NVIC_RestoreIRQs(prevBasePriority);
}

static void WriteSensorMessageToHighLevelApp(CS_Message *msg) {
    DEBUG("1: %s\r\n", msg->string1);
    DEBUG("2: %s\r\n", msg->string2);
}

_Noreturn void RTCoreMain(void)
{
    VectorTableInit(); // allows blocking interrupts in timer thread
    CPUFreq_Set(26000000); // don't know why this is needed! -- rsc

    DEBUG_Init(MT3620_UNIT_ISU1);   
    DEBUG("--------------AAA---------------\r\n");
    DEBUG("UART_RTApp_MT3620_BareMetal\r\n");
    DEBUG("App built on: " __DATE__ " " __TIME__ "\r\n");

    CS_Init();

    // timer is a clock on which to hang periodic events
    GPT* timer = GPT_Open(
        MT3620_UNIT_GPT1, 
        COLORSTREAM_TIMER_SPEED_IN_HZ, 
        GPT_MODE_REPEAT
    );

    // the timer callback (aka Timeout) periodically fires whenever the given timer
    // measures the given time period has elapsed
    int32_t error = GPT_StartTimeout(
        timer, 
        COLORSTREAM_PERIOD_IN_MS, 
        GPT_UNITS_MILLISEC, 
        &cbTimeoutInTimerThread
    );
    if (error != ERROR_NONE) {
        DEBUG("ERROR: Unable to start timer\r\n");
    }

    int sm;

    for (;;) {
        __asm__("wfi");
        if (isTimeToPollSensor == 1) {
            CS_Message *msg = CS_RunIteration(&sm);
            if (msg != NULL) {
                WriteSensorMessageToHighLevelApp(msg);
            }
            isTimeToPollSensor = 0;
        }
    }
}
