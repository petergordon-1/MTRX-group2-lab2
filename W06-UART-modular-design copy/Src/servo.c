#include "servo.h"

//store the servo's GPIO port, pin and pulse width
static GPIO_TypeDef *servo_gpio = 0;
static uint16_t servo_pin = 0;
static uint32_t servo_pulse_ms = 0;

/*
 * Builds the code for exercise 2c - initialises the servo, sets up the GPIO pin and timer
 */

void servo_init(GPIO_TypeDef *gpio, uint16_t pin) {
	/// Save the port and pin so the interrupt handler can use them
    servo_gpio = gpio;
    servo_pin = pin;
    // Turn on the clock for GPIOA
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    // Activate the clock for TIM1, a different timer to TIM2 which is also used
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    // Set the pin to output mode so we can drive it high and low
    // Set PA8 to alternate function mode
    GPIOA->MODER &= ~(0x3UL << 16);
    GPIOA->MODER |= (0x2UL << 16);
    GPIOA->OTYPER &= ~(1UL << 8);
    GPIOA->OSPEEDR |= (0x3UL << 16);
    GPIOA->PUPDR &= ~(0x3UL << 16);
    GPIOA->AFR[1] &= ~(0xFUL << 0);
    GPIOA->AFR[1] |= (0x6UL << 0);

    // Hardware PWM settings
    TIM1->PSC = 719;
    TIM1->ARR = 1999;
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;
    TIM1->CCER |= TIM_CCER_CC1E;
    TIM1->CCR1 = 150;
    TIM1->EGR = TIM_EGR_UG;
    TIM1->SR = 0;
    TIM1->CR1 |= TIM_CR1_ARPE;
    // Starts the timer count
    TIM1->BDTR |= TIM_BDTR_MOE;
    TIM1->CR1 |= TIM_CR1_CEN;
}

// For exercise 2C, this sets the Servo's position
void servo_set_position(float position) {
    if (position < 0.0f) position = 0.0f;
    if (position > 1.0f) position = 1.0f;
    uint32_t ccr = 100 + (uint32_t)(position * 100.0f);
    TIM1->CCR1 = ccr;
