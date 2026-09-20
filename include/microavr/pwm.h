/**
 * @file pwm.h
 * @brief PWM abstraction for ATmega328P - compile-time pin->timer mapping, zero-cost.
 *
 * Hardware: ATmega328P has 6 hardware PWM channels across 3 timers:
 *   Timer0 (8-bit): OC0A -> PD6 (Arduino D6),  OC0B -> PD5 (Arduino D5)
 *   Timer1 (16-bit): OC1A -> PB1 (Arduino D9), OC1B -> PB2 (Arduino D10)
 *   Timer2 (8-bit): OC2A -> PB3 (Arduino D11), OC2B -> PD3 (Arduino D3)
 *
 * Only these 6 pins support hardware PWM. Other pins cannot do hardware PWM.
 *
 * Registers per timer:
 *   Timer0: TCCR0A, TCCR0B, OCR0A, OCR0B
 *   Timer1: TCCR1A, TCCR1B, OCR1A, OCR1B, ICR1
 *   Timer2: TCCR2A, TCCR2B, OCR2A, OCR2B
 *
 * Mode: Fast PWM (8-bit for Timer0/2, 16-bit for Timer1)
 * Frequency: f_PWM = F_CPU / (prescaler * (TOP + 1))
 *   8-bit Fast PWM: TOP = 0xFF (255)
 *   16-bit Fast PWM: TOP = ICR1 (configurable)
 *
 * Compile-time resolution: all pin->timer/channel mapping via macros.
 * No runtime lookup tables. Both concise and fluent APIs compile to
 * identical machine code.
 */

#ifndef MICROAVR_PWM_H
#define MICROAVR_PWM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Pin -> Timer/Register Mapping (Token Pasting - Compile-Time Resolution)
 * ========================================================================== */

/* --- Timer0 (8-bit) --- */
#define _PWM_PD5_TIMER       TIMER0
#define _PWM_PD5_CHANNEL     CHANNEL_B
#define _PWM_PD5_OCR         OCR0B
#define _PWM_PD5_COM_MASK    (1<<COM0B1)
#define _PWM_PD5_DDR         DDRD
#define _PWM_PD5_DDR_BIT     DDD5

#define _PWM_PD6_TIMER       TIMER0
#define _PWM_PD6_CHANNEL     CHANNEL_A
#define _PWM_PD6_OCR         OCR0A
#define _PWM_PD6_COM_MASK    (1<<COM0A1)
#define _PWM_PD6_DDR         DDRD
#define _PWM_PD6_DDR_BIT     DDD6

/* --- Timer1 (16-bit) --- */
#define _PWM_PB1_TIMER       TIMER1
#define _PWM_PB1_CHANNEL     CHANNEL_A
#define _PWM_PB1_OCR         OCR1A
#define _PWM_PB1_COM_MASK    (1<<COM1A1)
#define _PWM_PB1_DDR         DDRB
#define _PWM_PB1_DDR_BIT     DDB1

#define _PWM_PB2_TIMER       TIMER1
#define _PWM_PB2_CHANNEL     CHANNEL_B
#define _PWM_PB2_OCR         OCR1B
#define _PWM_PB2_COM_MASK    (1<<COM1B1)
#define _PWM_PB2_DDR         DDRB
#define _PWM_PB2_DDR_BIT     DDB2

/* --- Timer2 (8-bit) --- */
#define _PWM_PB3_TIMER       TIMER2
#define _PWM_PB3_CHANNEL     CHANNEL_A
#define _PWM_PB3_OCR         OCR2A
#define _PWM_PB3_COM_MASK    (1<<COM2A1)
#define _PWM_PB3_DDR         DDRB
#define _PWM_PB3_DDR_BIT     DDB3

#define _PWM_PD3_TIMER       TIMER2
#define _PWM_PD3_CHANNEL     CHANNEL_B
#define _PWM_PD3_OCR         OCR2B
#define _PWM_PD3_COM_MASK    (1<<COM2B1)
#define _PWM_PD3_DDR         DDRD
#define _PWM_PD3_DDR_BIT     DDD3

/* ============================================================================
 * Pin Accessor Macros (Compile-Time Resolution via Token Pasting)
 * ========================================================================== */

#define PWM_PIN_OCR(pin)         _PWM_##pin##_OCR
#define PWM_PIN_COM_MASK(pin)    _PWM_##pin##_COM_MASK
#define PWM_PIN_DDR(pin)         _PWM_##pin##_DDR
#define PWM_PIN_DDR_BIT(pin)     _PWM_##pin##_DDR_BIT

/* Timer register access */
#define PWM_TCCR0A()    TCCR0A
#define PWM_TCCR0B()    TCCR0B
#define PWM_TCCR1A()    TCCR1A
#define PWM_TCCR1B()    TCCR1B
#define PWM_TCCR2A()    TCCR2A
#define PWM_TCCR2B()    TCCR2B

