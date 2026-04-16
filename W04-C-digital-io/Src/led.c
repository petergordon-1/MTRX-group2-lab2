#include "led.h"
#include "gpio.h"

#define LED_PORT GPIOE
#define LED_FIRST_PIN 8

/* internal state, hidden from other modules */
static uint8_t led_states[LED_COUNT] = {0};

static uint8_t led_index_to_pin(uint8_t led_index) {
    return (uint8_t)(LED_FIRST_PIN + led_index);
}

void led_init_all(void) {
    uint8_t i;

    for (i = 0; i < LED_COUNT; i++) {
        gpio_init_pin(LED_PORT, led_index_to_pin(i), GPIO_MODE_OUTPUT);
        gpio_write_pin(LED_PORT, led_index_to_pin(i), 0);
        led_states[i] = 0;
    }
}

void led_set(uint8_t led_index, uint8_t state) {
    if (led_index >= LED_COUNT) {
        return;
    }

    state = (state != 0U) ? 1U : 0U;
    led_states[led_index] = state;
    gpio_write_pin(LED_PORT, led_index_to_pin(led_index), state);
}

void led_toggle(uint8_t led_index) {
    if (led_index >= LED_COUNT) {
        return;
    }

    led_states[led_index] ^= 1U;
    gpio_write_pin(LED_PORT, led_index_to_pin(led_index), led_states[led_index]);
}

uint8_t led_get(uint8_t led_index) {
    if (led_index >= LED_COUNT) {
        return 0;
    }

    return led_states[led_index];
}

void led_set_all(uint8_t pattern) {
    uint8_t i;

    for (i = 0; i < LED_COUNT; i++) {
        uint8_t state = (pattern >> i) & 0x1U;
        led_set(i, state);
    }
}
