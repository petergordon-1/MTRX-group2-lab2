#include "timer.h"

static void (*timer_callback)(void) = 0;
static uint32_t current_period = 0;

void timer_init(uint32_t period_ms, void (*callback)(void)) {
    timer_callback = callback;
    current_period = period_ms;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 8000 - 1;
    TIM2->ARR = period_ms - 1;
    TIM2->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 2);
    TIM2->CR1 |= TIM_CR1_CEN;
}

uint32_t timer_get_period(void) {
    return current_period;
}

void timer_set_period(uint32_t period_ms) {
    current_period = period_ms;
    TIM2->ARR = period_ms - 1;
}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        if (timer_callback != 0) {
            timer_callback();
        }
    }
}
