#include "servo.h"

static GPIO_TypeDef *servo_gpio = 0;
static uint16_t servo_pin = 0;
static uint32_t servo_pulse_ms = 0;

void servo_init(GPIO_TypeDef *gpio, uint16_t pin) {
    servo_gpio = gpio;
    servo_pin = pin;

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    servo_gpio->MODER &= ~(3 << (8 * 2));
    servo_gpio->MODER |= (1 << (8 * 2));

    TIM1->PSC = 8000 - 1;
    TIM1->ARR = 20 - 1;
    TIM1->DIER |= TIM_DIER_UIE;

    NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
    NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 2);

    TIM1->CR1 |= TIM_CR1_CEN;
}

void servo_set_position(float position) {
    servo_pulse_ms = 1 + position;
}

void TIM1_UP_TIM16_IRQHandler(void) {
    static uint32_t state = 0;
    if (TIM1->SR & TIM_SR_UIF) {
        TIM1->SR &= ~TIM_SR_UIF;
        if (servo_gpio != 0) {
            if (state == 0) {
                servo_gpio->ODR |= servo_pin;
                TIM1->ARR = servo_pulse_ms - 1;
                state = 1;
            } else {
                servo_gpio->ODR &= ~servo_pin;
                TIM1->ARR = 20 - servo_pulse_ms - 1;
                state = 0;
            }
        }
    }
}
