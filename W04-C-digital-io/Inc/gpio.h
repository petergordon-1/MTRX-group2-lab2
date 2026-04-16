#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "stm32f303xc.h"

typedef enum {
    GPIO_MODE_INPUT = 0,
    GPIO_MODE_OUTPUT = 1
} gpio_mode_t;

void gpio_enable_port_clock(GPIO_TypeDef *port);
void gpio_init_pin(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode);
void gpio_write_pin(GPIO_TypeDef *port, uint8_t pin, uint8_t value);
uint8_t gpio_read_pin(GPIO_TypeDef *port, uint8_t pin);
void gpio_toggle_pin(GPIO_TypeDef *port, uint8_t pin);

#endif
