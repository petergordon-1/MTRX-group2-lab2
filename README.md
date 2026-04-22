# MTRX2700 Project 2

## Group Members

Tom Whitehead

## Roles and Responbilities

5.2 - Tom Whitehead

5.5 - Combined


## Project Overview

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

- **Servo module**: This module uses TIM1 to create a PWM signal that controls a servo motor. At first servo_init(gpio,pin) establishes a GPIO pin to be used as an output for this signal and also establishes the perameters for the TIM1 clock. Next the sero_set_position function uses a position float value as an input and adds it to the pulse period, if position is 0, a full clockwise rotation occurs, 0.5 rotation to centre position and 1 is a full anti-clockwise rotation. The updated servo_pulse_ms variable is then used by the interruopt handler to determine how long the pin stays high. The funciton`TIM1_UP_TIM16_IRQHandler` is called everytime TIM1 fires and toggles the GPIO pin as either high or low to generate the PWM signal. At each interrupt the current state is checked from the variable 'state', if it is low then drive the pin high and if it is high then drive the pin low. 

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
