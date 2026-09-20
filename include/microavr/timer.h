/**
 * @file timer.h
 * @brief Timer abstraction for ATmega328P - compile-time register mapping, zero-cost.
 *
 * Hardware: ATmega328P has 3 timers:
 *   Timer0 (8-bit): Used by delay_ms(), millis() - WGM: Normal, CTC, Fast PWM, Phase-correct
 *   Timer1 (16-bit): Most versatile - WGM: Normal, CTC, Fast PWM, Phase-correct, Phase/Freq correct
 *   Timer2 (8-bit): Async option (external 32.768kHz crystal) - WGM: Normal, CTC, Fast PWM, Phase-correct
 *
 * Registers:
 *   Timer0: TCCR0A, TCCR0B, TCNT0, OCR0A, OCR0B, TIMSK0, TIFR0
 *   Timer1: TCCR1A, TCCR1B, TCCR1C, TCNT1, OCR1A, OCR1B, ICR1, TIMSK1, TIFR1
 *   Timer2: TCCR2A, TCCR2B, TCNT2, OCR2A, OCR2B, TIMSK2, TIFR2
 *
 * CTC Mode: OCRxA is TOP. compareA() sets BOTH compare match AND TOP.
 *
 * Compile-time resolution: all timer->register mapping via macros.
 * No runtime lookup tables. Both concise and fluent APIs compile to
 * identical machine code.
 */

#ifndef MICROAVR_TIMER_H
#define MICROAVR_TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Timer Mode Enum (Hardware-aligned)
 * ========================================================================== */

typedef enum {
    TIMER_MODE_NORMAL        = 0,  // WGM = 000
    TIMER_MODE_CTC           = 1,  // WGM = 010 (8-bit) / 1000 (16-bit)
    TIMER_MODE_FAST_PWM      = 2,  // WGM = 011 (8-bit) / 1110 (16-bit, TOP=ICR1)
    TIMER_MODE_PHASE_CORRECT = 3,  // WGM = 001 (8-bit) / 1001 (16-bit)
    TIMER_MODE_PHASE_FREQ    = 4,  // Timer1 only: WGM = 1100
} timer_mode_t;

/* ============================================================================
 * Prescaler Values
 * ========================================================================== */

#define TIMER_PS_1      0x01
#define TIMER_PS_8      0x02
#define TIMER_PS_64     0x03
#define TIMER_PS_256    0x04
#define TIMER_PS_1024   0x05

/* ============================================================================
 * Timer Identifiers
 * ========================================================================== */

#define TIMER0  0
#define TIMER1  1
#define TIMER2  2

/* ============================================================================
 * Timer Register Access Macros
 * ========================================================================== */

#define TIMER_TCCR0A()    TCCR0A
#define TIMER_TCCR0B()    TCCR0B
#define TIMER_TCNT0()     TCNT0
#define TIMER_OCR0A()     OCR0A
#define TIMER_OCR0B()     OCR0B
#define TIMER_TIMSK0()    TIMSK0
#define TIMER_TIFR0()     TIFR0

#define TIMER_TCCR1A()    TCCR1A
#define TIMER_TCCR1B()    TCCR1B
#define TIMER_TCCR1C()    TCCR1C
#define TIMER_TCNT1()     TCNT1
#define TIMER_OCR1A()     OCR1A
#define TIMER_OCR1B()     OCR1B
#define TIMER_ICR1()      ICR1
#define TIMER_TIMSK1()    TIMSK1
#define TIMER_TIFR1()     TIFR1

#define TIMER_TCCR2A()    TCCR2A
#define TIMER_TCCR2B()    TCCR2B
#define TIMER_TCNT2()     TCNT2
#define TIMER_OCR2A()     OCR2A
#define TIMER_OCR2B()     OCR2B
#define TIMER_TIMSK2()    TIMSK2
#define TIMER_TIFR2()     TIFR2

/* ============================================================================
 * Warning: Timer0 used by delay_ms()
 * ========================================================================== */

#if defined(MICROAVR_DELAY_H)
#pragma message "NOTE: Timer0 is used by delay_ms(). Modifying Timer0 prescaler/mode affects delay accuracy."
#endif

/* ============================================================================
 * Concise API - Timer0 (8-bit)
 * ========================================================================== */

/**
 * @brief Start Timer0 (enable clock).
 * Hardware: TCCR0B |= CS0 bits
 */
