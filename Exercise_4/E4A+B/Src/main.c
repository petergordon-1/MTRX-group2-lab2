#include <stdint.h>
#include <math.h>
#include "stm32f303xc.h"
#include "magnetometer.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
#warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

/* =========================
 * Global Variables
 * ========================= */
volatile Reading_M Global_Reading_M;
volatile Reading_A Global_Reading_A;

volatile uint32_t ms = 0;
volatile uint8_t DRDY_M = 0;
volatile uint8_t DRDY_A = 0;

/* =========================
 * SysTick
 * ========================= */
/*
 * Start SysTick so it generates a regular interrupt.
 * This is used as the project millisecond time base.
 */
void Begin_SysTick(void) {
    SysTick_Config(125);
}

/*
 * SysTick interrupt handler.
 * Increment the global millisecond counter every tick.
 */
void SysTick_Handler(void) {
    ms++;
}

/* =========================
 * Sensor Configuration
 * ========================= */
/*
 * Configure the magnetometer.
 * Continuous conversion is enabled and DRDY output is turned on.
 */
void Config_M(void) {
    WriteToSensor(ADDR_M, CFG_REG_A_M, 0x00);
    WriteToSensor(ADDR_M, CFG_REG_B_M, 0b00000010);
    WriteToSensor(ADDR_M, CFG_REG_C_M, 0b00010001);
}

/*
 * Configure the accelerometer.
 * 100 Hz output, all three axes enabled, DRDY interrupt enabled,
 * and block data update/high-resolution mode selected.
 */
void Config_A(void) {
    WriteToSensor(ADDR_A, CFG_REG1_A, 0b01010111);
    WriteToSensor(ADDR_A, CFG_REG2_A, 0x00);
    WriteToSensor(ADDR_A, CFG_REG3_A, 0b00010000);
    WriteToSensor(ADDR_A, CFG_REG4_A, 0b10001000);
    WriteToSensor(ADDR_A, CFG_REG5_A, 0x00);
}

/* =========================
 * GPIO Configuration
 * ========================= */
/*
 * Configure PE0 and PE2 as inputs.
 * These pins receive the DRDY signals from the sensors.
 */
void GPIOE_Config(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    GPIOE->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER2);

    GPIOE->PUPDR &= ~(GPIO_PUPDR_PUPDR0 | GPIO_PUPDR_PUPDR2);
    GPIOE->PUPDR |=  GPIO_PUPDR_PUPDR0_1 | GPIO_PUPDR_PUPDR2_1;
}

/*
 * Configure PB6 and PB7 for I2C1.
 * PB6 = SCL, PB7 = SDA.
 */
void GPIOB_Config(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    GPIOB->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
    GPIOB->MODER |=  GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;

    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7);
    GPIOB->OTYPER |=  GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7;

    GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7);
    GPIOB->OSPEEDR |=  GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7;

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR7);
    GPIOB->PUPDR |=  GPIO_PUPDR_PUPDR6_0 | GPIO_PUPDR_PUPDR7_0;

    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFRL6 | GPIO_AFRL_AFRL7);
    GPIOB->AFR[0] |=  (4U << GPIO_AFRL_AFRL6_Pos) | (4U << GPIO_AFRL_AFRL7_Pos);
}

/* =========================
 * I2C Configuration
 * ========================= */
/*
 * Configure I2C1 for sensor communication.
 * Timing values are set for standard mode operation.
 */
void I2C1_Config(void) {
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    I2C1->CR1 &= ~I2C_CR1_PE;

    I2C1->TIMINGR =
          (0x1U  << I2C_TIMINGR_PRESC_Pos)
        | (0x13U << I2C_TIMINGR_SCLL_Pos)
        | (0x0FU << I2C_TIMINGR_SCLH_Pos)
        | (0x2U  << I2C_TIMINGR_SDADEL_Pos)
        | (0x4U  << I2C_TIMINGR_SCLDEL_Pos);

    I2C1->CR1 |= I2C_CR1_PE;
}

/* =========================
 * External Interrupt Configuration
 * ========================= */
/*
 * Configure EXTI2 for the magnetometer DRDY signal on PE2.
 */
void EXTI2_Config(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI2_Msk;
    SYSCFG->EXTICR[0] |=  SYSCFG_EXTICR1_EXTI2_PE;

    EXTI->IMR  |= EXTI_IMR_MR2;
    EXTI->RTSR |= EXTI_RTSR_TR2;
    EXTI->FTSR &= ~EXTI_FTSR_TR2;

    NVIC_SetPriority(EXTI2_TSC_IRQn, 0);
    NVIC_EnableIRQ(EXTI2_TSC_IRQn);
}

/*
 * Configure EXTI0 for the accelerometer DRDY signal on PE0.
 */
