/**
 * @file eeprom.h
 * @brief EEPROM abstraction for ATmega328P - zero-cost abstraction.
 *
 * Hardware: ATmega328P EEPROM
 *   1KB (0x0000–0x03FF)
 *   Byte read/write, timed write sequence
 *   Registers: EEARH/L (address), EEDR (data), EECR (control)
 *
 * Critical: Write requires timed sequence (EEMPE + EEPE within 4 cycles)
 *
 * Compile-time resolution: all config at compile time.
 * Zero-cost abstraction.
 */

#ifndef MICROAVR_EEPROM_H
#define MICROAVR_EEPROM_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * EEPROM Constants
 * ========================================================================== */

#define EEPROM_SIZE 1024      /* ATmega328P: 1KB */
#define EEPROM_END  0x03FF    /* Last valid address */
#define EEPROM_EMPTY 0xFF     /* Erased state */

/* ============================================================================
 * Core API (Static Inline)
 * ========================================================================== */

/**
 * @brief Check if EEPROM is ready for a new write.
 * @return true if ready (EEPE=0)
 * Usage: if (EEPROM_ready()) { ... }
 */
static inline bool EEPROM_ready(void) {
    return !(EECR & (1<<EEPE));
}

/**
 * @brief Wait for any pending write to complete.
 * Hardware: Polls EEPE bit until clear.
 * Usage: EEPROM_wait();
 */
static inline void EEPROM_wait(void) {
    while (!(EECR & (1<<EEPE)));
}

/**
 * @brief Read a byte from EEPROM address.
 * Hardware: Set EEAR, set EERE, return EEDR.
 * @param addr EEPROM address (0x0000–0x03FF)
 * @return Byte at address
 * Usage: uint8_t val = EEPROM_read(0x00);
 */
static inline uint8_t EEPROM_read(uint16_t addr) {
    EEPROM_wait();
    EEAR = addr;
    EECR |= (1<<EERE);
    return EEDR;
}

/**
 * @brief Write a byte to EEPROM address.
 * Hardware: Timed sequence EEMPE + EEPE within 4 cycles.
 * @param addr EEPROM address (0x0000–0x03FF)
 * @param data Byte to write
 * Usage: EEPROM_write(0x00, 0x42);
 */
static inline void EEPROM_write(uint16_t addr, uint8_t data) {
    uint8_t _sreg = SREG;
    cli();
    while (!(EECR & (1<<EEPE)));
    EEAR = addr;
    EEDR = data;
    EECR |= (1<<EEMPE);
    EECR |= (1<<EEPE);
    SREG = _sreg;
}

/**
 * @brief Update EEPROM byte only if different (saves wear).
 * Hardware: Compare before write.
 * @param addr EEPROM address
 * @param data New value
 * Usage: EEPROM_update(0x00, 0x43);
 */
static inline void EEPROM_update(uint16_t addr, uint8_t data) {
    if (EEPROM_read(addr) != data) {
        EEPROM_write(addr, data);
    }
}

/**
 * @brief Write a block of data to EEPROM.
 * Hardware: Loop over EEPROM_write().
 * @param addr Starting address
 * @param data Pointer to source data
 * @param len Number of bytes to write
 * Usage: EEPROM_write_block(0x10, buf, 3);
 */
static inline void EEPROM_write_block(uint16_t addr, const uint8_t *data, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        EEPROM_write(addr + i, data[i]);
    }
}

/**
 * @brief Read a block of data from EEPROM.
 * Hardware: Loop over EEPROM_read().
 * @param addr Starting address
 * @param data Pointer to destination buffer
 * @param len Number of bytes to read
 * Usage: EEPROM_read_block(0x10, buf, 3);
 */
static inline void EEPROM_read_block(uint16_t addr, uint8_t *data, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        data[i] = EEPROM_read(addr + i);
    }
}

/**
 * @brief Erase entire EEPROM (fill with 0xFF).
 * Hardware: Loop over EEPROM_write().
 * Usage: EEPROM_erase_all();
 */
static inline void EEPROM_erase_all(void) {
    for (uint16_t i = 0; i <= EEPROM_END; i++) {
        EEPROM_write(i, EEPROM_EMPTY);
    }
}

/**
 * @brief Enable EEPROM ready interrupt.
 * Hardware: EECR |= (1<<EERIE)
 * Usage: EEPROM_enable_interrupt();
 */
static inline void EEPROM_enable_interrupt(void) {
    EECR |= (1<<EERIE);
}

/**
 * @brief Disable EEPROM ready interrupt.
 * Hardware: EECR &= ~(1<<EERIE)
 * Usage: EEPROM_disable_interrupt();
 */
static inline void EEPROM_disable_interrupt(void) {
    EECR &= ~(1<<EERIE);
}

/* ============================================================================
 * ISR Generation Macro
 * ========================================================================== */

/**
 * @brief Auto-generate EEPROM ready ISR with user callback.
 * Usage:
 *   void eeprom_callback(void) { ... }
 *   EEPROM_ISR(eeprom_callback);
 */
#define EEPROM_ISR(callback) \
    static void _eeprom_isr_callback(void) { callback(); } \
    ISR(EE_READY_vect) { _eeprom_isr_callback(); }

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_EEPROM_H */