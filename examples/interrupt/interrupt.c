/**
 * @file main.c
 * @brief Interrupt example - Button (INT0) + Timer1 CTC interrupt
 *
 * Hardware: ATmega328P with LED on PB5, button on PD2 (INT0)
 *   Button: PD2 -> GND (with pull-up enabled)
 *   LED: PB5 (Arduino D13)
 *
 * Expected behavior:
 *   - LED blinks at 1Hz via Timer1 CTC interrupt
 *   - Button press on PD2 toggles LED immediately via INT0 interrupt
 *
 * Assembly verification:
 *   INT0: EICRA=FALLING, EIMSK=INT0
 *   Timer1: CTC, OCR1A=250, prescaler=64, OCIE1A enabled
 */

#include <microavr/microavr.h>

volatile uint32_t ms_ticks = 0;
volatile bool button_pressed = false;

/* Timer1 compare match A callback - 1ms tick */
void timer1_callback(void) {
    ms_ticks++;
}

/* Auto-generate Timer1 COMPA ISR */
TIMER_ISR(Timer1, TIMER1_COMPA_vect, timer1_callback);

/* INT0 (PD2) external interrupt callback - button press */
void int0_callback(void) {
    button_pressed = true;
}

/* Auto-generate INT0 ISR */
INTERRUPT_ISR(INT0_vect, int0_callback);

int main(void) {
    /* Configure LED on PB5 */
    GPIO_output(PB5);

    /* Configure button on PD2 (INT0) with pull-up */
    GPIO_input(PD2);
    GPIO_pullup(PD2);

    /* Configure INT0 for falling edge (button press) */
    Interrupt_configure(0, INT_FALLING);
    Interrupt_enable(0);
    Interrupt_on(0, INT_FALLING, int0_callback);

    /* Configure Timer1 for 1ms tick (CTC, 16MHz/64/250 = 1kHz) */
    Timer1_mode(TIMER_MODE_CTC);
    Timer1_prescaler(TIMER_PS_64);
    Timer1_compareA(250);
    Timer1_compareAInterrupt(timer1_callback);  // Enable interrupt
    Timer1_start();

    /* Enable global interrupts */
    Interrupt_enable_global();

    while (1) {
        /* Blink LED every 1 second via timer interrupt */
        if (ms_ticks >= 1000) {
            ms_ticks = 0;
            GPIO_toggle(PB5);
        }

        /* Toggle LED immediately on button press */
        if (button_pressed) {
            button_pressed = false;
            GPIO_toggle(PB5);
        }
    }

    return 0;
}