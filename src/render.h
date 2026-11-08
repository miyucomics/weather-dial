#pragma once

#include "weather.h"

struct ViewState {
    WeatherData sample;
    bool has_sample = false;
    char status_buffer[40] = "finding daemon";
};

void render_init();
void render_frame(const ViewState &view);
