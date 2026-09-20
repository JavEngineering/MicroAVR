/**
 * @file sleep.h
 * @brief Sleep mode abstraction for ATmega328P - zero-cost abstraction.
 *
 * Hardware: ATmega328P sleep modes and BOD control
 *   Sleep modes: Idle, ADC Noise Reduction, Power-down, Power-save, Standby, Extended Standby
 *   Registers: SMCR, MCUCR (BOD), WDTCSR (WDT wake)
 *
 * Compile-time resolution: all config at compile time.
 * No runtime lookup tables. Zero-cost abstraction.
 */

#ifndef MICROAVR_SLEEP_H
#define MICROAVR_SLEEP_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Sleep Mode Enum (matches SMCR SM2:0 bits)
 * ========================================================================== */

typedef enum {
    SLEEP_IDLE = 0x00,              // Idle mode
    SLEEP_ADC_NR = 0x01,            // ADC Noise Reduction
    SLEEP_PWR_DOWN = 0x02,          // Power-down
    SLEEP_PWR_SAVE = 0x03,          // Power-save
    SLEEP_STANDBY = 0x06,           // Standby
    SLEEP_EXT_STANDBY = 0x07,       // Extended Standby
} sleep_mode_t;

/* ============================================================================
 * Sleep Mode Configuration
 * ========================================================================== */

/**
 * @brief Set sleep mode.
 * Hardware: SMCR = (SMCR & ~0x07) | mode
 * Usage: Sleep_mode(SLEEP_PWR_DOWN);
 */
static inline void Sleep_mode(sleep_mode_t mode) {
    SMCR = (SMCR & ~0x07) | mode;
}

/**
 * @brief Enable sleep mode - auto-disables unused peripherals via PRR.
 * Hardware: SMCR |= (1<<SE); PRR |= unused peripherals
 * Usage: Sleep_enable();
 */
static inline void Sleep_enable(void) {
    PRR |= (1<<PRADC) | (1<<PRSPI) | (1<<PRTIM0) | (1<<PRTIM1) | (1<<PRTIM2) | (1<<PRTWI) | (1<<PRUSART0);
    SMCR |= (1<<SE);
}

/**
 * @brief Disable sleep mode - restores PRR.
 * Hardware: SMCR &= ~(1<<SE); PRR = 0
 * Usage: Sleep_disable();
 */
static inline void Sleep_disable(void) {
    SMCR &= ~(1<<SE);
    PRR = 0;
}

/**
 * @brief Enter sleep mode.
 * Hardware: sei(); sleep_cpu(); cli();
 * If WDT is enabled (WDE=1), automatically enables WDT interrupt for wake.
 * Usage: Sleep_cpu();
 */
static inline void Sleep_cpu(void) {
    sei();
    sleep_cpu();
    cli();
}

/**
 * @brief Disable Brown-out Detector during sleep for minimum power.
 * Hardware: Timed sequence in MCUCR
 * Must be called immediately before Sleep_cpu() for effect.
 * Usage: Sleep_bod_disable(); Sleep_cpu();
 */
static inline void Sleep_bod_disable(void) {
    MCUCR = (1<<BODS) | (1<<BODSE);
    MCUCR = (1<<BODS);
}

/**
 * @brief Helper macro: Disable BOD for next sleep cycle.
 * Usage: SLEEP_BOD_DISABLE(); Sleep_cpu();
 */
#define SLEEP_BOD_DISABLE() \
    do { MCUCR = (1<<BODS)|(1<<BODSE); MCUCR = (1<<BODS); } while(0)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_SLEEP_H */