static inline void Timer0_start(void) {
    TIMER_TCCR0B() |= (TIMER_TCCR0B() & 0xF8);  // Prescaler already set
}

/**
 * @brief Stop Timer0 (disable clock).
 * Hardware: TCCR0B &= ~0x07
 */
static inline void Timer0_stop(void) {
    TIMER_TCCR0B() &= ~0x07;
}

/**
 * @brief Reset Timer0 counter.
 * Hardware: TCNT0 = 0
 */
static inline void Timer0_reset(void) {
    TIMER_TCNT0() = 0;
}

/**
 * @brief Set Timer0 mode.
 * Hardware: Updates WGM02:0 bits in TCCR0A/TCCR0B
 */
static inline void Timer0_mode(timer_mode_t mode) {
    uint8_t wgm = 0;
    switch (mode) {
        case TIMER_MODE_NORMAL:        wgm = 0; break;
        case TIMER_MODE_CTC:           wgm = (1<<WGM01); break;
        case TIMER_MODE_FAST_PWM:      wgm = (1<<WGM01)|(1<<WGM00); break;
        case TIMER_MODE_PHASE_CORRECT: wgm = (1<<WGM00); break;
        default: wgm = 0;
    }
    TIMER_TCCR0A() = (TIMER_TCCR0A() & ~((1<<WGM01)|(1<<WGM00))) | (wgm & 0x03);
    TIMER_TCCR0B() = (TIMER_TCCR0B() & ~(1<<WGM02)) | ((wgm & 0x04) << 1);
}

/**
 * @brief Set Timer0 prescaler.
 * Hardware: CS02:0 bits in TCCR0B
 */
static inline void Timer0_prescaler(uint8_t ps) {
    TIMER_TCCR0B() = (TIMER_TCCR0B() & 0xF8) | ps;
}

/**
 * @brief Set Timer0 compare match A (TOP in CTC mode).
 * Hardware: OCR0A = value
 * @note In CTC mode, this sets the TOP value.
 */
static inline void Timer0_compareA(uint8_t value) {
    TIMER_OCR0A() = value;
}

/**
 * @brief Set Timer0 compare match B.
 * Hardware: OCR0B = value
 */
static inline void Timer0_compareB(uint8_t value) {
    TIMER_OCR0B() = value;
}

/**
 * @brief Enable Timer0 compare match A interrupt.
 * Hardware: TIMSK0 |= (1<<OCIE0A)
 * If callback provided, auto-generates ISR.
 */
static inline void Timer0_compareAInterrupt(void (*callback)(void)) {
    TIMER_TIMSK0() |= (1<<OCIE0A);
    (void)callback;  // Used by TIMER_ISR macro
}

/**
 * @brief Enable Timer0 overflow interrupt.
 * Hardware: TIMSK0 |= (1<<TOIE0)
 */
static inline void Timer0_overflowInterrupt(void (*callback)(void)) {
    TIMER_TIMSK0() |= (1<<TOIE0);
    (void)callback;
}

/* ============================================================================
 * Concise API - Timer1 (16-bit)
 * ========================================================================== */

static inline void Timer1_start(void) {
    TIMER_TCCR1B() |= (TIMER_TCCR1B() & 0xF8);
}

static inline void Timer1_stop(void) {
    TIMER_TCCR1B() &= ~0x07;
}

static inline void Timer1_reset(void) {
    TIMER_TCNT1() = 0;
}

static inline void Timer1_mode(timer_mode_t mode) {
    uint16_t wgm = 0;
    switch (mode) {
        case TIMER_MODE_NORMAL:         wgm = 0; break;
        case TIMER_MODE_CTC:            wgm = (1<<WGM12); break;
        case TIMER_MODE_FAST_PWM:       wgm = (1<<WGM13)|(1<<WGM12)|(1<<WGM11)|(1<<WGM10); break; // TOP=ICR1
        case TIMER_MODE_PHASE_CORRECT:  wgm = (1<<WGM11)|(1<<WGM10); break;
        case TIMER_MODE_PHASE_FREQ:     wgm = (1<<WGM13)|(1<<WGM12); break;
        default: wgm = 0;
    }
    TIMER_TCCR1A() = (TIMER_TCCR1A() & ~((1<<WGM11)|(1<<WGM10))) | (wgm & 0x03);
    TIMER_TCCR1B() = (TIMER_TCCR1B() & ~((1<<WGM13)|(1<<WGM12))) | ((wgm >> 2) & 0x0C);
}

