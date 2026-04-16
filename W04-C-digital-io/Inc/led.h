#ifndef LED_H
#define LED_H

#include <stdint.h>

#define LED_COUNT 8

void led_init_all(void);
void led_set(uint8_t led_index, uint8_t state);
void led_toggle(uint8_t led_index);
uint8_t led_get(uint8_t led_index);
void led_set_all(uint8_t pattern);

#endif
