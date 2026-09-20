/**
 * @file interrupt_latency.c
 * @brief MicroAVR interrupt latency benchmark
 */

#include <microavr/microavr.h>

volatile uint32_t isr_count = 0;

void timer1_callback(void) {
    isr_count++;
}

TIMER_ISR(Timer1, TIMER1_COMPA_vect, timer1_callback);

int main(void) {
    GPIO_output(PB5);

    Timer1_mode(TIMER_MODE_CTC);
    Timer1_prescaler(TIMER_PS_64);
    Timer1_compareA(250);
    Timer1_compareAInterrupt();
    Timer1_start();

    Interrupt_enable_global();

    while (1) {
        if (isr_count >= 1000) {
            isr_count = 0;
            GPIO_toggle(PB5);
        }
    }

    return 0;
}