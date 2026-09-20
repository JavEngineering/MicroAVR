/**
 * @file main.c
 * @brief WDT example - Watchdog interrupt + reset demonstration
 *
 * Hardware: ATmega328P with LED on PB5
 *
 * Expected behavior:
 *   - WDT enabled in interrupt mode (~4s timeout)
 *   - LED toggles every WDT interrupt (~4s)
 *   - Demonstrates WDT reset flag detection on startup
 *
 * Assembly verification:
 *   WDT_enable(WDT_4S, WDT_INTERRUPT) -> Timed WDTCSR sequence
 *   WDT_reset() -> WDR instruction
 */

#include <microavr/microavr.h>

volatile bool wdt_triggered = false;

/* WDT interrupt callback */
void wdt_callback(void) {
    wdt_triggered = true;
}

/* Auto-generate WDT ISR */
WDT_ISR(wdt_callback);

int main(void) {
    /* Check if last reset was caused by WDT */
    if (WDT_was_reset()) {
        GPIO_output(PB5);
        /* Fast blink 3 times to indicate WDT reset */
        for (uint8_t i = 0; i < 3; i++) {
            GPIO_high(PB5);
            delay_ms(100);
            GPIO_low(PB5);
            delay_ms(100);
        }
        WDT_clear_reset_flag();
    }

    /* Configure LED */
    GPIO_output(PB5);
    GPIO_low(PB5);

    /* Enable WDT: ~4s timeout, interrupt mode */
    WDT_enable(WDT_4S, WDT_INTERRUPT);

    /* Enable global interrupts */
    Interrupt_enable_global();

    while (1) {
        if (wdt_triggered) {
            wdt_triggered = false;
            GPIO_toggle(PB5);  // Toggle LED every ~4s
        }

        /* "Kick the dog" - reset WDT counter */
        WDT_reset();

        /* Do other work here */
        delay_ms(100);
    }

    return 0;
}