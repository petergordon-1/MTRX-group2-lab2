#include "button.h"
#include "gpio.h"

#define BUTTON_PORT GPIOA
#define BUTTON_PIN  0

static button_callback_t button_press_callback = 0;
static uint8_t previous_button_state = 0;

void button_init(button_callback_t callback) {
    gpio_init_pin(BUTTON_PORT, BUTTON_PIN, GPIO_MODE_INPUT);
    button_press_callback = callback;
    previous_button_state = button_is_pressed();
}

uint8_t button_is_pressed(void) {
    return gpio_read_pin(BUTTON_PORT, BUTTON_PIN);
}

/*
 * Poll this regularly in main.
 * Triggers callback only on rising edge:
 * not pressed -> pressed
 */
void button_update(void) {
    uint8_t current_button_state = button_is_pressed();

    if ((current_button_state == 1U) && (previous_button_state == 0U)) {
        if (button_press_callback != 0) {
            button_press_callback();
        }
    }

    previous_button_state = current_button_state;
}
