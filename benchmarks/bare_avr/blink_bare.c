/**
 * @file blink_bare.c
 * @brief Bare AVR C blink benchmark.
 *
 * Measures: flash, SRAM, cycles for direct register manipulation.
 * No framework (platformio.ini env:bench_bare).
 */

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    /* DDRB |= (1 << DDB5); */
    DDRB |= (1 << 5);

    while (1) {
        /* PORTB |= (1 << PB5); */
        PORTB |= (1 << 5);
        _delay_ms(500);

        /* PORTB &= ~(1 << PB5); */
        PORTB &= ~(1 << 5);
        _delay_ms(500);
    }

    return 0;
}