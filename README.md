# MTRX2700 Project 2

## Group Members

Tom Whitehead

## Roles and Responbilities

5.2 - Tom Whitehead

5.5 - Combined


## Project Overview

##Exercise 1: Digital I/O
###Summary
Implements a modular GPIO interface for the STM32F3 Discovery board, with independent modules for the LED array and user button built on top of a generic GPIO layer. Pressing the user button triggers a callback that toggles the LED pattern between two alternating states.
How it works
The program initialises the LED array and button module at startup, then enters a polling loop that calls button_update() on every iteration. The button module detects a rising edge (not pressed → pressed) and fires a registered callback function. The callback flips the LED pattern by XORing the current pattern with 0xFF, toggling all 8 LEDs between 0b10101010 and 0b01010101. LED state is encapsulated inside led.c and can only be accessed through the provided get/set functions — no other module can write to the LED pins directly.
The GPIO layer handles all register-level configuration. It clears and sets the relevant MODER bits for each pin, configures push-pull output and pull resistors, and provides generic read/write functions used by both the LED and button modules. Neither led.c nor button.c touch hardware registers directly.
###Usage
Flash the code onto the STM32F3 Discovery board. On startup the LEDs will display the initial pattern 0b10101010 (every other LED on). Press the user button to toggle the pattern. Each press alternates between 0b10101010 and 0b01010101.
###Module Structure
gpio.c / gpio.h
Generic GPIO driver. Handles pin initialisation, clock enabling, and provides read/write/toggle functions for any GPIO port and pin. This is the only module that touches MODER, ODR, and IDR directly.

gpio_enable_port_clock — enables the AHB clock for the specified port
gpio_init_pin — clears and sets the MODER bits for a pin, configures output type, speed, and pull resistors
gpio_write_pin — sets a pin high or low via ODR
gpio_read_pin — reads the current state of a pin via IDR
gpio_toggle_pin — flips a pin state by XORing the ODR bit

led.c / led.h
LED array driver built on top of gpio.c. Manages 8 LEDs on GPIOE pins 8–15. Internal state is stored in a private array and is only accessible through get/set functions.

led_init_all — initialises all 8 LED pins as outputs and sets them low
led_set — sets a specific LED on or off, updates internal state
led_get — returns the current state of a specific LED
led_toggle — flips a specific LED
led_set_all — sets all 8 LEDs at once using a bitmask pattern

button.c / button.h
Button driver built on top of gpio.c. Implements rising edge detection to fire a callback exactly once per press, regardless of how long the button is held.

button_init — initialises PA0 as input and registers a callback function
button_is_pressed — returns the raw current state of the button
button_update — must be called regularly in the main loop; detects rising edge and fires the callback

###Testing
LEDs initialise correctly
On power-up, confirm that LEDs 1, 3, 5, 7 are on and LEDs 0, 2, 4, 6 are off (pattern 0b10101010).
Button toggles pattern
Press the user button once. The pattern should invert to 0b01010101. Press again and it should return to 0b10101010.
Rising edge detection works
Hold the button down. The pattern should only change once when the button is first pressed, not repeatedly while it is held.
LED state encapsulation
Confirm that no code outside led.c writes directly to GPIOE ODR. All LED changes go through led_set or led_set_all.
Callback mechanism
Confirm that button.c has no knowledge of LEDs. The callback is registered by main.c at initialisation and called by the button module when a rising edge is detected.
###Notes
The button module uses polling in the main loop rather than hardware interrupts. This means the button is only checked as fast as the loop runs, which is sufficient for human button presses but would not be appropriate for high frequency signals. The tradeoff is simpler code with no interrupt configuration required.
The rising edge detection prevents the callback from firing continuously while the button is held, but does not implement full hardware debouncing. Rapid repeated presses may occasionally register twice due to mechanical bounce on the button contacts.
## Exercise 5.2 - Timer interface

### Summary
This exercise implements a timer based module and a PWM (pulse with moderation) signal to activate a small servo motor. First a timer module allows a callback function to be used in regular intervals and is used as a trigger for other modules, the servo motor uses this to create a PWM signal that controls the orientation of the motor arm. A oneshot timer is also used which activates after a delay causing an interrupt but doesn't continue unlike the other timer.

### Usage
1. Call `timer_init(period_ms, callback)` to start the repeating timer
2. Call `servo_init(GPIOA, (1 << 8))` to initialise the servo on PA8
3. Call `servo_set_position(0.5)` to set the servo position (0.0 to 1.0)
4. Call `oneshot_start(delay_ms, callback)` to trigger a one shot event
5. Use `timer_get_period()` and `timer_set_period()` to read or change the timer period

### Valid input
- period_ms: Any positive unit32_t value in milliseconds can be used
- postion: A float value between 0 (full clockwise rotation)and 1 (full anticlockwise rotation)
- delay_ms: Same as period_ms any positive time value in milliseconds
### Functions and modularity
This exercise is split into a modular design where each file has one unique task or function, each module has a header file used to define functions and variable types and a source .c file with the main implementations included.

- **Timer module**: This module uses a repeating timer to later be used by the Servo motor for a PWM signal. Using TIM2 an interrupt is fired at every 500ms and then calls the callback function, which in this case increases timer_count to keep track of the timer. This count is then read or changed by timer_get_period() and timer_set_period() getter and setter functions. 

- **Servo module**: This module uses TIM1 to create a PWM signal that controls a servo motor the servo_init(gpio, pin) functino sets up PA8 in alternate function mode (AF6) so it is 
directly driven by TIM1, and configures TIM1 to generate a 50Hz PWM signal with a period of 20ms. The hardware PWM means the signal is generated  automatically by the timer without needing any interrupt handler. Then the servo_set_position(position) functino takes a float between 0.0 and 1.0 and maps it to a pulse width between 1ms and 2ms. A position of 0.0 gives a 1ms pulse for full clockwise rotation, 0.5 gives a 1.5ms pulse for the centre position and 1.0 gives a 2ms pulse for a counterclockwise rotation. 

-**OneShot module**: This module uses a one off timer using the TIM3 clock, initially, the oneshot_start function gives the TIM3 clock the parameters to count to an 8 second delay and then create an interrupt. When the interrupt activates the timer stops itself using the One Pulse Mode flag which indicates for TIM3 to stop counting after it has reached the time, as such the function is only ever used once. 


### Testing

**Timer module:**
- Add `volatile uint32_t timer_count` to main and increment it in the callback
- Run in debugger and watch in the Expressions window
- With `period_ms = 500`, count should increment twice per second
- Check `current_period` in Expressions window updates correctly

**Servo module:**
- Call `servo_set_position(0.0)` for full clockwise
- Call `servo_set_position(1.0)` for full counterclockwise
- Verify servo moves to different positions

**Oneshot module :**
- Set `timer_count` to reset to 0 in the oneshot callback
- Watch `timer_count` in debugger — resets after the delay and then continues to increase
