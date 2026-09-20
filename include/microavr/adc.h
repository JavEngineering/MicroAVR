/**
 * @file adc.h
 * @brief ADC abstraction for ATmega328P - compile-time channel mapping, zero-cost.
 *
 * Hardware: ATmega328P 10-bit ADC (6 channels + temp sensor + bandgap)
 *   ADC0-ADC5: PC0-PC5 (Arduino A0-A5)
 *   ADC6: Temperature sensor (internal)
 *   ADC7: 1.1V bandgap reference (internal)
 *
 * Registers:
 *   ADMUX  - Reference selection, channel selection, left-adjust
 *   ADCSRA - Enable, start conversion, prescaler, interrupt
 *   ADCL/ADCH - 10-bit result (read ADCL first!)
 *
 * Compile-time resolution: all channel-to-MUX mapping via macros.
 * No runtime lookup tables. Both concise and fluent APIs compile to
 * identical machine code.
 */

#ifndef MICROAVR_ADC_H
#define MICROAVR_ADC_H

#include <avr/io.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Compile-Time Channel -> MUX Bit Mapping (Token Pasting)
 * ========================================================================== */

/* --- ADC Channels (MUX bits) --- */
#define _ADC_CH_ADC0_MUX   0x00  // PC0
#define _ADC_CH_ADC1_MUX   0x01  // PC1
#define _ADC_CH_ADC2_MUX   0x02  // PC2
#define _ADC_CH_ADC3_MUX   0x03  // PC3
#define _ADC_CH_ADC4_MUX   0x04  // PC4
#define _ADC_CH_ADC5_MUX   0x05  // PC5
#define _ADC_CH_ADC6_MUX   0x06  // Temperature sensor
#define _ADC_CH_ADC7_MUX   0x07  // 1.1V bandgap

/* ============================================================================
 * User-Facing Channel Identifiers
 * Use these in API calls: ADC_read(ADC0);
 * ========================================================================== */

/* Undefine any conflicting macros */
#undef ADC0
#undef ADC1
#undef ADC2
#undef ADC3
#undef ADC4
#undef ADC5
#undef ADC6
#undef ADC7

/* Define channel identifiers as literal tokens */
#define ADC0  ADC0
#define ADC1  ADC1
#define ADC2  ADC2
#define ADC3  ADC3
#define ADC4  ADC4
#define ADC5  ADC5
#define ADC6  ADC6
#define ADC7  ADC7

/* ============================================================================
 * Reference Voltage Selection (REFS1:0 in ADMUX)
 * ========================================================================== */

#define ADC_REF_AVCC           0x40  // REFS0=1, AVCC with external capacitor at AREF
#define ADC_REF_INTERNAL_1V1   0xC0  // REFS1=1, REFS0=1, Internal 1.1V reference
#define ADC_REF_INTERNAL_2V56  0x80  // REFS1=1, Internal 2.56V reference (not on all AVRs)

/* ============================================================================
 * Prescaler Selection (ADPS2:0 in ADCSRA)
 * ADC clock = F_CPU / prescaler (target 50-200 kHz for max resolution)
 * ========================================================================== */

#define ADC_PS_2    0x00  // ADPS2=0, ADPS1=0, ADPS0=0
#define ADC_PS_4    0x02  // ADPS2=0, ADPS1=1, ADPS0=0
#define ADC_PS_8    0x03  // ADPS2=0, ADPS1=1, ADPS0=1
#define ADC_PS_16   0x04  // ADPS2=1, ADPS1=0, ADPS0=0
#define ADC_PS_32   0x05  // ADPS2=1, ADPS1=0, ADPS0=1
#define ADC_PS_64   0x06  // ADPS2=1, ADPS1=1, ADPS0=0
#define ADC_PS_128  0x07  // ADPS2=1, ADPS1=1, ADPS0=1

/* ============================================================================
 * Channel Accessor Macros (Compile-Time Resolution)
 * ========================================================================== */

#define ADC_CHANNEL_MUX(ch)  _ADC_CH_##ch##_MUX

/* ============================================================================
 * Concise API (Primary) - Macros for Zero Overhead
 * ========================================================================== */

/**
 * @brief Initialize ADC with default settings.
 * Hardware: ADCSRA |= (1 << ADEN);
 * Must call before any ADC operations.
 * Usage: ADC_begin();
 */
static inline void ADC_begin(void) {
    ADCSRA |= (1 << ADEN);
}

/**
 * @brief Disable ADC to save power.
 * Hardware: ADCSRA &= ~(1 << ADEN);
 * Usage: ADC_end();
 */
static inline void ADC_end(void) {
    ADCSRA &= ~(1 << ADEN);
}

/**
 * @brief Set voltage reference.
 * Hardware: ADMUX = (ADMUX & 0x3F) | ref;
 * Usage: ADC_reference(ADC_REF_AVCC);
 * @param ref One of ADC_REF_AVCC, ADC_REF_INTERNAL_1V1, ADC_REF_INTERNAL_2V56
 */
static inline void ADC_reference(uint8_t ref) {
    ADMUX = (ADMUX & 0x3F) | ref;
}

/**
 * @brief Set ADC clock prescaler.
 * Hardware: ADCSRA = (ADCSRA & 0xF8) | ps;
 * Usage: ADC_prescaler(ADC_PS_128);
 * @param ps One of ADC_PS_2, ADC_PS_4, ..., ADC_PS_128
 */
static inline void ADC_prescaler(uint8_t ps) {
    ADCSRA = (ADCSRA & 0xF8) | ps;
}

/**
 * @brief Perform single conversion on channel and return result.
 * Hardware: 
 *   ADMUX = (ADMUX & 0xF0) | channel_mux;
 *   ADCSRA |= (1 << ADSC);
 *   while (ADCSRA & (1 << ADSC));
 *   return (ADCH << 8) | ADCL;
 * Usage: uint16_t value = ADC_read(ADC0);
 * @return 10-bit ADC value (0-1023)
 */
#define ADC_read(ch) \
    ({ \
        ADMUX = (ADMUX & 0xF0) | ADC_CHANNEL_MUX(ch); \
        ADCSRA |= (1 << ADSC); \
        while (ADCSRA & (1 << ADSC)); \
        (ADCH << 8) | ADCL; \
    })

/* ============================================================================
 * Fluent API (Optional) - PERIPHERAL -> TARGET -> ACTION grammar
 * ========================================================================== */

/** Channel proxy struct (zero-size, optimized away) */
typedef struct { uint8_t ch; } adc_channel_t;

/** ADC.channel(ADC0) -> returns proxy for fluent chaining */
#define ADC_channel(ch)  ((adc_channel_t){0})  /* ch unused; macros expand at call site */

/* Fluent operations - macros that expand at call site with literal channel */
#define adc_channel_read(ch)  ADC_read(ch)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_ADC_H */