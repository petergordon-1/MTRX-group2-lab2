#include "serial.h"
#include <stdint.h>

/* Base addresses */
#define RCC_BASE        0x40021000UL
#define GPIOC_BASE      0x48000800UL
#define USART1_BASE     0x40013800UL

/* RCC registers */
#define RCC_AHBENR      (*(volatile uint32_t *)(RCC_BASE + 0x14))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x1C))

/* GPIOC registers */
#define GPIOC_MODER     (*(volatile uint32_t *)(GPIOC_BASE + 0x00))
#define GPIOC_OSPEEDR   (*(volatile uint32_t *)(GPIOC_BASE + 0x08))
#define GPIOC_AFRL      (*(volatile uint32_t *)(GPIOC_BASE + 0x20))

/* USART1 registers */
#define USART1_CR1      (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_BRR      (*(volatile uint32_t *)(USART1_BASE + 0x0C))
#define USART1_ISR      (*(volatile uint32_t *)(USART1_BASE + 0x1C))
#define USART1_RDR      (*(volatile uint32_t *)(USART1_BASE + 0x24))
#define USART1_TDR      (*(volatile uint32_t *)(USART1_BASE + 0x28))

/* RCC bit definitions */
#define RCC_APB1ENR_PWREN       (1U << 28)
#define RCC_APB2ENR_SYSCFGEN    (1U << 0)
#define RCC_AHBENR_GPIOCEN      (1U << 19)
#define RCC_APB2ENR_USART1EN    (1U << 14)

/* USART bit definitions */
#define USART_CR1_UE            (1U << 0)
#define USART_CR1_RE            (1U << 2)
#define USART_CR1_TE            (1U << 3)

#define USART_ISR_RXNE          (1U << 5)
#define USART_ISR_TXE           (1U << 7)

struct _SerialPort {
    uint32_t uart_base;
    uint32_t gpio_base;
    volatile uint32_t MaskAPB2ENR;
    volatile uint32_t MaskAPB1ENR;
    volatile uint32_t MaskAHBENR;
    volatile uint32_t TxRxPinModeMask;
    volatile uint32_t TxRxPinModeValue;
    volatile uint32_t TxRxPinSpeedMask;
    volatile uint32_t TxRxPinSpeedValue;
    volatile uint32_t AlternateFunctionLowMask;
    volatile uint32_t AlternateFunctionLowValue;
    volatile uint32_t AlternateFunctionHighMask;
    volatile uint32_t AlternateFunctionHighValue;

    SerialTxCallback tx_callback;
    SerialRxCallback rx_callback;

    uint8_t rx_buffer[RX_BUFFER_SIZE];
};

SerialPort USART1_PORT = {
    USART1_BASE,
    GPIOC_BASE,
    RCC_APB2ENR_USART1EN,
    0x00,
    RCC_AHBENR_GPIOCEN,

    ((3U << (4 * 2)) | (3U << (5 * 2))),
    ((2U << (4 * 2)) | (2U << (5 * 2))),

    ((3U << (4 * 2)) | (3U << (5 * 2))),
    ((3U << (4 * 2)) | (3U << (5 * 2))),

    ((0xFU << (4 * 4)) | (0xFU << (5 * 4))),
    ((7U << (4 * 4)) | (7U << (5 * 4))),

    0x00,
    0x00,

    0x00,
    0x00,

    {0}
};

static uint16_t SerialBaudToBRR(uint32_t baudRate)
{
    uint32_t pclk = 8000000UL;
    return (uint16_t)((pclk + (baudRate / 2U)) / baudRate);
}

