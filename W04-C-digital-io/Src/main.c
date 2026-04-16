#include <stdint.h>
#include "stm32f303xc.h"

#include "led.h"
#include "button.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

static void button_pressed_handler(void) {
    static uint8_t pattern = 0b10101010;

    pattern ^= 0xFFU;
    led_set_all(pattern);
}

int main(void)
{
    led_init_all();
    button_init(button_pressed_handler);

    /* initial pattern */
    led_set_all(0b10101010);

    for (;;) {
        button_update();
    }
}
