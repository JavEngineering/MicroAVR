/**
 * @file main.c
 * @brief ADC example - reads potentiometer on ADC0 (PC0 / Arduino A0)
 *
 * Hardware: ATmega328P with potentiometer on PC0 (Arduino A0)
 *   - Pot outer pins: VCC and GND
 *   - Pot wiper: PC0 (A0)
 *
 * Expected behavior:
 *   - Reads ADC value (0-1023) from potentiometer
 *   - Could drive LED brightness, etc.
 *
 * Assembly verification:
 *   ADC_begin()     -> ADCSRA |= (1<<ADEN)
 *   ADC_read(ADC0)  -> ADMUX set, ADSC start, wait, read ADCL/ADCH
 */

#include <microavr/microavr.h>

int main(void) {
    /* Initialize ADC */
    ADC_begin();
    ADC_reference(ADC_REF_AVCC);   // AVCC reference
    ADC_prescaler(ADC_PS_128);     // 16MHz/128 = 125kHz ADC clock

    while (1) {
        /* Read potentiometer value (0-1023) */
        uint16_t value = ADC_read(ADC0);

        /* Example use: could drive PWM, UART output, etc. */
        delay_ms(100);
    }

    return 0;
}