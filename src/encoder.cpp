// encoder.h and encoder.cpp adapted from https://github.com/m5stack/M5Dial/issues/8

#include "encoder.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"
#include <cstdint>

void encoder_init() {
    pcnt_config_t enc_config = {
        .pulse_gpio_num = GPIO_NUM_40,
        .ctrl_gpio_num = GPIO_NUM_41,

        .lctrl_mode = PCNT_MODE_KEEP,
        .hctrl_mode = PCNT_MODE_REVERSE,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_DEC,

        .counter_h_lim = INT16_MAX,
        .counter_l_lim = INT16_MIN,

        .unit = PCNT_UNIT_0,
        .channel = PCNT_CHANNEL_0,
    };
    pcnt_unit_config(&enc_config);

    enc_config.pulse_gpio_num = GPIO_NUM_41;
    enc_config.ctrl_gpio_num = GPIO_NUM_40;
    enc_config.channel = PCNT_CHANNEL_1;
    enc_config.pos_mode = PCNT_COUNT_DEC;
    enc_config.neg_mode = PCNT_COUNT_INC;
    pcnt_unit_config(&enc_config);

    pcnt_set_filter_value(PCNT_UNIT_0, 250);

    pcnt_filter_enable(PCNT_UNIT_0);

    gpio_pullup_en(GPIO_NUM_40);
    gpio_pullup_en(GPIO_NUM_41);

    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    pcnt_counter_resume(PCNT_UNIT_0);
}

long floordiv(long a, long b) {
    long q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0)))
        q--;
    return q;
}

long get_encoder_value() {
    int16_t count;
    pcnt_get_counter_value(PCNT_UNIT_0, &count);
    return floordiv(count + 2, 4);
}
