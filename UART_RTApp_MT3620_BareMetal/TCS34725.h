#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "lib/I2CMaster.h"

int32_t TCS34725_CheckWhoAmI(I2CMaster* driver);
int32_t TCS_ReadColorData(I2CMaster* driver, uint16_t* rdata, uint16_t* gdata, uint16_t* bdata);
int32_t TCS_RegRead(I2CMaster* driver, uint8_t addr, void* value, size_t valueSize);
int32_t TCS_RegWrite(I2CMaster* driver, uint8_t addr, uint8_t value);
int32_t TCS_Reset(I2CMaster* driver);

