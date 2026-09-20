/**
 * @file timer_ctc.c
 * @brief MicroAVR Timer1 CTC benchmark
 */

#include <microavr/microavr.h>

volatile uint32_t ms_ticks = 0;

void timer1_callback(void) {
    ms_ticks++;
}

TIMER_ISR(Timer1, TIMER1_COMPA_vect, timer1_callback);

int main(void) {
    GPIO_output(PB5);

    Timer1_mode(TIMER_MODE_CTC);
    Timer1_prescaler(TIMER_PS_64);
    Timer1_compareA(250);
    Timer1_compareAInterrupt();
    Timer1_start();

    sei();

    while (1) {
        if (ms_ticks >= 1000) {
            ms_ticks = 0;
            GPIO_toggle(PB5);
        }
    }

    return 0;
}