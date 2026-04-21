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
    void (*completion_function)(uint32_t);
};

/*
 * USART1 over ST-LINK VCP on STM32F3DISCOVERY:
 * PC4 = TX
 * PC5 = RX
 * AF7
 */
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
    0x00
};

static uint16_t SerialBaudToBRR(uint32_t baudRate)
{
    uint32_t pclk = 8000000UL;
    return (uint16_t)((pclk + (baudRate / 2U)) / baudRate);
}

void SerialInitialise(uint32_t baudRate,
                      SerialPort *serial_port,
                      void (*completion_function)(uint32_t))
{
    serial_port->completion_function = completion_function;

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

    if (serial_port->completion_function != 0) {
        serial_port->completion_function(counter);
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

    if (serial_port->completion_function != 0) {
        serial_port->completion_function(length);
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

/* ===== Exercise 3c additions ===== */

static uint8_t calculateChecksum(uint8_t msg_size,
                                 uint8_t msg_type,
                                 uint8_t *msg_body)
{
    uint8_t checksum = 0;
    uint32_t i;

    checksum ^= msg_size;
    checksum ^= msg_type;

    for (i = 0; i < msg_size; i++) {
        checksum ^= msg_body[i];
    }

    return checksum;
}

void sendMsg(void *msg_struct,
             uint8_t msg_size,
             uint8_t msg_type,
             SerialPort *serial_port)
{
    uint8_t checksum;
    uint8_t *body_ptr;

    if (msg_struct == 0) {
        return;
    }

    body_ptr = (uint8_t *)msg_struct;

    /* Packet format:
       [START][SIZE][TYPE][BODY...][CHECKSUM][STOP]
    */
    SerialOutputChar(MSG_START_BYTE, serial_port);
    SerialOutputChar(msg_size, serial_port);
    SerialOutputChar(msg_type, serial_port);
    SerialOutputBytes(body_ptr, msg_size, serial_port);

    checksum = calculateChecksum(msg_size, msg_type, body_ptr);
    SerialOutputChar(checksum, serial_port);

    SerialOutputChar(MSG_STOP_BYTE, serial_port);
}
