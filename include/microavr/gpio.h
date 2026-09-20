/**
 * @file gpio.h
 * @brief GPIO abstraction for ATmega328P - compile-time pin mapping, zero-cost.
 *
 * Hardware mapping (ATmega328P):
 *   Port B: PB0-PB5 (PB6/PB7 = XTAL)
 *   Port C: PC0-PC5 (PC6 = RESET)
 *   Port D: PD0-PD7
 *
 * Each pin maps to:
 *   - PORTx register (output latch)
 *   - DDRx  register (data direction)
 *   - PINx  register (input read)
 *   - Bit position in each register
 *
 * Example:
 *   PB5 -> PORTB bit 5, DDRB bit 5 (DDB5), PINB bit 5
 *
 * Compile-time resolution: all pin-to-register mapping via macros.
 * No runtime lookup tables. Both concise and fluent APIs compile to
 * identical machine code (single SBI/CBI instructions where applicable).
 */

#ifndef MICROAVR_GPIO_H
#define MICROAVR_GPIO_H

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Compile-Time Pin -> Register/Bit Mapping (Token Pasting)
 * Internal names match user-facing pin identifiers.
 * ========================================================================== */

/* --- Port B (PB0-PB5) --- */
#define _GPIO_PIN_PB0_PORT   PORTB
#define _GPIO_PIN_PB0_DDR    DDRB
#define _GPIO_PIN_PB0_PIN    PINB
#define _GPIO_PIN_PB0_BIT    0
#define _GPIO_PIN_PB0_DDRBIT DDB0

#define _GPIO_PIN_PB1_PORT   PORTB
#define _GPIO_PIN_PB1_DDR    DDRB
#define _GPIO_PIN_PB1_PIN    PINB
#define _GPIO_PIN_PB1_BIT    1
#define _GPIO_PIN_PB1_DDRBIT DDB1

#define _GPIO_PIN_PB2_PORT   PORTB
#define _GPIO_PIN_PB2_DDR    DDRB
#define _GPIO_PIN_PB2_PIN    PINB
#define _GPIO_PIN_PB2_BIT    2
#define _GPIO_PIN_PB2_DDRBIT DDB2

#define _GPIO_PIN_PB3_PORT   PORTB
#define _GPIO_PIN_PB3_DDR    DDRB
#define _GPIO_PIN_PB3_PIN    PINB
#define _GPIO_PIN_PB3_BIT    3
#define _GPIO_PIN_PB3_DDRBIT DDB3

#define _GPIO_PIN_PB4_PORT   PORTB
#define _GPIO_PIN_PB4_DDR    DDRB
#define _GPIO_PIN_PB4_PIN    PINB
#define _GPIO_PIN_PB4_BIT    4
#define _GPIO_PIN_PB4_DDRBIT DDB4

#define _GPIO_PIN_PB5_PORT   PORTB
#define _GPIO_PIN_PB5_DDR    DDRB
#define _GPIO_PIN_PB5_PIN    PINB
#define _GPIO_PIN_PB5_BIT    5
#define _GPIO_PIN_PB5_DDRBIT DDB5

/* --- Port C (PC0-PC5) --- */
#define _GPIO_PIN_PC0_PORT   PORTC
#define _GPIO_PIN_PC0_DDR    DDRC
#define _GPIO_PIN_PC0_PIN    PINC
#define _GPIO_PIN_PC0_BIT    0
#define _GPIO_PIN_PC0_DDRBIT DDC0

#define _GPIO_PIN_PC1_PORT   PORTC
#define _GPIO_PIN_PC1_DDR    DDRC
#define _GPIO_PIN_PC1_PIN    PINC
#define _GPIO_PIN_PC1_BIT    1
#define _GPIO_PIN_PC1_DDRBIT DDC1

#define _GPIO_PIN_PC2_PORT   PORTC
#define _GPIO_PIN_PC2_DDR    DDRC
#define _GPIO_PIN_PC2_PIN    PINC
#define _GPIO_PIN_PC2_BIT    2
#define _GPIO_PIN_PC2_DDRBIT DDC2

