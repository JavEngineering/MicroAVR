/**
 * @file adc_read.c
 * @brief Bare AVR ADC benchmark
 */

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    /* ADEN=1, ADPS=128 (111) */
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    /* REFS0=1 (AVCC ref) */
    ADMUX = (1 << REFS0);

    while (1) {
        /* Select ADC0 */
        ADMUX = (ADMUX & 0xF0) | 0x00;
        /* Start conversion */
        ADCSRA |= (1 << ADSC);
        /* Wait for completion */
        while (ADCSRA & (1 << ADSC));
        /* Read result (ADCL first!) */
        uint16_t value = ADCL | (ADCH << 8);

        _delay_ms(100);
    }

    return 0;
}