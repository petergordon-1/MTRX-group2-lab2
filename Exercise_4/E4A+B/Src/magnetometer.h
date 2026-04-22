#ifndef MAGNETOMETER_H
#define MAGNETOMETER_H

#include <stdint.h>

/* =========================
 * Sensor I2C Addresses
 * ========================= */
#define ADDR_A          (0b0011001 << 1)
#define ADDR_M          (0b0011110 << 1)

/* =========================
 * Accelerometer Registers
 * ========================= */
#define CFG_REG1_A      0x20
#define CFG_REG2_A      0x21
#define CFG_REG3_A      0x22
#define CFG_REG4_A      0x23
#define CFG_REG5_A      0x24
#define CFG_REG6_A      0x25
#define OUTX_L_A        0x28
#define OUTZ_H_A        0x2D

/* =========================
 * Magnetometer Registers
 * ========================= */
#define CFG_REG_A_M     0x60
#define CFG_REG_B_M     0x61
#define CFG_REG_C_M     0x62
#define OUTX_L_REG_M    0x68
#define OUT_H_REG_M     0x68

/* =========================
 * Data Structures
 * ========================= */
typedef struct {
    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;
    uint32_t timestamp;
    float heading;
    uint8_t flag;
} Reading_M;

typedef struct {
    int16_t a_x;
    int16_t a_y;
    int16_t a_z;
    uint32_t timestamp;
} Reading_A;

/* =========================
 * Function Prototypes
 * ========================= */
void Begin_SysTick(void);

void GPIOE_Config(void);
void GPIOB_Config(void);
void I2C1_Config(void);

void EXTI2_Config(void);
void EXTI0_Config(void);

void Config_M(void);
void Config_A(void);

void WriteToSensor(uint8_t addr, uint8_t reg, uint8_t config);
void ReadSensorData(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);

#endif /* MAGNETOMETER_H */