#define _GPIO_PIN_PC3_PORT   PORTC
#define _GPIO_PIN_PC3_DDR    DDRC
#define _GPIO_PIN_PC3_PIN    PINC
#define _GPIO_PIN_PC3_BIT    3
#define _GPIO_PIN_PC3_DDRBIT DDC3

#define _GPIO_PIN_PC4_PORT   PORTC
#define _GPIO_PIN_PC4_DDR    DDRC
#define _GPIO_PIN_PC4_PIN    PINC
#define _GPIO_PIN_PC4_BIT    4
#define _GPIO_PIN_PC4_DDRBIT DDC4

#define _GPIO_PIN_PC5_PORT   PORTC
#define _GPIO_PIN_PC5_DDR    DDRC
#define _GPIO_PIN_PC5_PIN    PINC
#define _GPIO_PIN_PC5_BIT    5
#define _GPIO_PIN_PC5_DDRBIT DDC5

/* --- Port D (PD0-PD7) --- */
#define _GPIO_PIN_PD0_PORT   PORTD
#define _GPIO_PIN_PD0_DDR    DDRD
#define _GPIO_PIN_PD0_PIN    PIND
#define _GPIO_PIN_PD0_BIT    0
#define _GPIO_PIN_PD0_DDRBIT DDD0

#define _GPIO_PIN_PD1_PORT   PORTD
#define _GPIO_PIN_PD1_DDR    DDRD
#define _GPIO_PIN_PD1_PIN    PIND
#define _GPIO_PIN_PD1_BIT    1
#define _GPIO_PIN_PD1_DDRBIT DDD1

#define _GPIO_PIN_PD2_PORT   PORTD
#define _GPIO_PIN_PD2_DDR    DDRD
#define _GPIO_PIN_PD2_PIN    PIND
#define _GPIO_PIN_PD2_BIT    2
#define _GPIO_PIN_PD2_DDRBIT DDD2

#define _GPIO_PIN_PD3_PORT   PORTD
#define _GPIO_PIN_PD3_DDR    DDRD
#define _GPIO_PIN_PD3_PIN    PIND
#define _GPIO_PIN_PD3_BIT    3
#define _GPIO_PIN_PD3_DDRBIT DDD3

#define _GPIO_PIN_PD4_PORT   PORTD
#define _GPIO_PIN_PD4_DDR    DDRD
#define _GPIO_PIN_PD4_PIN    PIND
#define _GPIO_PIN_PD4_BIT    4
#define _GPIO_PIN_PD4_DDRBIT DDD4

#define _GPIO_PIN_PD5_PORT   PORTD
#define _GPIO_PIN_PD5_DDR    DDRD
#define _GPIO_PIN_PD5_PIN    PIND
#define _GPIO_PIN_PD5_BIT    5
#define _GPIO_PIN_PD5_DDRBIT DDD5

#define _GPIO_PIN_PD6_PORT   PORTD
#define _GPIO_PIN_PD6_DDR    DDRD
#define _GPIO_PIN_PD6_PIN    PIND
#define _GPIO_PIN_PD6_BIT    6
#define _GPIO_PIN_PD6_DDRBIT DDD6

#define _GPIO_PIN_PD7_PORT   PORTD
#define _GPIO_PIN_PD7_DDR    DDRD
#define _GPIO_PIN_PD7_PIN    PIND
#define _GPIO_PIN_PD7_BIT    7
#define _GPIO_PIN_PD7_DDRBIT DDD7

/* ============================================================================
 * User-Facing Pin Identifiers
 * Use these in API calls: GPIO_output(PB5);
 * These are distinct from avr/io.h PB5/PD2 macros which expand to integers.
 * ========================================================================== */

/* Undefine any conflicting avr/io.h macros */
#undef PB0
#undef PB1
#undef PB2
#undef PB3
#undef PB4
#undef PB5
#undef PC0
#undef PC1
#undef PC2
#undef PC3
#undef PC4
#undef PC5
#undef PD0
#undef PD1
#undef PD2
#undef PD3
#undef PD4
#undef PD5
#undef PD6
#undef PD7

/* Define our pin identifiers as literal tokens */
#define PB0  PB0
#define PB1  PB1
#define PB2  PB2
#define PB3  PB3
#define PB4  PB4
#define PB5  PB5

