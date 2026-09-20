/**
 * @file main.c
 * @brief Timer example - 1ms system tick using Timer1 CTC + ISR
 *
 * Hardware: ATmega328P with LED on PB5 (Arduino pin 13)
 *
 * Expected behavior:
 *   - LED toggles every 1000ms (1 second)
 *   - Uses Timer1 CTC mode at 1kHz with ISR
 *
 * Assembly verification:
 *   Timer1_mode(CTC) -> TCCR1B WGM12=1
 *   Timer1_prescaler(64) -> TCCR1B CS11=1, CS10=1
 *   Timer1_compareA(250) -> OCR1A = 250
 *   Timer1_compareAInterrupt() -> TIMSK1 OCIE1A=1
 *   Timer1_start() -> TCCR1B CS11=1, CS10=1
 */

#include <microavr/microavr.h>

volatile uint32_t ms_ticks = 0;

/* Timer1 compare match A callback - called from ISR */
void timer1_callback(void) {
    ms_ticks++;
}

/* Auto-generate ISR with our callback */
TIMER_ISR(Timer1, TIMER1_COMPA_vect, timer1_callback);

int main(void) {
    /* Configure LED */
    GPIO_output(PB5);

    /* Configure Timer1 for 1ms tick (1kHz) */
    Timer1_mode(TIMER_MODE_CTC);          // CTC mode
    Timer1_prescaler(TIMER_PS_64);        // Prescaler 64
    Timer1_compareA(250);                 // 16MHz/64/250 = 1000Hz = 1ms
    Timer1_compareAInterrupt(timer1_callback);  // Enable interrupt
    Timer1_start();                       // Start timer

    /* Enable global interrupts */
    sei();

    while (1) {
        if (ms_ticks >= 1000) {
            ms_ticks = 0;
            GPIO_toggle(PB5);             // Blink LED every 1 second
        }
    }

    return 0;
}