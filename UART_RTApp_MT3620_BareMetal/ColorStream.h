#pragma once

#include <stdint.h>
#include "sensor_message.h"

#define CS_Message SM_SensorMessage

void CS_Init();
CS_Message *CS_RunIteration();
