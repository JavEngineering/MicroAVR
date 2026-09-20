/**
 * @file blink_microavr.c
 * @brief MicroAVR blink benchmark.
 *
 * Measures: flash, SRAM, cycles for GPIO_high/GPIO_toggle.
 * Uses MicroAVR library (platformio.ini env:bench_microavr).
 */

#include <microavr/microavr.h>

int main(void) {
    GPIO_output(PB5);

    while (1) {
        GPIO_high(PB5);
        delay_ms(500);
        GPIO_low(PB5);
        delay_ms(500);
    }

    return 0;
}