/**
 * @file main.c
 * @brief Blink example - demonstrates GPIO and delay APIs.
 *
 * Hardware: ATmega328P with LED on PB5 (Arduino pin 13)
 *
 * Expected behavior:
 *   - LED toggles every 500ms
 *
 * Assembly verification:
 *   GPIO_output(PB5)  -> SBI DDRB, 5
 *   GPIO_toggle(PB5)  -> SBI PORTB, 5  (or PORTB ^= (1<<5))
 *   delay_ms(500)     -> busy-wait loop
 */

#include <microavr/microavr.h>

int main(void) {
    /* Configure PB5 as output (Arduino LED pin) */
    GPIO_output(PB5);

    while (1) {
        /* Toggle LED */
        GPIO_toggle(PB5);

        /* Wait 500ms */
        delay_ms(500);
    }

    return 0;
}