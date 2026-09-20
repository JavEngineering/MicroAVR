/**
 * @file i2c.h
 * @brief I2C/TWI abstraction for ATmega328P - compile-time config, zero-cost.
 *
 * Hardware: ATmega328P TWI (Two-Wire Interface) - I2C compatible
 *   Pins: PC4 = SDA, PC5 = SCL
 *   Registers: TWBR, TWSR, TWDR, TWCR, TWAR, TWAMR
 *
 * Modes: Master (primary), Slave
 * Speeds: 100kHz (standard), 400kHz (fast)
 *
 * Compile-time resolution: all config at compile time.
 * No runtime lookup tables. Both concise and fluent APIs compile to
 * identical machine code.
 */

#ifndef MICROAVR_I2C_H
#define MICROAVR_I2C_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * I2C Speed Prescaler Values
 * ========================================================================== */

#define I2C_PS_1    0x00  // TWPS = 00
#define I2C_PS_4    0x01  // TWPS = 01
#define I2C_PS_16   0x02  // TWPS = 10
#define I2C_PS_64   0x03  // TWPS = 11

/* ============================================================================
 * TWI Status Codes (from TWSR & 0xF8)
 * ========================================================================== */

#define I2C_START          0x08
#define I2C_REP_START      0x10
#define I2C_MT_SLA_ACK     0x18
#define I2C_MT_SLA_NACK    0x20
#define I2C_MT_DATA_ACK    0x28
#define I2C_MT_DATA_NACK   0x30
#define I2C_MR_SLA_ACK     0x40
#define I2C_MR_SLA_NACK    0x48
#define I2C_MR_DATA_ACK    0x50
#define I2C_MR_DATA_NACK   0x58

/* ============================================================================
 * Concise API (Primary) - Macros/Static Inline for Zero Overhead
 * ========================================================================== */

/**
 * @brief Initialize I2C as Master.
 * Hardware: Sets TWBR, TWSR, enables TWI.
 * @param freq_khz Clock frequency in kHz (100 or 400)
 * @param prescaler I2C_PS_1, I2C_PS_4, I2C_PS_16, I2C_PS_64
 * Usage: I2C_begin(100, I2C_PS_1);  // 100kHz
 */
static inline void I2C_begin(uint16_t freq_khz, uint8_t prescaler) {
    /* Set prescaler */
    TWSR = (TWSR & ~((1<<TWPS1)|(1<<TWPS0))) | prescaler;

    /* Calculate TWBR: SCL = F_CPU / (16 + 2*TWBR*4^TWPS) */
    uint8_t twbr = (uint8_t)((F_CPU / (1000UL * freq_khz * 2UL)) - 8);
    TWBR = twbr;

    /* Enable TWI */
    TWCR = (1<<TWEN);
}

/**
 * @brief Initialize I2C as Master at 100kHz (default).
 * Usage: I2C_begin();
 */
#define I2C_begin()  I2C_begin(100, I2C_PS_1)

/**
 * @brief Initialize I2C at 400kHz.
 * Usage: I2C_begin_fast();
 */
#define I2C_begin_fast()  I2C_begin(400, I2C_PS_1)

/**
 * @brief Disable I2C/TWI.
 * Hardware: TWEN=0
 * Usage: I2C_end();
 */
static inline void I2C_end(void) {
    TWCR &= ~(1<<TWEN);
}

/**
 * @brief Send START condition and wait for completion.
 * Hardware: TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); wait for TWINT
 * Usage: I2C_start();
 */
static inline void I2C_start(void) {
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}

/**
 * @brief Send repeated START condition.
 * Usage: I2C_rep_start();
 */
static inline void I2C_rep_start(void) {
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}

/**
 * @brief Send STOP condition.
 * Hardware: TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN)
 * Usage: I2C_stop();
 */
static inline void I2C_stop(void) {
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

/**
 * @brief Write byte to TWI data register and wait for completion.
 * @return TWSR status code
 * Usage: uint8_t status = I2C_write(0x55);
 */
static inline uint8_t I2C_write(uint8_t data) {
    TWDR = data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    return (TWSR & 0xF8);
}

/**
 * @brief Read byte with ACK.
 * Hardware: TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN); wait for TWINT
 * @return Received byte
 * Usage: uint8_t data = I2C_read_ack();
 */
static inline uint8_t I2C_read_ack(void) {
    TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

/**
 * @brief Read byte with NACK (last byte).
 * Hardware: TWCR = (1<<TWINT)|(1<<TWEN); wait for TWINT
 * @return Received byte
 * Usage: uint8_t data = I2C_read_nack();
 */
static inline uint8_t I2C_read_nack(void) {
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

/**
 * @brief Get current TWI status.
 * @return TWSR & 0xF8
 */
static inline uint8_t I2C_status(void) {
    return (TWSR & 0xF8);
}

/**
 * @brief Write to I2C device.
 * Hardware: START -> SLA+W -> data... -> STOP
 * @return true on success
 * Usage: bool ok = I2C_device_write(0x68, buf, len);
 */
static inline bool I2C_device_write(uint8_t addr, const uint8_t* data, uint8_t len) {
    I2C_start();
    if ((I2C_status() != I2C_START) && (I2C_status() != I2C_REP_START)) return false;

    I2C_write((addr << 1) | 0);  // SLA+W
    if (I2C_status() != I2C_MT_SLA_ACK) { I2C_stop(); return false; }

    while (len--) {
        I2C_write(*data++);
        if (I2C_status() != I2C_MT_DATA_ACK) { I2C_stop(); return false; }
    }

    I2C_stop();
    return true;
}

/**
 * @brief Read from I2C device.
 * Hardware: START -> SLA+W -> reg -> REP_START -> SLA+R -> data... -> STOP
 * @return true on success
 * Usage: bool ok = I2C_device_read(0x68, 0x00, buf, len);
 */
static inline bool I2C_device_read(uint8_t addr, uint8_t reg, uint8_t* data, uint8_t len) {
    I2C_start();
    if ((I2C_status() != I2C_START) && (I2C_status() != I2C_REP_START)) return false;

    I2C_write((addr << 1) | 0);  // SLA+W
    if (I2C_status() != I2C_MT_SLA_ACK) { I2C_stop(); return false; }

    I2C_write(reg);              // Register address
    if (I2C_status() != I2C_MT_DATA_ACK) { I2C_stop(); return false; }

    I2C_rep_start();
    if (I2C_status() != I2C_REP_START) { I2C_stop(); return false; }

    I2C_write((addr << 1) | 1);  // SLA+R
    if (I2C_status() != I2C_MR_SLA_ACK) { I2C_stop(); return false; }

    while (len--) {
        if (len == 0) {
            *data++ = I2C_read_nack();  // Last byte: NACK
        } else {
            *data++ = I2C_read_ack();   // Continue: ACK
        }
    }

    I2C_stop();
    return true;
}

/* ============================================================================
 * Fluent API (Optional) - PERIPHERAL -> TARGET -> ACTION grammar
 * ========================================================================== */

#define I2C_device(addr)  (addr)  /* placeholder for fluent syntax */

#define i2c_device_write(addr, data, len)  I2C_device_write(addr, data, len)
#define i2c_device_read(addr, reg, data, len)  I2C_device_read(addr, reg, data, len)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_I2C_H */