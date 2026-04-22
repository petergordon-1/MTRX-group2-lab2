#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include <stdint.h>
#include "stm32f303xc.h"

void timer_init(uint32_t period_ms, void (*callback)(void));
void timer_set_period(uint32_t period_ms);
uint32_t timer_get_period(void);

#endif
