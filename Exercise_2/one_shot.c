#include "one_shot.h"

static void (*oneshot_callback)(void) = 0;

void oneshot_start(uint32_t delay_ms, void (*callback)(void)) {
    oneshot_callback = callback;

    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    TIM3->PSC = 8000 - 1;
    TIM3->ARR = delay_ms - 1;
    TIM3->DIER |= TIM_DIER_UIE;
    TIM3->CR1 |= TIM_CR1_OPM;

    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_SetPriority(TIM3_IRQn, 2);

    TIM3->CR1 |= TIM_CR1_CEN;
}

void TIM3_IRQHandler(void) {
    if (TIM3->SR & TIM_SR_UIF) {
        TIM3->SR &= ~TIM_SR_UIF;
        if (oneshot_callback != 0) {
            oneshot_callback();
        }
    }
}
