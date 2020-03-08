#include "ColorStream.h"

#include "lib/I2CMaster.h"

#include "TCS34725.h"
#include "debug.h"

static I2CMaster* driver = NULL;

void CS_Init() {
    driver = I2CMaster_Open(MT3620_UNIT_ISU0);
    if (!driver) {
        DEBUG("ERROR: I2C initialisation failed\r\n");
    }

    I2CMaster_SetBusSpeed(driver, I2C_BUS_SPEED_FAST);

    if (!TCS34725_CheckWhoAmI(driver)) {
        DEBUG("Who Am I Failed");
    }

    if (!TCS_Reset(driver)) {
        DEBUG("Reset Failed");
    }

    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;

    if (!TCS_ReadColorData(driver, &red, &green, &blue)) {
        DEBUG("Failed to read colors");
    }

    DEBUG("Red: %d\r\nGreen: %d\r\nBlue: %d\r\n", red, green, blue);
}