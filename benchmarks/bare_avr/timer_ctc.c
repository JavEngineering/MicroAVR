/**
 * @file timer_ctc.c
 * @brief Bare AVR Timer1 CTC benchmark
 */

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t ms_ticks = 0;

ISR(TIMER1_COMPA_vect) {
    ms_ticks++;
}

int main(void) {
    DDRB |= (1 << 5);  // PB5 output

    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);  // CTC, prescaler 64
    OCR1A = 250;
    TIMSK1 = (1 << OCIE1A);

    sei();

    while (1) {
        if (ms_ticks >= 1000) {
            ms_ticks = 0;
            PORTB ^= (1 << 5);
        }
    }

    return 0;
}