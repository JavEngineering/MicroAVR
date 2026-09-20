/**
 * @file power.h
 * @brief Power reduction abstraction for ATmega328P - zero-cost abstraction.
 *
 * Hardware: ATmega328P Power Reduction Register (PRR)
 *   Disables clock to unused peripherals for power savings.
 *
 * Compile-time resolution: all config at compile time.
 * Zero-cost abstraction.
 */

#ifndef MICROAVR_POWER_H
#define MICROAVR_POWER_H

#include <avr/io.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Individual Peripheral Power Reduction
 * ========================================================================== */

static inline void Power_reduce_adc(void)    { PRR |= (1<<PRADC); }
static inline void Power_reduce_spi(void)    { PRR |= (1<<PRSPI); }
static inline void Power_reduce_timer0(void) { PRR |= (1<<PRTIM0); }
static inline void Power_reduce_timer1(void) { PRR |= (1<<PRTIM1); }
static inline void Power_reduce_timer2(void) { PRR |= (1<<PRTIM2); }
static inline void Power_reduce_twi(void)    { PRR |= (1<<PRTWI); }
static inline void Power_reduce_uart(void)   { PRR |= (1<<PRUSART0); }

/**
 * @brief Enable clock to peripheral.
 */
static inline void Power_enable_adc(void)    { PRR &= ~(1<<PRADC); }
static inline void Power_enable_spi(void)    { PRR &= ~(1<<PRSPI); }
static inline void Power_enable_timer0(void) { PRR &= ~(1<<PRTIM0); }
static inline void Power_enable_timer1(void) { PRR &= ~(1<<PRTIM1); }
static inline void Power_enable_timer2(void) { PRR &= ~(1<<PRTIM2); }
static inline void Power_enable_twi(void)    { PRR &= ~(1<<PRTWI); }
static inline void Power_enable_uart(void)   { PRR &= ~(1<<PRUSART0); }

/* ============================================================================
 * Bulk Power Control
 * ========================================================================== */

/**
 * @brief Disable all unused peripherals - called by Sleep_enable().
 * Hardware: PRR = 0xFF (all bits set)
 */
static inline void Power_reduce_all_unused(void) {
    PRR = (1<<PRADC) | (1<<PRSPI) | (1<<PRTIM0) | (1<<PRTIM1) | (1<<PRTIM2) | (1<<PRTWI) | (1<<PRUSART0);
}

/**
 * @brief Restore all peripherals - called by Sleep_disable().
 * Hardware: PRR = 0
 */
static inline void Power_restore_all(void) {
    PRR = 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_POWER_H */