/* ============================================================================
 * Prescaler Values
 * ========================================================================== */

#define PWM_PS_1      0x01
#define PWM_PS_8      0x02
#define PWM_PS_64     0x03
#define PWM_PS_256    0x04
#define PWM_PS_1024   0x05

/* ============================================================================
 * Concise API (Primary) - Macros for Zero Overhead
 * ========================================================================== */

/**
 * @brief Enable PWM output on pin (Fast PWM, non-inverting).
 * Hardware: Sets DDR, configures TCCRx for Fast PWM, enables output compare.
 * Usage: PWM_enable(PD6);
 */
#define PWM_enable(pin) \
    do { \
        /* Set pin as output */ \
        PWM_PIN_DDR(pin) |= (1 << PWM_PIN_DDR_BIT(pin)); \
        /* Configure timer for Fast PWM, non-inverting */ \
        _PWM_configure_timer(pin); \
        /* Enable output compare (non-inverting) */ \
        _PWM_enable_output(pin); \
    } while (0)

/**
 * @brief Disable PWM output on pin.
 * Hardware: Disables output compare, pin becomes normal I/O.
 * Usage: PWM_disable(PD6);
 */
#define PWM_disable(pin) \
    do { \
        _PWM_disable_output(pin); \
    } while (0)

/**
 * @brief Set PWM duty cycle.
 * Hardware: Writes to OCRx register (8-bit or 16-bit).
 * For 8-bit timers (Timer0/2): 0-255
 * For 16-bit timer (Timer1): 0-65535
 * Usage: PWM_duty(PD6, 128);
 */
#define PWM_duty(pin, duty) \
    (PWM_PIN_OCR(pin) = (duty))

/**
 * @brief Set PWM frequency (approximate).
 * Hardware: Calculates best prescaler and TOP value, updates TCCRx and ICR1 (Timer1).
 * For 8-bit timers: TOP fixed at 255, adjusts prescaler only.
 * For 16-bit timer: Adjusts prescaler and ICR1 for best match.
 * Usage: PWM_frequency(PD6, 1000);  // ~1kHz
 */
#define PWM_frequency(pin, hz) \
    _PWM_set_frequency(pin, hz)

/* ============================================================================
 * Internal Helper Macros
 * ========================================================================== */

#define _PWM_configure_timer(pin) \
    do { \
        if (_PWM_IS_TIMER0(pin)) { \
            PWM_TCCR0A() = (PWM_TCCR0A() & ~((1<<WGM01)|(1<<WGM00))) | ((1<<WGM01)|(1<<WGM00)); \
            PWM_TCCR0B() = (PWM_TCCR0B() & ~(1<<WGM02)) | ((1<<CS01)|(1<<CS00)); \
        } else if (_PWM_IS_TIMER1(pin)) { \
            PWM_TCCR1A() = (PWM_TCCR1A() & ~((1<<WGM11)|(1<<WGM10))) | ((1<<WGM11)|(1<<WGM10)); \
            PWM_TCCR1B() = (PWM_TCCR1B() & ~((1<<WGM13)|(1<<WGM12)|0x07)) | ((1<<WGM13)|(1<<WGM12)|(1<<WGM11)|(1<<WGM10)) | ((1<<CS11)|(1<<CS10)); \
            ICR1 = 0xFFFF; \
        } else if (_PWM_IS_TIMER2(pin)) { \
            PWM_TCCR2A() = (PWM_TCCR2A() & ~((1<<WGM21)|(1<<WGM20))) | ((1<<WGM21)|(1<<WGM20)); \
            PWM_TCCR2B() = (PWM_TCCR2B() & ~(1<<WGM22)) | ((1<<CS21)|(1<<CS20)); \
        } \
    } while (0)

#define _PWM_enable_output(pin) \
    do { \
        if (_PWM_IS_TIMER0(pin)) { \
            PWM_TCCR0A() |= PWM_PIN_COM_MASK(pin); \
        } else if (_PWM_IS_TIMER1(pin)) { \
            PWM_TCCR1A() |= PWM_PIN_COM_MASK(pin); \
        } else if (_PWM_IS_TIMER2(pin)) { \
            PWM_TCCR2A() |= PWM_PIN_COM_MASK(pin); \
        } \
    } while (0)

#define _PWM_disable_output(pin) \
    do { \
        if (_PWM_IS_TIMER0(pin)) { \
            PWM_TCCR0A() &= ~PWM_PIN_COM_MASK(pin); \
        } else if (_PWM_IS_TIMER1(pin)) { \
            PWM_TCCR1A() &= ~PWM_PIN_COM_MASK(pin); \
        } else if (_PWM_IS_TIMER2(pin)) { \
            PWM_TCCR2A() &= ~PWM_PIN_COM_MASK(pin); \
        } \
    } while (0)

