#include "TCS34725.h"

#include "lib/UART.h"
#include "lib/Print.h"

extern UART* debug;

#define TCS_REG_WHO_AM_I 0x92
#define TCS_WHO_AM_I 0x44
#define TCS_ADDRESS 0x29
#define TCS_REG_ATIME 0x81
#define TCS_REG_AGAIN 0x8F
#define TCS_REG_ENABLE 0x80

bool TCS_CheckWhoAmI(I2CMaster* driver) {
	if (!driver) {
		return false;
	}

	uint8_t ident;

	if (TCS_RegRead(driver, TCS_REG_WHO_AM_I, &ident) == false) {
		return false;
	}

	return (ident == TCS_WHO_AM_I);
}

bool TCS_RegRead(I2CMaster* driver, uint8_t addr, uint8_t* value) {
	uint8_t v;
	int32_t status = I2CMaster_WriteThenReadSync(
		driver, TCS_ADDRESS, &addr, sizeof(addr), &v, sizeof(v));
	if (value) {
		*value = v;
	}
	return (status == ERROR_NONE);
}

bool TCS_RegWrite(I2CMaster* driver, uint8_t addr, uint8_t value) {
	const uint8_t cmd[] = { addr, value };
	return (I2CMaster_WriteSync(driver, TCS_ADDRESS, &cmd, sizeof(cmd)) == ERROR_NONE);
}

bool TCS_Reset(I2CMaster* driver) {
	if (!driver) {
		return false;
	}

	if (!TCS_RegWrite(driver, TCS_REG_ATIME, 0xFD)) {
		UART_Print(debug, "Failed to write aTime");
		return false;
	}

	if (!TCS_RegWrite(driver, TCS_REG_AGAIN, 0x03)) {
		UART_Print(debug, "Failed to write aGain");
		return false;
	}

	if (!TCS_RegWrite(driver, TCS_REG_ENABLE, 0x01)) {
		UART_Print(debug, "Failed to write enable");
		return false;
	}

	if (!TCS_RegWrite(driver, TCS_REG_ENABLE, 0x03)) {
		UART_Print(debug, "Failed to write enable twice");
		return false;
	}

	uint8_t v;
	if (!TCS_RegRead(driver, TCS_REG_ATIME, &v)) {
		return false;	
	}
	UART_Printf(debug, "aTime register = 0x%02x\n", v);

	if (!TCS_RegRead(driver, TCS_REG_AGAIN, &v)) {
		return false;
	}
	UART_Printf(debug, "aGain register = 0x%02x\n", v);

	if (!TCS_RegRead(driver, TCS_REG_ENABLE, &v)) {
		return false;
	}
	UART_Printf(debug, "enable register = 0x%02x\n", v);

	return true;
}