/**
 * @file delay.h
 * @brief Blocking delay functions using <util/delay.h>.
 *
 * These are busy-wait delays. They block CPU execution.
 * Accuracy depends on F_CPU being correctly defined at compile time.
 *
 * Hardware: Timer-based busy-wait loops (compiler-generated).
 * Not interrupt-driven. Not suitable for precise timing in presence
 * of interrupts unless interrupts are disabled.
 *
 * @see <util/delay.h> for implementation details.
 */

#ifndef MICROAVR_DELAY_H
#define MICROAVR_DELAY_H

#include <util/delay.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Blocking delay in milliseconds.
 * @param ms Milliseconds to delay (max ~262ms at 16MHz due to _delay_ms limit)
 *
 * Hardware: Busy-wait loop calibrated to F_CPU.
 * Compiler: Requires F_CPU defined. Uses __builtin_avr_delay_cycles.
 *
 * Note: _delay_ms() has a maximum delay limit. For longer delays,
 * call in a loop or use delay_ms_loop().
 */
static inline void delay_ms(uint16_t ms) {
    _delay_ms(ms);
}

/**
 * @brief Blocking delay in microseconds.
 * @param us Microseconds to delay (max ~768us at 16MHz due to _delay_us limit)
 *
 * Hardware: Busy-wait loop calibrated to F_CPU.
 * Compiler: Requires F_CPU defined. Uses __builtin_avr_delay_cycles.
 */
static inline void delay_us(uint16_t us) {
    _delay_us(us);
}

/**
 * @brief Longer blocking delay in milliseconds (loop-based).
 * @param ms Milliseconds to delay (no practical upper limit)
 *
 * Use for delays > 262ms where _delay_ms() would overflow.
 */
static inline void delay_ms_long(uint16_t ms) {
    while (ms--) {
        _delay_ms(1);
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_DELAY_H */