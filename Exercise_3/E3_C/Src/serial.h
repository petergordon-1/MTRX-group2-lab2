#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#define BAUD_9600      9600U
#define BAUD_19200     19200U
#define BAUD_38400     38400U
#define BAUD_57600     57600U
#define BAUD_115200    115200U

#define MSG_START_BYTE '<'
#define MSG_STOP_BYTE  '>'

typedef struct _SerialPort SerialPort;

extern SerialPort USART1_PORT;

/* Message identifiers */
typedef enum {
    MSG_TYPE_TEXT     = 1,
    MSG_TYPE_SHAPE    = 2,
    MSG_TYPE_MOVEMENT = 3
} MessageType;

void SerialInitialise(uint32_t baudRate,
                      SerialPort *serial_port,
                      void (*completion_function)(uint32_t));

void SerialOutputChar(uint8_t data, SerialPort *serial_port);
void SerialOutputString(uint8_t *pt, SerialPort *serial_port);
void SerialOutputBytes(uint8_t *data, uint32_t length, SerialPort *serial_port);
uint8_t SerialInputChar(SerialPort *serial_port);
void SerialInputBytes(uint8_t *buffer, uint32_t length, SerialPort *serial_port);

void sendMsg(void *msg_struct,
             uint8_t msg_size,
             uint8_t msg_type,
             SerialPort *serial_port);

#endif
