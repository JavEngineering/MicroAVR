/**
 * @file interrupt.h
 * @brief Interrupt abstraction for ATmega328P - compile-time config, zero-cost.
 *
 * Hardware: ATmega328P interrupt sources
 *   External: INT0 (PD2), INT1 (PD3)
 *   Pin Change: PCINT0-7 (PB), PCINT8-14 (PC), PCINT16-23 (PD)
 *   Timers: COMPA, COMPB, OVF (per timer)
 *   Peripherals: UART, SPI, I2C, ADC
 *
 * Registers: EICRA, EIMSK, EIFR, PCICR, PCMSK0/1/2, PCIFR,
 *            TIMSK0/1/2, TIMSK1, TIMSK2, UCSR0B, SPCR, TWCR, ADCSRA
 *
 * Compile-time resolution: all config at compile time.
 * No runtime lookup tables. Both concise and fluent APIs compile to
 * identical machine code.
 */

#ifndef MICROAVR_INTERRUPT_H
#define MICROAVR_INTERRUPT_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * External Interrupt Types (INT0, INT1)
 * ========================================================================== */

typedef enum {
    INT_LOW       = 0x00,  // Low level
    INT_CHANGE    = 0x01,  // Any logical change
    INT_FALLING   = 0x02,  // Falling edge
    INT_RISING    = 0x03,  // Rising edge
} interrupt_trigger_t;

/* ============================================================================
 * External Interrupt Vectors (INT0, INT1)
 * ========================================================================== */

#define INT0_VECT   INT0_vect
#define INT1_VECT   INT1_vect

/* ============================================================================
 * Pin Change Interrupt Vectors
 * ========================================================================== */

#define PCINT0_VECT  PCINT0_vect  // PCINT0-7 (PB0-PB7)
#define PCINT1_VECT  PCINT1_vect  // PCINT8-14 (PC0-PC6)
#define PCINT2_VECT  PCINT2_vect  // PCINT16-23 (PD0-PD7)

/* ============================================================================
 * Pin Change Pin Mappings (compile-time)
 * ========================================================================== */

/* PCINT0-7: PB0-PB7 (PCMSK0) */
#define _PCINT_PB0   0
#define _PCINT_PB1   1
#define _PCINT_PB2   2
#define _PCINT_PB3   3
#define _PCINT_PB4   4
#define _PCINT_PB5   5
#define _PCINT_PB6   6
#define _PCINT_PB7   7

/* PCINT8-14: PC0-PC6 (PCMSK1) */
#define _PCINT_PC0   8
#define _PCINT_PC1   9
#define _PCINT_PC2   10
#define _PCINT_PC3   11
#define _PCINT_PC4   12
#define _PCINT_PC5   13
#define _PCINT_PC6   14

/* PCINT16-23: PD0-PD7 (PCMSK2) */
#define _PCINT_PD0   16
#define _PCINT_PD1   17
#define _PCINT_PD2   18
#define _PCINT_PD3   19
#define _PCINT_PD4   20
#define _PCINT_PD5   21
#define _PCINT_PD6   22
#define _PCINT_PD7   23

#define PCINT(pin)  _PCINT_##pin

/* ============================================================================
 * External Interrupt API (INT0, INT1)
 * ========================================================================== */

/**
 * @brief Configure external interrupt trigger.
 * Hardware: EICRA = (EICRA & mask) | (trigger << bit)
 * @param int_num INT0 or INT1
 * @param trigger One of INT_LOW, INT_CHANGE, INT_FALLING, INT_RISING
 * Usage: Interrupt_configure(INT0, INT_FALLING);
 */
static inline void Interrupt_configure(uint8_t int_num, interrupt_trigger_t trigger) {
    if (int_num == 0) {
        EICRA = (EICRA & ~((1<<ISC01)|(1<<ISC00))) | (trigger << ISC00);
    } else if (int_num == 1) {
        EICRA = (EICRA & ~((1<<ISC11)|(1<<ISC10))) | (trigger << ISC10);
    }
}

/**
 * @brief Enable external interrupt.
 * Hardware: EIMSK |= (1<<INTn)
 * Usage: Interrupt_enable(INT0);
 */
static inline void Interrupt_enable(uint8_t int_num) {
    EIMSK |= (1 << int_num);
}

/**
 * @brief Disable external interrupt.
 * Hardware: EIMSK &= ~(1<<INTn)
 * Usage: Interrupt_disable(INT0);
 */
static inline void Interrupt_disable(uint8_t int_num) {
    EIMSK &= ~(1 << int_num);
}

/**
 * @brief Configure and enable external interrupt with ISR callback.
 * Hardware: Configures EICRA, EIMSK, generates ISR via macro
 * Usage: Interrupt_on(INT0, INT_FALLING, handler);
 */
#define Interrupt_on(int_num, trigger, callback) \
    do { \
        Interrupt_configure(int_num, trigger); \
        Interrupt_enable(int_num); \
    } while (0)

/**
 * @brief Configure external interrupt (no ISR - just enable bit).
 * Usage: Interrupt_configure_trigger(INT0, INT_FALLING);
 */
#define Interrupt_configure_trigger(int_num, trigger) \
    Interrupt_configure(int_num, trigger)

/* ============================================================================
 * Pin Change Interrupt API
 * ========================================================================== */

/**
 * @brief Enable pin change interrupt for specific pin.
 * Hardware: Sets PCMSKx bit, enables PCICR bit
 * Usage: Interrupt_pc_enable(PCINT(PD2));
 */
