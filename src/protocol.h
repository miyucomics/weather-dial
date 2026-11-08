#pragma once

#include "weather.h"
#include <stddef.h>

enum class ProtocolEvent {
    NoResponse,
    Error,
    Data,
};

void protocol_init();
void protocol_request(int offset_hours);
ProtocolEvent protocol_poll(WeatherData &data, char *error, size_t error_len);
