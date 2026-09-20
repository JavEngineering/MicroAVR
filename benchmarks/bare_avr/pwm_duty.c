/**
 * @file pwm_duty.c
 * @brief Bare AVR PWM benchmark
 */

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    /* PD6 as output */
    DDRD |= (1 << DDD6);

    /* Timer0 Fast PWM, non-inverting OC0A */
    TCCR0A = (1 << WGM01) | (1 << WGM00) | (1 << COM0A1);
    TCCR0B = (1 << CS01) | (1 << CS00);  // Prescaler 64

    while (1) {
        for (uint8_t i = 0; i < 255; i++) {
            OCR0A = i;
            _delay_ms(5);
        }
        for (uint8_t i = 255; i > 0; i--) {
            OCR0A = i;
            _delay_ms(5);
        }
    }

    return 0;
}