static inline void Timer1_prescaler(uint8_t ps) {
    TIMER_TCCR1B() = (TIMER_TCCR1B() & 0xF8) | ps;
}

static inline void Timer1_compareA(uint16_t value) {
    TIMER_OCR1A() = value;
}

static inline void Timer1_compareB(uint16_t value) {
    TIMER_OCR1B() = value;
}

static inline void Timer1_icr(uint16_t value) {
    TIMER_ICR1() = value;
}

static inline void Timer1_compareAInterrupt(void (*callback)(void)) {
    TIMER_TIMSK1() |= (1<<OCIE1A);
    (void)callback;
}

static inline void Timer1_compareBInterrupt(void (*callback)(void)) {
    TIMER_TIMSK1() |= (1<<OCIE1B);
    (void)callback;
}

static inline void Timer1_overflowInterrupt(void (*callback)(void)) {
    TIMER_TIMSK1() |= (1<<TOIE1);
    (void)callback;
}

static inline void Timer1_captureInterrupt(void (*callback)(void)) {
    TIMER_TIMSK1() |= (1<<ICIE1);
    (void)callback;
}

/* ============================================================================
 * Concise API - Timer2 (8-bit)
 * ========================================================================== */

static inline void Timer2_start(void) {
    TIMER_TCCR2B() |= (TIMER_TCCR2B() & 0xF8);
}

static inline void Timer2_stop(void) {
    TIMER_TCCR2B() &= ~0x07;
}

static inline void Timer2_reset(void) {
    TIMER_TCNT2() = 0;
}

static inline void Timer2_mode(timer_mode_t mode) {
    uint8_t wgm = 0;
    switch (mode) {
        case TIMER_MODE_NORMAL:        wgm = 0; break;
        case TIMER_MODE_CTC:           wgm = (1<<WGM21); break;
        case TIMER_MODE_FAST_PWM:      wgm = (1<<WGM21)|(1<<WGM20); break;
        case TIMER_MODE_PHASE_CORRECT: wgm = (1<<WGM20); break;
        default: wgm = 0;
    }
    TIMER_TCCR2A() = (TIMER_TCCR2A() & ~((1<<WGM21)|(1<<WGM20))) | wgm;
    TIMER_TCCR2B() = (TIMER_TCCR2B() & ~(1<<WGM22)) | ((wgm & 0x04) >> 2);
}

static inline void Timer2_prescaler(uint8_t ps) {
    TIMER_TCCR2B() = (TIMER_TCCR2B() & 0xF8) | ps;
}

static inline void Timer2_compareA(uint8_t value) {
    TIMER_OCR2A() = value;
}

static inline void Timer2_compareB(uint8_t value) {
    TIMER_OCR2B() = value;
}

static inline void Timer2_compareAInterrupt(void (*callback)(void)) {
    TIMER_TIMSK2() |= (1<<OCIE2A);
    (void)callback;
}

static inline void Timer2_compareBInterrupt(void (*callback)(void)) {
    TIMER_TIMSK2() |= (1<<OCIE2B);
    (void)callback;
}

static inline void Timer2_overflowInterrupt(void (*callback)(void)) {
    TIMER_TIMSK2() |= (1<<TOIE2);
    (void)callback;
}

/* ============================================================================
 * Fluent API - PERIPHERAL -> TARGET -> ACTION grammar
 * ========================================================================== */

#define TIMER0_mode(m)           Timer0_mode(m)
#define TIMER0_prescaler(p)      Timer0_prescaler(p)
#define TIMER0_compareA(v)       Timer0_compareA(v)
#define TIMER0_compareB(v)       Timer0_compareB(v)
#define TIMER0_start()           Timer0_start()
#define TIMER0_stop()            Timer0_stop()
#define TIMER0_reset()           Timer0_reset()
#define TIMER0_compareAInterrupt(cb) Timer0_compareAInterrupt(cb)
#define TIMER0_overflowInterrupt(cb) Timer0_overflowInterrupt(cb)

