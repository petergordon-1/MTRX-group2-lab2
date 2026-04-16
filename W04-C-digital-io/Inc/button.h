#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

typedef void (*button_callback_t)(void);

void button_init(button_callback_t callback);
uint8_t button_is_pressed(void);
void button_update(void);

#endif
