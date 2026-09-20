/**
 * @file pwm_duty.c
 * @brief MicroAVR PWM benchmark
 */

#include <microavr/microavr.h>

int main(void) {
    PWM_enable(PD6);
    PWM_frequency(PD6, 1000);

    while (1) {
        for (uint16_t i = 0; i < 255; i++) {
            PWM_duty(PD6, i);
            delay_ms(5);
        }
        for (uint16_t i = 255; i > 0; i--) {
            PWM_duty(PD6, i);
            delay_ms(5);
        }
    }

    return 0;
}