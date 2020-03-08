#pragma once

#define SENSOR_MAX_STRING_LENGTH 64

typedef struct _sensor_message {
    int string1Length;
    char string1[SENSOR_MAX_STRING_LENGTH + 1];

    int string2Length;
    char string2[SENSOR_MAX_STRING_LENGTH + 1];
} SM_SensorMessage;