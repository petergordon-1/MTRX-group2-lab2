#include <stdint.h>
#include <stdio.h>
#include "serial.h"

void finished_transmission(uint32_t bytes_sent)
{
    (void)bytes_sent;
}

int main(void)
{
    uint8_t tx_data[4] = {'T', 'E', 'S', 'T'};
    uint8_t rx_data[4];
    uint8_t msg_buffer[64];

    SerialInitialise(BAUD_115200, &USART1_PORT, &finished_transmission);

    SerialOutputString((uint8_t *)"Exercise 3a\r\n", &USART1_PORT);
    SerialOutputString((uint8_t *)"Sending 4 bytes: ", &USART1_PORT);
    SerialOutputBytes(tx_data, 4, &USART1_PORT);
    SerialOutputString((uint8_t *)"\r\n", &USART1_PORT);

    SerialOutputString((uint8_t *)"Type 4 characters:\r\n", &USART1_PORT);

    SerialInputBytes(rx_data, 4, &USART1_PORT);

    sprintf((char *)msg_buffer, "You sent: %c%c%c%c\r\n",
            rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
    SerialOutputString(msg_buffer, &USART1_PORT);

    while (1) {
    }
}
