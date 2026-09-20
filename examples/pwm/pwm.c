/**
 * @file main.c
 * @brief PWM example - LED fade on PD6 (OC0A / Timer0 / Arduino D6)
 *
 * Hardware: ATmega328P with LED on PD6 (Arduino D6 / OC0A)
 *
 * Expected behavior:
 *   - LED fades in and out smoothly
 *   - Uses Timer0 Fast PWM at ~1kHz
 *
 * Assembly verification:
 *   PWM_enable(PD6)     -> DDRD |= (1<<DDD6); TCCR0A/B config
 *   PWM_duty(PD6, val)  -> OCR0A = val
 *   PWM_frequency(PD6, 1000) -> TCCR0B prescaler set
 */

#include <microavr/microavr.h>

int main(void) {
    /* Enable PWM on PD6 (OC0A / Timer0) at ~1kHz */
    PWM_enable(PD6);
    PWM_frequency(PD6, 1000);

    while (1) {
        /* Fade in */
        for (uint16_t i = 0; i < 255; i++) {
            PWM_duty(PD6, i);
            delay_ms(5);
        }

        /* Fade out */
        for (uint16_t i = 255; i > 0; i--) {
            PWM_duty(PD6, i);
            delay_ms(5);
        }
    }

    return 0;
}