#define _PWM_set_frequency(pin, hz) \
    do { \
        uint32_t _f = (hz); \
        (void)_f; \
        if (_PWM_IS_TIMER0(pin)) { \
            _PWM_set_freq_timer0(_f); \
        } else if (_PWM_IS_TIMER1(pin)) { \
            _PWM_set_freq_timer1(_f); \
        } else if (_PWM_IS_TIMER2(pin)) { \
            _PWM_set_freq_timer2(_f); \
        } \
    } while (0)

/* Timer identification via token pasting */
#define _PWM_IS_TIMER0(pin)  (_PWM_##pin##_TIMER == TIMER0)
#define _PWM_IS_TIMER1(pin)  (_PWM_##pin##_TIMER == TIMER1)
#define _PWM_IS_TIMER2(pin)  (_PWM_##pin##_TIMER == TIMER2)

/* Timer constants */
#define TIMER0  0
#define TIMER1  1
#define TIMER2  2
#define CHANNEL_A 0
#define CHANNEL_B 1

/* Fast PWM 8-bit: WGM = 011 (TOP=0xFF) */
#define _PWM_WGM_FAST_8BIT()  ((1<<WGM01)|(1<<WGM00))

/* Fast PWM 16-bit: WGM = 1110 (TOP=ICR1) */
#define _PWM_WGM_FAST_16BIT()  ((1<<WGM13)|(1<<WGM12)|(1<<WGM11)|(1<<WGM10))

/* Default prescaler: 64 */
#define _PWM_CS_64_TIMER0()  ((1<<CS01)|(1<<CS00))
#define _PWM_CS_64_TIMER1()  ((1<<CS11)|(1<<CS10))
#define _PWM_CS_64_TIMER2()  ((1<<CS21)|(1<<CS20))

/* ============================================================================
 * Frequency Calculation
 * ========================================================================== */

#define _PWM_set_freq_timer0(freq) \
    do { \
        uint32_t _f = (freq); \
        uint8_t _ps; \
        if (_f >= 61000) _ps = PWM_PS_1; \
        else if (_f >= 7600) _ps = PWM_PS_8; \
        else if (_f >= 950) _ps = PWM_PS_64; \
        else if (_f >= 240) _ps = PWM_PS_256; \
        else _ps = PWM_PS_1024; \
        PWM_TCCR0B() = (PWM_TCCR0B() & 0xF8) | _ps; \
    } while (0)

#define _PWM_set_freq_timer2(freq) \
    do { \
        uint32_t _f = (freq); \
        uint8_t _ps; \
        if (_f >= 61000) _ps = PWM_PS_1; \
        else if (_f >= 7600) _ps = PWM_PS_8; \
        else if (_f >= 950) _ps = PWM_PS_64; \
        else if (_f >= 240) _ps = PWM_PS_256; \
        else _ps = PWM_PS_1024; \
        PWM_TCCR2B() = (PWM_TCCR2B() & 0xF8) | _ps; \
    } while (0)

#define _PWM_set_freq_timer1(freq) \
    do { \
        uint32_t _f = (freq); \
        uint32_t _top; \
        uint8_t _ps; \
        if (_f >= 244140) { _ps = PWM_PS_1; _top = (F_CPU / _f) - 1; } \
        else if (_f >= 30517) { _ps = PWM_PS_8; _top = (F_CPU / 8 / _f) - 1; } \
        else if (_f >= 3814) { _ps = PWM_PS_64; _top = (F_CPU / 64 / _f) - 1; } \
        else if (_f >= 953) { _ps = PWM_PS_256; _top = (F_CPU / 256 / _f) - 1; } \
        else { _ps = PWM_PS_1024; _top = (F_CPU / 1024 / _f) - 1; } \
        if (_top > 0xFFFF) _top = 0xFFFF; \
        ICR1 = (uint16_t)_top; \
        PWM_TCCR1B() = (PWM_TCCR1B() & 0xF8) | _ps; \
    } while (0)

/* ============================================================================
 * Prescaler Values
 * ========================================================================== */

#define PWM_PS_1      0x01
#define PWM_PS_8      0x02
#define PWM_PS_64     0x03
#define PWM_PS_256    0x04
#define PWM_PS_1024   0x05

/* ============================================================================
 * Fluent API (Optional) - PERIPHERAL -> TARGET -> ACTION grammar
 * ========================================================================== */

typedef struct { uint8_t pin; } pwm_pin_t;

#define PWM_pin(pin)  ((pwm_pin_t){0})  /* pin unused; macros expand at call site */

#define pwm_pin_enable(pin)      PWM_enable(pin)
#define pwm_pin_disable(pin)     PWM_disable(pin)
#define pwm_pin_duty(pin, d)     PWM_duty(pin, d)
#define pwm_pin_frequency(pin, f) PWM_frequency(pin, f)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_PWM_H */