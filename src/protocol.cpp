#include "protocol.h"
#include "HWCDC.h"
#include "weather.h"

char buffer[256];
int cursor = 0;

void protocol_init() {
    Serial.begin(115200);
}

void protocol_request(int offset_hours) {
    Serial.printf("GET,%d\n", offset_hours);
}

bool parse_data(const char *response, WeatherData &output) {
    WeatherData s;
    int consumed = 0;
    sscanf(response, "DATA,%d,%d-%dT%d:%d,%f,%d,%f,%d,%d,%d,%n", &s.offset, &s.month, &s.day, &s.hour, &s.minute, &s.temperature, &s.precip_percent, &s.precip_mm, &s.sunrise, &s.sunset, &s.now, &consumed);

    const char *head = response + consumed;
    for (int i = 0; i < 24; i++) {
        char *marker;
        long chance = strtol(head, &marker, 10);
        if (marker == head)
            return false;

        s.chances[i] = (uint8_t)chance;
        head = marker;
        if (*head == ';')
            head++;
    }

    output = s;
    return true;
}

ProtocolEvent protocol_poll(WeatherData &data, char *error, size_t error_len) {
    while (Serial.available()) {
        char ch = (char)Serial.read();
        if (ch != '\n') {
            if (cursor < (int)sizeof(buffer) - 1)
                buffer[cursor++] = ch;
            continue;
        }

        buffer[cursor] = '\0';
        cursor = 0;

        if (strncmp(buffer, "DATA,", 5) == 0) {
            if (parse_data(buffer, data))
                return ProtocolEvent::Data;
        } else if (strncmp(buffer, "ERR,", 4) == 0) {
            strlcpy(error, buffer + 4, error_len);
            return ProtocolEvent::Error;
        }
    }

    return ProtocolEvent::NoResponse;
}
