#include "gpio.h"

void gpio_enable_port_clock(GPIO_TypeDef *port) {
    if (port == GPIOA) {
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    } else if (port == GPIOB) {
        RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    } else if (port == GPIOC) {
        RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    } else if (port == GPIOD) {
        RCC->AHBENR |= RCC_AHBENR_GPIODEN;
    } else if (port == GPIOE) {
        RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    } else if (port == GPIOF) {
        RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
    }
}

void gpio_init_pin(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode) {
    uint32_t shift = pin * 2U;

    gpio_enable_port_clock(port);

    /* Clear the 2 mode bits first */
    port->MODER &= ~(0x3U << shift);

    if (mode == GPIO_MODE_OUTPUT) {
        /* Set to 01 for output */
        port->MODER |= (0x1U << shift);

        /* Push-pull output */
        port->OTYPER &= ~(1U << pin);

        /* Low speed is fine for LEDs */
        port->OSPEEDR &= ~(0x3U << shift);

        /* No pull-up / pull-down */
        port->PUPDR &= ~(0x3U << shift);
    } else {
        /* Input mode is 00, already cleared above */
        port->PUPDR &= ~(0x3U << shift);
    }
}

void gpio_write_pin(GPIO_TypeDef *port, uint8_t pin, uint8_t value) {
    if (value) {
        port->ODR |= (1U << pin);
    } else {
        port->ODR &= ~(1U << pin);
    }
}

uint8_t gpio_read_pin(GPIO_TypeDef *port, uint8_t pin) {
    return (uint8_t)((port->IDR >> pin) & 0x1U);
}

void gpio_toggle_pin(GPIO_TypeDef *port, uint8_t pin) {
    port->ODR ^= (1U << pin);
}
