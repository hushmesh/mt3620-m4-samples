#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "lib/I2CMaster.h"

bool TCS_CheckWhoAmI(I2CMaster* driver);
int TCS_ReadColorData(I2CMaster* driver, uint16_t* rdata, uint16_t* gdata, uint16_t* bdata);
bool TCS_RegRead(I2CMaster* driver, uint8_t addr, void* value, size_t valueSize);
bool TCS_RegWrite(I2CMaster* driver, uint8_t addr, uint8_t value);
bool TCS_Reset(I2CMaster* driver);

