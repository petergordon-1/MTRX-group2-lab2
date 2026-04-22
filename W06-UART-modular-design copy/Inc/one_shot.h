#ifndef INC_ONESHOT_H_
#define INC_ONESHOT_H_

#include <stdint.h>
#include "stm32f303xc.h"

void oneshot_start(uint32_t delay_ms, void (*callback)(void));

#endif
