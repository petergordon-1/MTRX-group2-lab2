# MTRX2700 Project 2

## Group Members

Tom Whitehead

## Roles and Responbilities

5.2 - Tom Whitehead

5.5 - Combined


## Project Overview

# Exercise 1 - Digital I/O

## Summary
This exercise implements a modular GPIO interface for the STM32F3 Discovery board. A generic GPIO layer handles all the register level hardware configuration, and independent LED and button modules are built on top of it. Pressing the user button triggers a callback that toggles the LED array between two alternating patterns.

## Usage
1. Call `led_init_all()` to initialise all 8 LEDs on GPIOE pins 8-15
2. Call `button_init(callback)` to initialise the button on PA0 and register a callback function
3. Call `button_update()` repeatedly in the main loop to check for button presses
4. Use `led_set(index, state)` to control individual LEDs or `led_set_all(pattern)` for all 8 at once
5. Use `led_get(index)` to read the current state of an LED

## Valid Input
* `led_index`: A value between 0 and 7 corresponding to LEDs on PE8 to PE15
* `pattern`: A uint8_t bitmask where each bit corresponds to one LED
* `callback`: Any function with signature `void f(void)` can be registered as the button callback

## Functions and Modularity
This exercise is split into three modules each with a header and source file. Each module has one responsibility and the hardware register access is fully contained inside `gpio.c`.

* **GPIO module**: The base layer for all hardware pin access. `gpio_init_pin` clears and sets the MODER bits for a given pin and configures output type, speed and pull resistors. `gpio_write_pin` and `gpio_read_pin` write and read via ODR and IDR. `gpio_toggle_pin` flips a pin by XORing the ODR bit. No other module touches these registers directly.

* **LED module**: Built on top of the GPIO module. Manages 8 LEDs on GPIOE pins 8-15. Internal LED state is stored in a private array inside `led.c` and is only accessible through `led_get` and `led_set` — nothing outside the module can write to the LED pins directly. `led_set_all` takes a bitmask and sets all 8 LEDs in one call.

* **Button module**: Also built on top of the GPIO module. Reads PA0 and detects a rising edge — the transition from not pressed to pressed. When this happens it fires the registered callback function. This means the callback only triggers once per press regardless of how long the button is held down. The callback is registered during `button_init` so the button module has no knowledge of what the callback actually does.

## Testing

* **LEDs initialise correctly**: On power up confirm LEDs 1, 3, 5, 7 are on and 0, 2, 4, 6 are off matching the initial pattern `0b10101010`
* **Button toggles pattern**: Press the button once and confirm the pattern inverts to `0b01010101`, press again and it should return to `0b10101010`
* **Rising edge detection works**: Hold the button down and confirm the pattern only changes once when first pressed and does not keep toggling while held
* **LED encapsulation**: Confirm no code outside `led.c` writes to GPIOE ODR directly, all changes go through `led_set` or `led_set_all`
* **Callback mechanism**: Confirm `button.c` has no reference to LEDs anywhere, the LED logic lives entirely in the callback registered by `main.c`
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




# Exercise 3 - Serial Interface

## Summary
This exercise implements a modular UART serial interface for the STM32F3 Discovery board using USART1 on PC4 (TX) and PC5 (RX). It provides functions for sending strings, sending structured binary messages with framing and checksums, and receiving messages using an interrupt driven approach that fires a callback when a complete valid packet arrives.

## Usage
1. Call `SerialInitialise(baudRate, &USART1_PORT, tx_callback, rx_callback)` to set up the serial port
2. Call `SerialOutputString(str, &USART1_PORT)` to send a null terminated string
3. Call `sendMsg(struct_ptr, size, type, &USART1_PORT)` to send a structured packet
4. Call `receiveMsg(&USART1_PORT)` to wait for and process an incoming packet
5. The rx_callback registered during initialisation is called automatically when a valid packet is received

## Valid Input
* `baudRate`: Use the provided constants — `BAUD_9600`, `BAUD_19200`, `BAUD_38400`, `BAUD_57600`, `BAUD_115200`
* `msg_size`: A value from 0 to 9 — the packet format encodes size as a single ASCII digit
* `msg_type`: A value from 0 to 9 for the same reason
* Packets sent from a terminal should follow the format `<SIZETYPE BODY CHECKSUM>` for example `<41TEST17>`

## Functions and Modularity
This exercise is contained in a single serial module with a header and source file. The `SerialPort` struct holds all the hardware configuration and callbacks so the same module could support multiple serial ports with different settings.

* **SerialInitialise**: Configures GPIOC pins for alternate function mode, sets up USART1 at the given baud rate, enables TX and RX, and registers the TX and RX callback functions.

* **SerialOutputString / SerialOutputBytes**: Transmit a string or byte array one byte at a time by polling the TXE flag before each write. Fires the TX callback with the byte count when done.

* **sendMsg**: Assembles a structured packet from a message struct and transmits it. The format is `< SIZE TYPE BODY CHECKSUM >` where SIZE and TYPE are ASCII digit characters and CHECKSUM is a two character hex string calculated by XORing the size, type and all body bytes together.

* **receiveMsg**: Interrupt driven receive. Collects incoming bytes into an internal buffer until the stop byte `>` arrives, validates the checksum, then calls the RX callback with the message type, a pointer to the body, and the body length. If the checksum fails or the packet is malformed an error message is sent back over serial.

## Testing

* **String transmission works**: Call `SerialOutputString` with a known string and confirm it appears correctly in the serial terminal
* **Packet assembly is correct**: Call `sendMsg` with a known struct and confirm the output in the terminal matches the expected format with correct size, type, body and checksum
* **Checksum validation works**: Send a packet from the terminal with a deliberately wrong checksum and confirm the board responds with a checksum error and does not fire the callback
* **Receive callback fires on valid packet**: Send a valid packet like `<41TEST17>` from the terminal and confirm the callback is called with the correct message type, body and length
* **Interrupt driven receive does not block**: Confirm the main loop continues running while waiting for data — the receive is entirely interrupt driven with no polling loop blocking execution
* **Buffer overflow is handled**: Send a packet longer than `RX_BUFFER_SIZE` and confirm the module outputs an error rather than corrupting memory
