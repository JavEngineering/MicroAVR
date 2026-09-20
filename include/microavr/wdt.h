/**
 * @file wdt.h
 * @brief Watchdog Timer abstraction for ATmega328P - zero-cost abstraction.
 *
 * Hardware: ATmega328P Watchdog Timer
 *   Independent ~128 kHz oscillator
 *   Timeouts: 16ms to 8s
 *   Modes: Interrupt, System Reset, Interrupt+Reset
 *   Register: WDTCSR (Watchdog Timer Control Register)
 *
 * Critical: Configuration requires timed sequence (WDCE + WDE, then config within 4 cycles)
 *
 * Compile-time resolution: all config at compile time.
 * Zero-cost abstraction.
 */

#ifndef MICROAVR_WDT_H
#define MICROAVR_WDT_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Watchdog Timeout Enum (matches WDP3:0 bits)
 * ========================================================================== */

typedef enum {
    WDT_16MS   = 0x00,  // WDP3:0 = 0000
    WDT_32MS   = 0x01,  // WDP3:0 = 0001
    WDT_64MS   = 0x02,  // WDP3:0 = 0010
    WDT_128MS  = 0x03,  // WDP3:0 = 0011
    WDT_256MS  = 0x04,  // WDP3:0 = 0100
    WDT_512MS  = 0x05,  // WDP3:0 = 0101
    WDT_1S     = 0x06,  // WDP3:0 = 0110
    WDT_2S     = 0x07,  // WDP3:0 = 0111
    WDT_4S     = 0x08,  // WDP3:0 = 1000
    WDT_8S     = 0x09,  // WDP3:0 = 1001
} wdt_timeout_t;

/* ============================================================================
 * Watchdog Mode Enum
 * ========================================================================== */

typedef enum {
    WDT_RESET      = 0,  // System reset only
    WDT_INTERRUPT  = 1,  // Interrupt only
    WDT_INT_RESET  = 2,  // Interrupt first, then reset on next timeout
} wdt_mode_t;

/* ============================================================================
 * Watchdog Configuration Macros (Timed Sequence)
 * ========================================================================== */

/**
 * @brief Timed sequence to enable WDT configuration changes.
 * Hardware: WDTCSR = (1<<WDCE)|(1<<WDE); then new config within 4 cycles.
 * Must be called with interrupts disabled for safety.
 */
#define _WDT_CONFIG_SEQ(config) \
    do { \
        uint8_t _sreg = SREG; \
        cli(); \
        WDTCSR = (1<<WDCE) | (1<<WDE); \
        WDTCSR = (config); \
        SREG = _sreg; \
    } while(0)

/* ============================================================================
 * Timeout Bits (WDP3:0)
 * ========================================================================== */

#define _WDT_TIMEOUT_BITS(timeout) \
    ((timeout) == WDT_16MS   ? 0x00 : \
     (timeout) == WDT_32MS   ? (1<<WDP0) : \
     (timeout) == WDT_64MS   ? (1<<WDP1) : \
     (timeout) == WDT_128MS  ? (1<<WDP1)|(1<<WDP0) : \
     (timeout) == WDT_256MS  ? (1<<WDP2) : \
     (timeout) == WDT_512MS  ? (1<<WDP2)|(1<<WDP0) : \
     (timeout) == WDT_1S     ? (1<<WDP2)|(1<<WDP1) : \
     (timeout) == WDT_2S     ? (1<<WDP2)|(1<<WDP1)|(1<<WDP0) : \
     (timeout) == WDT_4S     ? (1<<WDP3) : \
     (1<<WDP3)|(1<<WDP0))

/* ============================================================================
 * Mode Configuration Bits
 * ========================================================================== */

#define _WDT_MODE_BITS(mode) \
    ((mode) == WDT_RESET      ? (1<<WDE) : \
     (mode) == WDT_INTERRUPT  ? (1<<WDE)|(1<<WDIE) : \
     (1<<WDE)|(1<<WDIE)|(1<<WDIE))  // WDT_INT_RESET uses WDIE + special handling

/* ============================================================================
 * Concise API (Primary)
 * ========================================================================== */

/**
 * @brief Enable Watchdog Timer with timeout and mode.
 * Hardware: Timed sequence WDCE+WDE, then config.
 * @param timeout One of WDT_16MS ... WDT_8S
 * @param mode One of WDT_RESET, WDT_INTERRUPT, WDT_INT_RESET
 * Usage: WDT_enable(WDT_4S, WDT_INTERRUPT);
 */
static inline void WDT_enable(wdt_timeout_t timeout, wdt_mode_t mode) {
    uint8_t config = _WDT_TIMEOUT_BITS(timeout) | _WDT_MODE_BITS(mode);
    _WDT_CONFIG_SEQ(config);
}

/**
 * @brief Disable Watchdog Timer.
 * Hardware: Timed sequence to clear WDE.
 * Usage: WDT_disable();
 */
static inline void WDT_disable(void) {
    uint8_t _sreg = SREG;
    cli();
    WDTCSR = (1<<WDCE) | (1<<WDE);
    WDTCSR = 0x00;
    SREG = _sreg;
}

/**
 * @brief Reset Watchdog Timer (kick the dog).
 * Hardware: WDTCSR |= (1<<WDRF) clearing, then WDTCSR |= (1<<WDRF) is not needed.
 * Actually: asm("wdr") or WDRF bit clear.
 * Usage: WDT_reset();
 */
static inline void WDT_reset(void) {
    __asm__ __volatile__("wdr" ::: "memory");
}

/**
 * @brief Enable WDT interrupt (WDIE).
 * Hardware: WDTCSR |= (1<<WDIE)
 * Usage: WDT_isr_enable();
 */
static inline void WDT_isr_enable(void) {
    WDTCSR |= (1<<WDIE);
}

/**
 * @brief Disable WDT interrupt (WDIE).
 * Hardware: WDTCSR &= ~(1<<WDIE)
 * Usage: WDT_isr_disable();
 */
static inline void WDT_isr_disable(void) {
    WDTCSR &= ~(1<<WDIE);
}

/**
 * @brief Check if last reset was caused by Watchdog.
 * @return true if WDT caused last reset
 * Usage: if (WDT_was_reset()) { ... }
 */
static inline bool WDT_was_reset(void) {
    return (MCUSR & (1<<WDRF)) != 0;
}

/**
 * @brief Clear Watchdog reset flag.
 * Usage: WDT_clear_reset_flag();
 */
static inline void WDT_clear_reset_flag(void) {
    MCUSR &= ~(1<<WDRF);
}

/* ============================================================================
 * ISR Generation Macro
 * ========================================================================== */

/**
 * @brief Auto-generate WDT ISR with user callback.
 * Usage:
 *   void wdt_callback(void) { periodic_task(); }
 *   WDT_ISR(wdt_callback);
 */
#define WDT_ISR(callback) \
    static void _wdt_isr_callback(void) { callback(); } \
    ISR(WDT_vect) { _wdt_isr_callback(); }

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_WDT_H */