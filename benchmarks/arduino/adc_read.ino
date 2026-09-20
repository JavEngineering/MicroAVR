/**
 * @file adc_read.c
 * @brief Arduino-style ADC benchmark - analogRead()
 */

#include <Arduino.h>

void setup() {
    analogReference(DEFAULT);  // AVCC
}

void loop() {
    int value = analogRead(A0);
    delay(100);
}