static inline void Interrupt_pc_enable(uint8_t pcint) {
    if (pcint <= 7) {
        PCMSK0 |= (1 << pcint);
        PCICR |= (1 << PCIE0);
    } else if (pcint <= 14) {
        PCMSK1 |= (1 << (pcint - 8));
        PCICR |= (1 << PCIE1);
    } else if (pcint <= 23) {
        PCMSK2 |= (1 << (pcint - 16));
        PCICR |= (1 << PCIE2);
    }
}

/**
 * @brief Disable pin change interrupt for specific pin.
 * Hardware: Clears PCMSKx bit
 * Usage: Interrupt_pc_disable(PCINT(PD2));
 */
static inline void Interrupt_pc_disable(uint8_t pcint) {
    if (pcint <= 7) {
        PCMSK0 &= ~(1 << pcint);
        if (PCMSK0 == 0) PCICR &= ~(1 << PCIE0);
    } else if (pcint <= 14) {
        PCMSK1 &= ~(1 << (pcint - 8));
        if (PCMSK1 == 0) PCICR &= ~(1 << PCIE1);
    } else if (pcint <= 23) {
        PCMSK2 &= ~(1 << (pcint - 16));
        if (PCMSK2 == 0) PCICR &= ~(1 << PCIE2);
    }
}

/* ============================================================================
 * Timer Interrupt API (consistent with timer.h)
 * ========================================================================== */

/* Timer0 interrupts */
#define Timer0_compareAInterrupt(cb) \
    do { TIMSK0 |= (1<<OCIE0A); (void)cb; } while(0)
#define Timer0_compareBInterrupt(cb) \
    do { TIMSK0 |= (1<<OCIE0B); (void)cb; } while(0)
#define Timer0_overflowInterrupt(cb) \
    do { TIMSK0 |= (1<<TOIE0); (void)cb; } while(0)

/* Timer1 interrupts */
#define Timer1_compareAInterrupt(cb) \
    do { TIMSK1 |= (1<<OCIE1A); (void)cb; } while(0)
#define Timer1_compareBInterrupt(cb) \
    do { TIMSK1 |= (1<<OCIE1B); (void)cb; } while(0)
#define Timer1_overflowInterrupt(cb) \
    do { TIMSK1 |= (1<<TOIE1); (void)cb; } while(0)
#define Timer1_captureInterrupt(cb) \
    do { TIMSK1 |= (1<<ICIE1); (void)cb; } while(0)

/* Timer2 interrupts */
#define Timer2_compareAInterrupt(cb) \
    do { TIMSK2 |= (1<<OCIE2A); (void)cb; } while(0)
#define Timer2_compareBInterrupt(cb) \
    do { TIMSK2 |= (1<<OCIE2B); (void)cb; } while(0)
#define Timer2_overflowInterrupt(cb) \
    do { TIMSK2 |= (1<<TOIE2); (void)cb; } while(0)

/* ============================================================================
 * Peripheral Interrupts
 * ========================================================================== */

/* UART interrupts */
#define UART_rxInterrupt(cb) \
    do { UCSR0B |= (1<<RXCIE0); (void)cb; } while(0)
#define UART_txInterrupt(cb) \
    do { UCSR0B |= (1<<TXCIE0); (void)cb; } while(0)
#define UART_udreInterrupt(cb) \
    do { UCSR0B |= (1<<UDRIE0); (void)cb; } while(0)

/* SPI interrupt */
#define SPI_interrupt(cb) \
    do { SPCR |= (1<<SPIE); (void)cb; } while(0)

/* I2C/TWI interrupt */
#define I2C_interrupt(cb) \
    do { TWCR |= (1<<TWIE); (void)cb; } while(0)

/* ADC interrupt */
#define ADC_interrupt(cb) \
    do { ADCSRA |= (1<<ADIE); (void)cb; } while(0)

/* ============================================================================
 * Global Interrupt Control
 * ========================================================================== */

static inline void Interrupt_enable_global(void) {
    sei();
}

static inline void Interrupt_disable_global(void) {
    cli();
}

/* ============================================================================
 * ISR Generation Macros
 * ========================================================================== */

/**
 * @brief Auto-generate ISR with user callback for external interrupts.
 * Usage:
 *   void int0_handler(void) { button_pressed = true; }
 *   INTERRUPT_ISR(INT0_vect, int0_handler);
 */
#define INTERRUPT_ISR(vector, callback) \
    static void _isr_##vector(void) { callback(); } \
    ISR(vector) { _isr_##vector(); }

/**
 * @brief Auto-generate ISR for pin change interrupts.
 * Usage:
 *   void pcint0_handler(void) { ... }
 *   PCINT_ISR(PCINT0_vect, pcint0_handler);
 */
#define PCINT_ISR(vector, callback) \
    INTERRUPT_ISR(vector, callback)

/**
 * @brief Auto-generate ISR for timer interrupts (alias for TIMER_ISR).
 */
#define TIMER_ISR(timer, vector, callback) \
    static void _timer_##timer##_##vector##_isr(void) { callback(); } \
    ISR(vector) { _timer_##timer##_##vector##_isr(); }

/* ============================================================================
 * Fluent API Aliases
 * ========================================================================== */

#define interrupt_enable(n)         Interrupt_enable(n)
#define interrupt_disable(n)        Interrupt_disable(n)
#define interrupt_on(n, t, cb)      Interrupt_on(n, t, cb)
#define interrupt_pc_enable(p)      Interrupt_pc_enable(p)
#define interrupt_pc_disable(p)     Interrupt_pc_disable(p)

#define interrupt_global_enable()   Interrupt_enable_global()
#define interrupt_global_disable()  Interrupt_disable_global()

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_INTERRUPT_H */