void SerialInitialise(uint32_t baudRate,
                      SerialPort *serial_port,
                      SerialTxCallback tx_callback,
                      SerialRxCallback rx_callback)
{
    serial_port->tx_callback = tx_callback;
    serial_port->rx_callback = rx_callback;

    RCC_APB1ENR |= RCC_APB1ENR_PWREN;
    RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC_AHBENR  |= serial_port->MaskAHBENR;
    RCC_APB1ENR |= serial_port->MaskAPB1ENR;
    RCC_APB2ENR |= serial_port->MaskAPB2ENR;

    GPIOC_MODER &= ~(serial_port->TxRxPinModeMask);
    GPIOC_MODER |=  serial_port->TxRxPinModeValue;

    GPIOC_OSPEEDR &= ~(serial_port->TxRxPinSpeedMask);
    GPIOC_OSPEEDR |=  serial_port->TxRxPinSpeedValue;

    GPIOC_AFRL &= ~(serial_port->AlternateFunctionLowMask);
    GPIOC_AFRL |=  serial_port->AlternateFunctionLowValue;

    USART1_CR1 = 0;
    USART1_BRR = SerialBaudToBRR(baudRate);
    USART1_CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void SerialOutputChar(uint8_t data, SerialPort *serial_port)
{
    (void)serial_port;

    while ((USART1_ISR & USART_ISR_TXE) == 0U) {
    }

    USART1_TDR = data;
}

void SerialOutputString(uint8_t *pt, SerialPort *serial_port)
{
    uint32_t counter = 0U;

    if (pt == 0) {
        return;
    }

    while (*pt != '\0') {
        SerialOutputChar(*pt, serial_port);
        counter++;
        pt++;
    }

    if (serial_port->tx_callback != 0) {
        serial_port->tx_callback(counter);
    }
}

void SerialOutputBytes(uint8_t *data, uint32_t length, SerialPort *serial_port)
{
    uint32_t counter;

    if (data == 0) {
        return;
    }

    for (counter = 0; counter < length; counter++) {
        SerialOutputChar(data[counter], serial_port);
    }

    if (serial_port->tx_callback != 0) {
        serial_port->tx_callback(length);
    }
}

uint8_t SerialInputChar(SerialPort *serial_port)
{
    (void)serial_port;

    while ((USART1_ISR & USART_ISR_RXNE) == 0U) {
    }

    return (uint8_t)(USART1_RDR & 0xFFU);
}

void SerialInputBytes(uint8_t *buffer, uint32_t length, SerialPort *serial_port)
{
    uint32_t counter;

    if (buffer == 0) {
        return;
    }

    for (counter = 0; counter < length; counter++) {
        buffer[counter] = SerialInputChar(serial_port);
    }
}

static uint8_t calculateChecksumAscii(uint8_t msg_size,
                                      uint8_t msg_type,
                                      uint8_t *msg_body,
                                      uint32_t body_length)
{
    uint8_t checksum = 0;
    uint32_t i;

    checksum ^= msg_size;
    checksum ^= msg_type;

    for (i = 0; i < body_length; i++) {
        checksum ^= msg_body[i];
    }

    return checksum;
}

static uint8_t nibbleToHexChar(uint8_t nibble)
{
    if (nibble < 10) {
        return (uint8_t)('0' + nibble);
    } else {
        return (uint8_t)('A' + (nibble - 10));
    }
}

static uint8_t hexCharToNibble(uint8_t c)
{
    if (c >= '0' && c <= '9') {
        return (uint8_t)(c - '0');
    }
    if (c >= 'A' && c <= 'F') {
        return (uint8_t)(c - 'A' + 10);
    }
    if (c >= 'a' && c <= 'f') {
        return (uint8_t)(c - 'a' + 10);
    }

    return 0xFF;
}

void sendMsg(void *msg_struct,
             uint8_t msg_size,
             uint8_t msg_type,
             SerialPort *serial_port)
{
    uint8_t checksum;
    uint8_t *body_ptr;
    uint8_t size_char;
    uint8_t type_char;
    uint8_t checksum_hi;
    uint8_t checksum_lo;

    if (msg_struct == 0) {
        return;
    }

    /* This simple terminal-friendly format only supports sizes 0-9 */
    if (msg_size > 9 || msg_type > 9) {
        return;
    }

    body_ptr = (uint8_t *)msg_struct;

    size_char = (uint8_t)('0' + msg_size);
    type_char = (uint8_t)('0' + msg_type);

    checksum = calculateChecksumAscii(size_char, type_char, body_ptr, msg_size);
    checksum_hi = nibbleToHexChar((uint8_t)((checksum >> 4) & 0x0F));
    checksum_lo = nibbleToHexChar((uint8_t)(checksum & 0x0F));

    /*
     * Packet format:
     * < SIZE TYPE BODY CHECKSUM >
     * Example: <41TEST17>
     */
    SerialOutputChar(MSG_START_BYTE, serial_port);
    SerialOutputChar(size_char, serial_port);
    SerialOutputChar(type_char, serial_port);
    SerialOutputBytes(body_ptr, msg_size, serial_port);
    SerialOutputChar(checksum_hi, serial_port);
    SerialOutputChar(checksum_lo, serial_port);
    SerialOutputChar(MSG_STOP_BYTE, serial_port);
}

void receiveMsg(SerialPort *serial_port)
{
    uint8_t byte;
    uint32_t count = 0;
    uint8_t size_char;
    uint8_t type_char;
    uint8_t msg_type;
    uint32_t body_length;
    uint8_t received_checksum;
    uint8_t calculated_checksum;
    uint8_t checksum_hi_nibble;
    uint8_t checksum_lo_nibble;
    uint32_t i;

    /* Wait for start byte */
    do {
        byte = SerialInputChar(serial_port);
    } while (byte != MSG_START_BYTE);

    serial_port->rx_buffer[count++] = byte;

    /* Read until stop byte */
    do {
        byte = SerialInputChar(serial_port);

        if (count < RX_BUFFER_SIZE - 1) {
            serial_port->rx_buffer[count++] = byte;
        } else {
            SerialOutputString((uint8_t *)"Buffer overflow\r\n", serial_port);
            return;
        }

    } while (byte != MSG_STOP_BYTE);

    /*
     * Packet format:
     * < SIZE TYPE BODY CHECKSUM >
     * Minimum valid packet is:
     * < 0 1 XX >
     * which is 6 chars long
     */
    if (count < 6) {
        SerialOutputString((uint8_t *)"Packet too short\r\n", serial_port);
        return;
    }

    if (serial_port->rx_buffer[0] != MSG_START_BYTE ||
        serial_port->rx_buffer[count - 1] != MSG_STOP_BYTE) {
        SerialOutputString((uint8_t *)"Bad packet framing\r\n", serial_port);
        return;
    }

    size_char = serial_port->rx_buffer[1];
    type_char = serial_port->rx_buffer[2];

    if (size_char < '0' || size_char > '9') {
        SerialOutputString((uint8_t *)"Invalid size\r\n", serial_port);
        return;
    }

    if (type_char < '0' || type_char > '9') {
        SerialOutputString((uint8_t *)"Invalid type\r\n", serial_port);
        return;
    }

    body_length = (uint32_t)(size_char - '0');
    msg_type = (uint8_t)(type_char - '0');

    /* Expected total length = 1 start + 1 size + 1 type + body + 2 checksum + 1 stop */
    if (count != (uint32_t)(body_length + 6)) {
        SerialOutputString((uint8_t *)"Length mismatch\r\n", serial_port);
        return;
    }

    checksum_hi_nibble = hexCharToNibble(serial_port->rx_buffer[count - 3]);
    checksum_lo_nibble = hexCharToNibble(serial_port->rx_buffer[count - 2]);

    if (checksum_hi_nibble == 0xFF || checksum_lo_nibble == 0xFF) {
        SerialOutputString((uint8_t *)"Invalid checksum format\r\n", serial_port);
        return;
    }

    received_checksum = (uint8_t)((checksum_hi_nibble << 4) | checksum_lo_nibble);

    calculated_checksum = calculateChecksumAscii(size_char,
                                                type_char,
                                                &serial_port->rx_buffer[3],
                                                body_length);

    if (received_checksum != calculated_checksum) {
        SerialOutputString((uint8_t *)"Checksum error\r\n", serial_port);
        return;
    }

    /* Move body to the front of the buffer for easier callback use */
    for (i = 0; i < body_length; i++) {
        serial_port->rx_buffer[i] = serial_port->rx_buffer[3 + i];
    }

    if (serial_port->rx_callback != 0) {
        serial_port->rx_callback(msg_type, serial_port->rx_buffer, body_length);
    }
}
