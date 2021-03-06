## Overview

This sample demos I2C; reading from an attached LSM6DS3 sensor breakout board
when the user presses A.

## How to build the application

See the top level [README](../README.md) for details.

## Hardware Setup

1. Connect UART USB adapter to H3.6 (IO0_TXD) and GND (this can be the same
   GND connected to breadboard for LSM6DS3).
2. Connect the LSM6DS3 and in accordance with the [Connection Diagram](Connection%20Diagram.png).
3. Sideload the application.
4. Press button A to transfer data on the looped-back ISU0 UART.
5. The terminal program which is connected to the M4 debug port should display
   something like the following:

```
'--------------------------------'.
'I2C_RTApp_MT3620_BareMetal!'
'App built on: (date) (time)'
'Connect LSM6DS3, and press button A to read accelerometer.'
'INFO: Acceleration: x, y, z'
'.
```

## Connection Diagram

![Connection Diagram](Connection%20Diagram.png)

Note: we assume the use of MT3620 Reference Development Board (RDB) hardware,
such as the MT3620 development kit from Seeed Studio. Different Azure Sphere
hardware may require different wiring: consult its manufacturer’s
documentation.