void EXTI0_Config(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0_Msk;
    SYSCFG->EXTICR[0] |=  SYSCFG_EXTICR1_EXTI0_PE;

    EXTI->IMR  |= EXTI_IMR_MR0;
    EXTI->RTSR |= EXTI_RTSR_TR0;
    EXTI->FTSR &= ~EXTI_FTSR_TR0;

    NVIC_SetPriority(EXTI0_IRQn, 1);
    NVIC_EnableIRQ(EXTI0_IRQn);
}

/*
 * EXTI interrupt handler for the magnetometer.
 * Set a flag so the main loop knows new data is ready.
 */
void EXTI2_TSC_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR2) {
        EXTI->PR |= EXTI_PR_PR2;
        DRDY_M = 1;
    }
}

/*
 * EXTI interrupt handler for the accelerometer.
 * Set a flag so the main loop knows new data is ready.
 */
void EXTI0_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR0) {
        EXTI->PR = EXTI_PR_PR0;
        DRDY_A = 1;
    }
}

/* =========================
 * I2C Sensor Access
 * ========================= */
/*
 * Write one byte of configuration data to a sensor register.
 */
void WriteToSensor(uint8_t addr, uint8_t reg, uint8_t config) {
    I2C1->CR2 =
          (addr << I2C_CR2_SADD_Pos)
        | (2U   << I2C_CR2_NBYTES_Pos)
        | (1U   << I2C_CR2_START_Pos);

    while ((I2C1->ISR & I2C_ISR_TXIS) == 0U) {
    }
    I2C1->TXDR = reg;

    while ((I2C1->ISR & I2C_ISR_TXIS) == 0U) {
    }
    I2C1->TXDR = config;

    while ((I2C1->ISR & I2C_ISR_TC) == 0U) {
    }
    I2C1->CR2 |= I2C_CR2_STOP;
}

/*
 * Read multiple bytes starting from a sensor register.
 * Auto-increment is used so consecutive output registers can be read.
 */
void ReadSensorData(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
    I2C1->CR2 =
          (addr << I2C_CR2_SADD_Pos)
        | (1U   << I2C_CR2_NBYTES_Pos)
        | (1U   << I2C_CR2_START_Pos);

    while ((I2C1->ISR & I2C_ISR_TXIS) == 0U) {
    }
    I2C1->TXDR = reg | (1U << 7);

    while ((I2C1->ISR & I2C_ISR_TC) == 0U) {
    }

    I2C1->CR2 =
          (addr << I2C_CR2_SADD_Pos)
        | (1U   << I2C_CR2_RD_WRN_Pos)
        | (len  << I2C_CR2_NBYTES_Pos)
        | (1U   << I2C_CR2_START_Pos);

    for (uint8_t i = 0; i < len; i++) {
        while ((I2C1->ISR & I2C_ISR_RXNE) == 0U) {
        }
        buf[i] = (uint8_t)I2C1->RXDR;
    }

    while ((I2C1->ISR & I2C_ISR_TC) == 0U) {
    }
    I2C1->CR2 |= I2C_CR2_STOP;
}

/* =========================
 * Main Program
 * ========================= */
int main(void) {
    SCB->CPACR |= (0xFU << 20);
    __DSB();
    __ISB();

    GPIOB_Config();
    GPIOE_Config();
    I2C1_Config();

    Config_M();
    Config_A();

    {
        uint8_t buf[6];
        ReadSensorData(ADDR_M, OUTX_L_REG_M, buf, 6);
        ReadSensorData(ADDR_A, OUTX_L_A, buf, 6);
    }

    Begin_SysTick();
    EXTI2_Config();
    EXTI0_Config();

    for (;;) {
        if (DRDY_M) {
            uint8_t buf[6];

            DRDY_M = 0;
            ReadSensorData(ADDR_M, OUTX_L_REG_M, buf, 6);

            Global_Reading_M.mag_x = (int16_t)(((uint16_t)buf[1] << 8) | buf[0]) * 15 / 100;
            Global_Reading_M.mag_y = (int16_t)(((uint16_t)buf[3] << 8) | buf[2]) * 15 / 100;
            Global_Reading_M.mag_z = (int16_t)(((uint16_t)buf[5] << 8) | buf[4]) * 15 / 100;

            Global_Reading_M.heading = atan2f((float)Global_Reading_M.mag_y,
                                              (float)Global_Reading_M.mag_x)
                                     * 180.0f / 3.14159265f;

            if (Global_Reading_M.heading < 0.0f) {
                Global_Reading_M.heading += 180.0f;
            }

            Global_Reading_M.timestamp = ms;
        }

        if (DRDY_A) {
            uint8_t buf[6];

            DRDY_A = 0;
            ReadSensorData(ADDR_A, OUTX_L_A, buf, 6);

            Global_Reading_A.a_x = (int16_t)(((uint16_t)buf[1] << 8) | buf[0]);
            Global_Reading_A.a_y = (int16_t)(((uint16_t)buf[3] << 8) | buf[2]);
            Global_Reading_A.a_z = (int16_t)(((uint16_t)buf[5] << 8) | buf[4]);

            Global_Reading_A.timestamp = ms;
        }
    }
}
