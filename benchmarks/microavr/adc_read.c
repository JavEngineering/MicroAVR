/**
 * @file adc_read.c
 * @brief MicroAVR ADC benchmark
 */

#include <microavr/microavr.h>

int main(void) {
    ADC_begin();
    ADC_reference(ADC_REF_AVCC);
    ADC_prescaler(ADC_PS_128);

    while (1) {
        uint16_t value = ADC_read(ADC0);
        delay_ms(100);
    }

    return 0;
}