#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

/* Supported baud rates */
#define BAUD_9600      9600U
#define BAUD_19200     19200U
#define BAUD_38400     38400U
#define BAUD_57600     57600U
#define BAUD_115200    115200U

typedef struct _SerialPort SerialPort;

/* Public serial port instance */
extern SerialPort USART1_PORT;

/* Initialise a serial port */
void SerialInitialise(uint32_t baudRate,
                      SerialPort *serial_port,
                      void (*completion_function)(uint32_t));

/* Send one character */
void SerialOutputChar(uint8_t data, SerialPort *serial_port);

/* Send a null-terminated string */
void SerialOutputString(uint8_t *pt, SerialPort *serial_port);

/* Send a fixed number of bytes from memory */
void SerialOutputBytes(uint8_t *data, uint32_t length, SerialPort *serial_port);

/* Receive one character */
uint8_t SerialInputChar(SerialPort *serial_port);

/* Receive a fixed number of bytes into a buffer */
void SerialInputBytes(uint8_t *buffer, uint32_t length, SerialPort *serial_port);

#endif