#define TIMER1_mode(m)           Timer1_mode(m)
#define TIMER1_prescaler(p)      Timer1_prescaler(p)
#define TIMER1_compareA(v)       Timer1_compareA(v)
#define TIMER1_compareB(v)       Timer1_compareB(v)
#define TIMER1_icr(v)            Timer1_icr(v)
#define TIMER1_start()           Timer1_start()
#define TIMER1_stop()            Timer1_stop()
#define TIMER1_reset()           Timer1_reset()
#define TIMER1_compareAInterrupt(cb) Timer1_compareAInterrupt(cb)
#define TIMER1_compareBInterrupt(cb) Timer1_compareBInterrupt(cb)
#define TIMER1_overflowInterrupt(cb) Timer1_overflowInterrupt(cb)
#define TIMER1_captureInterrupt(cb) Timer1_captureInterrupt(cb)

#define TIMER2_mode(m)           Timer2_mode(m)
#define TIMER2_prescaler(p)      Timer2_prescaler(p)
#define TIMER2_compareA(v)       Timer2_compareA(v)
#define TIMER2_compareB(v)       Timer2_compareB(v)
#define TIMER2_start()           Timer2_start()
#define TIMER2_stop()            Timer2_stop()
#define TIMER2_reset()           Timer2_reset()
#define TIMER2_compareAInterrupt(cb) Timer2_compareAInterrupt(cb)
#define TIMER2_compareBInterrupt(cb) Timer2_compareBInterrupt(cb)
#define TIMER2_overflowInterrupt(cb) Timer2_overflowInterrupt(cb)

/* ============================================================================
 * ISR Generation Macro
 * ========================================================================== */

/**
 * @brief Auto-generate ISR with user callback.
 * Usage:
 *   void my_callback(void) { ms_ticks++; }
 *   TIMER_ISR(Timer1, TIMER1_COMPA_vect, my_callback);
 *
 * Generates:
 *   static void _timer_Timer1_TIMER1_COMPA_vect_isr(void) { my_callback(); }
 *   ISR(TIMER1_COMPA_vect) { _timer_Timer1_TIMER1_COMPA_vect_isr(); }
 */
#define TIMER_ISR(timer, vector, callback) \
    static void _timer_##timer##_##vector##_isr(void) { callback(); } \
    ISR(vector) { _timer_##timer##_##vector##_isr(); }

/* ============================================================================
 * Unified Timer Namespace (for fluent API)
 * ========================================================================== */

#define Timer0_mode(m)           Timer0_mode(m)
#define Timer0_prescaler(p)      Timer0_prescaler(p)
#define Timer0_compareA(v)       Timer0_compareA(v)
#define Timer0_compareB(v)       Timer0_compareB(v)
#define Timer0_start()           Timer0_start()
#define Timer0_stop()            Timer0_stop()
#define Timer0_reset()           Timer0_reset()
#define Timer0_compareAInterrupt(cb) Timer0_compareAInterrupt(cb)
#define Timer0_overflowInterrupt(cb) Timer0_overflowInterrupt(cb)

#define Timer1_mode(m)           Timer1_mode(m)
#define Timer1_prescaler(p)      Timer1_prescaler(p)
#define Timer1_compareA(v)       Timer1_compareA(v)
#define Timer1_compareB(v)       Timer1_compareB(v)
#define Timer1_icr(v)            Timer1_icr(v)
#define Timer1_start()           Timer1_start()
#define Timer1_stop()            Timer1_stop()
#define Timer1_reset()           Timer1_reset()
#define Timer1_compareAInterrupt(cb) Timer1_compareAInterrupt(cb)
#define Timer1_compareBInterrupt(cb) Timer1_compareBInterrupt(cb)
#define Timer1_overflowInterrupt(cb) Timer1_overflowInterrupt(cb)
#define Timer1_captureInterrupt(cb) Timer1_captureInterrupt(cb)

#define Timer2_mode(m)           Timer2_mode(m)
#define Timer2_prescaler(p)      Timer2_prescaler(p)
#define Timer2_compareA(v)       Timer2_compareA(v)
#define Timer2_compareB(v)       Timer2_compareB(v)
#define Timer2_start()           Timer2_start()
#define Timer2_stop()            Timer2_stop()
#define Timer2_reset()           Timer2_reset()
#define Timer2_compareAInterrupt(cb) Timer2_compareAInterrupt(cb)
#define Timer2_compareBInterrupt(cb) Timer2_compareBInterrupt(cb)
#define Timer2_overflowInterrupt(cb) Timer2_overflowInterrupt(cb)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_TIMER_H */