#include <M5Unified.h>
#include <cstdint>

#include "encoder.h"
#include "protocol.h"
#include "render.h"

ViewState view;
bool waiting = false;
uint32_t last_request_time = 0;
long prev_encoder = 0;
long offset = 0;

void show_status(const char *message) {
    view.has_sample = false;
    strlcpy(view.status_buffer, message, sizeof(view.status_buffer));
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);

    encoder_init();
    protocol_init();
    render_init();
}

void loop() {
    M5.update();
    uint32_t now = millis();

    if (M5.BtnA.wasClicked()) {
        offset = 0;

        protocol_request(0);
        waiting = true;
        last_request_time = now;
        M5.Speaker.tone(2000, 100);
    }

    long encoder = get_encoder_value();
    if (encoder != prev_encoder) {
        offset += encoder - prev_encoder;
        prev_encoder = encoder;

        protocol_request(offset);
        waiting = true;
        last_request_time = now;
        M5.Speaker.tone(4000, 8);
    }

    if (now - last_request_time > 1000) {
        protocol_request(offset);
        waiting = true;
        last_request_time = now;
    }

    char error[40];
    switch (protocol_poll(view.sample, error, sizeof(error))) {
    case ProtocolEvent::Data:
        view.has_sample = true;
        waiting = false;
        render_frame(view);
        break;
    case ProtocolEvent::Error:
        show_status(error);
        waiting = false;
        render_frame(view);
        break;
    case ProtocolEvent::NoResponse:
        break;
    }

    render_frame(view);
    delay(20);
}
