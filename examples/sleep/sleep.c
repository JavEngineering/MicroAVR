/**
 * @file main.c
 * @brief Sleep example - Deep sleep with INT0 button wake and WDT periodic wake
 *
 * Hardware: ATmega328P with LED on PB5, button on PD2 (INT0)
 *   Button: PD2 -> GND (with pull-up enabled)
 *   LED: PB5 (Arduino D13)
 *
 * Expected behavior:
 *   - Enters deep sleep (Power-down mode)
 *   - Wakes on button press (INT0 falling edge)
 *   - Wakes periodically via WDT (~4 seconds)
 *   - LED blinks on each wake event
 */

#include <microavr/microavr.h>

volatile bool wdt_wake = false;
volatile bool button_wake = false;

/* WDT interrupt - periodic wake */
void wdt_callback(void) {
    wdt_wake = true;
}

/* Auto-generate WDT ISR */
ISR(WDT_vect) {
    wdt_callback();
}

/* INT0 (PD2) external interrupt - button press */
void int0_callback(void) {
    button_wake = true;
}

/* Auto-generate INT0 ISR */
INTERRUPT_ISR(INT0_vect, int0_callback);

int main(void) {
    /* Configure LED */
    GPIO_output(PB5);

    /* Configure button on PD2 (INT0) with pull-up */
    GPIO_input(PD2);
    GPIO_pullup(PD2);

    /* Configure INT0 for falling edge (button press) */
    Interrupt_configure(0, INT_FALLING);
    Interrupt_on(0, INT_FALLING, int0_callback);

    /* Configure WDT for ~4s timeout with interrupt */
    WDTCSR = (1<<WDCE) | (1<<WDE);  // Enable change
    WDTCSR = (1<<WDE) | (1<<WDIE) | (1<<WDP3) | (1<<WDP0);  // ~4s, interrupt mode

    /* Configure sleep: Power-down mode */
    Sleep_mode(SLEEP_PWR_DOWN);
    Sleep_enable();

    /* Enable global interrupts */
    Interrupt_enable_global();

    while (1) {
        /* Enter deep sleep - wakes on button (INT0) or WDT (~4s) */
        Sleep_cpu();

        if (wdt_wake) {
            wdt_wake = false;
            GPIO_toggle(PB5);  // Blink on WDT wake
        }

        if (button_wake) {
            button_wake = false;
            GPIO_toggle(PB5);  // Blink on button wake
        }
    }

    return 0;
}