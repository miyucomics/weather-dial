#pragma once

#include <stdint.h>

struct WeatherData {
    int offset = 0;
    int month = 0, day = 0, hour = 0, minute = 0;

    float temperature = 0;
    int precip_percent = 0;
    float precip_mm = 0;

    int sunrise = 0;
    int sunset = 0;
    int now = -1;

    uint8_t chances[24] = {};
};
