#include "TCS34725.h"

#include "Debug.h"

#define TCS_REG_WHO_AM_I 0x92
#define TCS_WHO_AM_I 0x44
#define TCS_ADDRESS 0x29
#define TCS_REG_ATIME 0x81
#define TCS_REG_AGAIN 0x8F
#define TCS_REG_ENABLE 0x80

int32_t TCS34725_CheckWhoAmI(I2CMaster* driver) {
	uint8_t ident;

	int32_t error = TCS34725_RegRead(driver, TCS_REG_WHO_AM_I, &ident, sizeof(ident));
	if (error != ERROR_NONE) {
		return error;
	}

	if (ident != TCS_WHO_AM_I) {
		return ERROR_UNSUPPORTED;
	}

	return ERROR_NONE;
}

int32_t TCS34725_RegRead(I2CMaster* driver, uint8_t addr, void* value, size_t valueSize) {
	return I2CMaster_WriteThenReadSync(
		driver, 
		TCS_ADDRESS, 
		&addr, 
		sizeof(uint8_t), 
		value, 
		valueSize
	);
}

int32_t TCS34725_RegWrite(I2CMaster* driver, uint8_t addr, uint8_t value) {
	const uint8_t cmd[] = { addr, value };
	size_t cmdsize = sizeof(cmd);
	return I2CMaster_WriteSync(driver, TCS_ADDRESS, &cmd, cmdsize);
}

int32_t TCS34725_Reset(I2CMaster* driver) {
	int32_t error = ERROR_NONE;

	error = TCS34725_RegWrite(driver, TCS_REG_ATIME, 0xFD);
	if (error != ERROR_NONE) {
		DEBUG("Failed to write aTime");
		return error;
	}

	error = TCS34725_RegWrite(driver, TCS_REG_AGAIN, 0x03);
	if (error != ERROR_NONE) {
		DEBUG("Failed to write aGain");
		return error;
	}

	error = TCS34725_RegWrite(driver, TCS_REG_ENABLE, 0x01);
	if (error != ERROR_NONE) {
		DEBUG("Failed to write enable");
		return error;
	}

	error = TCS34725_RegWrite(driver, TCS_REG_ENABLE, 0x03);
	if (error != ERROR_NONE) {
			DEBUG("Failed to write enable twice");
			return error;
	}

	uint8_t v;
	error = TCS34725_RegRead(driver, TCS_REG_ATIME, &v, sizeof(v));
	if (error != ERROR_NONE) {
		return error;
	}
	DEBUG("aTime register = 0x%02x\r\n", v);

	error = TCS34725_RegRead(driver, TCS_REG_AGAIN, &v, sizeof(v));
	if (error != ERROR_NONE) {
		return error;
	}
	DEBUG("aGain register = 0x%02x\r\n", v);

	error = TCS34725_RegRead(driver, TCS_REG_ENABLE, &v, sizeof(v));
	if (error != ERROR_NONE) {
		return error;
	}
	DEBUG("enable register = 0x%02x\r\n", v);

	return ERROR_NONE;
}

#define TCS_REG_RED 0x96
#define TCS_REG_GREEN 0x98
#define TCS_REG_BLUE 0x9A

int32_t TCS34725_ReadColorData(I2CMaster *driver, uint16_t* rdata, uint16_t* gdata, uint16_t* bdata) {
	int32_t error = ERROR_NONE;
	error = TCS34725_RegRead(driver, TCS_REG_RED, rdata, sizeof(uint16_t));
	if (error != ERROR_NONE) {
		return error;
	}

	error = TCS34725_RegRead(driver, TCS_REG_GREEN, gdata, sizeof(uint16_t));
	if (error != ERROR_NONE) {
		return error;
	}

	error = TCS34725_RegRead(driver, TCS_REG_BLUE, bdata, sizeof(uint16_t));
	if (error != ERROR_NONE) {
		return error;
	}

	return ERROR_NONE;
}