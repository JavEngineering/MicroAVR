/**
 * @file blink_arduino.c
 * @brief Arduino-style blink benchmark.
 *
 * Measures: flash, SRAM, cycles for digitalWrite() equivalent.
 * Requires Arduino framework (platformio.ini env:bench_arduino).
 */

#include <Arduino.h>

void setup() {
    pinMode(13, OUTPUT);
}

void loop() {
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
}