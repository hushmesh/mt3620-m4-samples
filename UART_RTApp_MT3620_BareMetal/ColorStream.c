#include "ColorStream.h"

#include <stdlib.h>

#include "lib/I2CMaster.h"

#include "TCS34725.h"
#include "debug.h"

static I2CMaster* driver = NULL;

typedef struct _cs_sensors {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} cs_sensors;

static cs_sensors current;
//static CS_Message message;

static int32_t cs_ReadSensor(void);


void CS_Init() {
    driver = I2CMaster_Open(MT3620_UNIT_ISU0);
    if (!driver) {
        DEBUG("ERROR: I2C initialisation failed\r\n");
    }

    I2CMaster_SetBusSpeed(driver, I2C_BUS_SPEED_FAST);

    int32_t error = TCS34725_CheckWhoAmI(driver);
    if (error != ERROR_NONE) {
        DEBUG("Who Am I Failed");
        return;
    }

    error = TCS34725_Reset(driver);
    if (error != ERROR_NONE) {
        DEBUG("Reset Failed");
    }

    current.red = 0;
    current.green = 0;
    current.blue = 0;

    cs_ReadSensor();
}

int32_t cs_ReadSensor() {
    int32_t error = TCS34725_ReadColorData(driver, &current.red, &current.green, &current.blue);
    if (error == ERROR_NONE) {
        DEBUG("Red: %d\r\nGreen: %d\r\nBlue: %d\r\n", current.red, current.green, current.blue);
    }
    return error;
}

CS_Message* CS_RunIteration() {
    if (cs_ReadSensor() != ERROR_NONE) {
        DEBUG("ERROR: Unable to read sensor");
        return NULL;
    }

    return NULL;
}