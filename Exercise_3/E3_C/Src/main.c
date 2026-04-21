#include <stdint.h>
#include "serial.h"

typedef struct __attribute__((packed)) {
    char text[4];
} TextMessage;

typedef struct __attribute__((packed)) {
    uint8_t shape_id;
    uint16_t size;
} ShapeMessage;

typedef struct __attribute__((packed)) {
    int16_t speed;
    int16_t direction;
} MovementMessage;

int main(void)
{
    TextMessage text_msg = { {'T', 'E', 'S', 'T'} };
    ShapeMessage shape_msg = { 2, 150 };
    MovementMessage move_msg = { 25, -10 };

    SerialInitialise(BAUD_115200, &USART1_PORT, 0);

    while (1)
    {
        SerialOutputString((uint8_t *)"Sending TEXT packet...\r\n", &USART1_PORT);
        sendMsg(&text_msg, sizeof(text_msg), MSG_TYPE_TEXT, &USART1_PORT);

        for (volatile uint32_t i = 0; i < 800000; i++) {
        }

        SerialOutputString((uint8_t *)"Sending SHAPE packet...\r\n", &USART1_PORT);
        sendMsg(&shape_msg, sizeof(shape_msg), MSG_TYPE_SHAPE, &USART1_PORT);

        for (volatile uint32_t i = 0; i < 800000; i++) {
        }

        SerialOutputString((uint8_t *)"Sending MOVEMENT packet...\r\n", &USART1_PORT);
        sendMsg(&move_msg, sizeof(move_msg), MSG_TYPE_MOVEMENT, &USART1_PORT);

        for (volatile uint32_t i = 0; i < 1200000; i++) {
        }
    }
}
