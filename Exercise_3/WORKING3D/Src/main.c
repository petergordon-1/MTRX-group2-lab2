#include <stdint.h>
#include <stdio.h>
#include "serial.h"

static int is_printable_text(uint8_t *data, uint32_t length)
{
    uint32_t i;

    for (i = 0; i < length; i++) {
        if (data[i] < 32 || data[i] > 126) {
            return 0;
        }
    }

    return 1;
}

static void print_hex(uint8_t *data, uint32_t length)
{
    uint32_t i;
    char buffer[16];

    for (i = 0; i < length; i++) {
        sprintf(buffer, "%02X ", data[i]);
        SerialOutputString((uint8_t *)buffer, &USART1_PORT);
    }

    SerialOutputString((uint8_t *)"\r\n", &USART1_PORT);
}

void tx_done(uint32_t bytes_sent)
{
    (void)bytes_sent;
}

void rx_done(uint8_t msg_type, uint8_t *data, uint32_t length)
{
    uint32_t i;
    char buffer[64];
    uint8_t text_buffer[RX_BUFFER_SIZE];

    SerialOutputString((uint8_t *)"\r\nPacket received\r\n", &USART1_PORT);

    sprintf(buffer, "Type: %u\r\n", msg_type);
    SerialOutputString((uint8_t *)buffer, &USART1_PORT);

    sprintf(buffer, "Size: %lu\r\n", (unsigned long)length);
    SerialOutputString((uint8_t *)buffer, &USART1_PORT);

    if (is_printable_text(data, length)) {
        for (i = 0; i < length && i < RX_BUFFER_SIZE - 1; i++) {
            text_buffer[i] = data[i];
        }
        text_buffer[i] = '\0';

        SerialOutputString((uint8_t *)"Body (text): ", &USART1_PORT);
        SerialOutputString(text_buffer, &USART1_PORT);
        SerialOutputString((uint8_t *)"\r\n", &USART1_PORT);
    } else {
        SerialOutputString((uint8_t *)"Body (text): [binary]\r\n", &USART1_PORT);
    }

    SerialOutputString((uint8_t *)"Body (hex): ", &USART1_PORT);
    print_hex(data, length);

}

int main(void)
{
    SerialInitialise(BAUD_115200, &USART1_PORT, tx_done, rx_done);

    SerialOutputString((uint8_t *)"\n3d receive test ready\r\n", &USART1_PORT);
    SerialOutputString((uint8_t *)"Enter packet like <41TEST13>\r\n", &USART1_PORT);
    SerialOutputString((uint8_t *)"Format = <size type body checksum>\r\n", &USART1_PORT);
    SerialOutputString((uint8_t *)"Waiting for packet...\r\n", &USART1_PORT);

    while (1) {
        receiveMsg(&USART1_PORT);
    }
}
