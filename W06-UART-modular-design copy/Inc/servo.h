#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include <stdint.h>
#include "stm32f303xc.h"

void servo_init(GPIO_TypeDef *gpio, uint16_t pin);
void servo_set_position(float position);

#endif