#define PC0  PC0
#define PC1  PC1
#define PC2  PC2
#define PC3  PC3
#define PC4  PC4
#define PC5  PC5

#define PD0  PD0
#define PD1  PD1
#define PD2  PD2
#define PD3  PD3
#define PD4  PD4
#define PD5  PD5
#define PD6  PD6
#define PD7  PD7

/* ============================================================================
 * Pin Accessor Macros (Compile-Time Resolution - use at call site)
 * ========================================================================== */

#define GPIO_PIN_PORT(pin)    _GPIO_PIN_##pin##_PORT
#define GPIO_PIN_DDR(pin)     _GPIO_PIN_##pin##_DDR
#define GPIO_PIN_PIN(pin)     _GPIO_PIN_##pin##_PIN
#define GPIO_PIN_BIT(pin)     _GPIO_PIN_##pin##_BIT
#define GPIO_PIN_DDRBIT(pin)  _GPIO_PIN_##pin##_DDRBIT

/* ============================================================================
 * Concise API (Primary) - Macros for compile-time pin resolution
 * ========================================================================== */

/**
 * @brief Configure pin as digital output.
 * Hardware: DDRx |= (1 << DDBn);
 * Usage: GPIO_output(PB5);
 */
#define GPIO_output(pin) \
    (GPIO_PIN_DDR(pin) |= (1 << GPIO_PIN_DDRBIT(pin)))

/**
 * @brief Configure pin as digital input (high-impedance).
 * Hardware: DDRx &= ~(1 << DDBn);
 * Usage: GPIO_input(PD2);
 */
#define GPIO_input(pin) \
    (GPIO_PIN_DDR(pin) &= ~(1 << GPIO_PIN_DDRBIT(pin)))

/**
 * @brief Set pin output HIGH.
 * Hardware: PORTx |= (1 << PBn);
 * Usage: GPIO_high(PB5);
 */
#define GPIO_high(pin) \
    (GPIO_PIN_PORT(pin) |= (1 << GPIO_PIN_BIT(pin)))

/**
 * @brief Set pin output LOW.
 * Hardware: PORTx &= ~(1 << PBn);
 * Usage: GPIO_low(PB5);
 */
#define GPIO_low(pin) \
    (GPIO_PIN_PORT(pin) &= ~(1 << GPIO_PIN_BIT(pin)))

/**
 * @brief Toggle pin output.
 * Hardware: PINx |= (1 << PBn);  (writing 1 to PINx toggles PORTx on ATmega328P)
 * Usage: GPIO_toggle(PB5);
 */
#define GPIO_toggle(pin) \
    (GPIO_PIN_PIN(pin) |= (1 << GPIO_PIN_BIT(pin)))

/**
 * @brief Read pin input state.
 * Hardware: (PINx & (1 << PBn)) != 0
 * Usage: bool state = GPIO_read(PD2);
 * @return true if HIGH, false if LOW
 */
#define GPIO_read(pin) \
    ((GPIO_PIN_PIN(pin) & (1 << GPIO_PIN_BIT(pin))) != 0)

/**
 * @brief Enable internal pull-up resistor (pin must be input mode).
 * Hardware: PORTx |= (1 << PBn);  (while DDRx bit = 0)
 * Usage: GPIO_pullup(PD2);
 */
#define GPIO_pullup(pin) \
    (GPIO_PIN_PORT(pin) |= (1 << GPIO_PIN_BIT(pin)))

/* ============================================================================
 * Fluent API (Optional) - PERIPHERAL -> TARGET -> ACTION grammar
 * Same underlying macros, different naming convention.
 * Usage: gpio_pin_output(PB5);  // expands to same as GPIO_output(PB5)
 * ========================================================================== */

#define gpio_pin_output(pin)   GPIO_output(pin)
#define gpio_pin_input(pin)    GPIO_input(pin)
#define gpio_pin_high(pin)     GPIO_high(pin)
#define gpio_pin_low(pin)      GPIO_low(pin)
#define gpio_pin_toggle(pin)   GPIO_toggle(pin)
#define gpio_pin_read(pin)     GPIO_read(pin)
#define gpio_pin_pullup(pin)   GPIO_pullup(pin)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_GPIO_H */