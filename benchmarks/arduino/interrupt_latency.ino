/**
 * @file interrupt_latency.ino
 * @brief Arduino-style interrupt latency benchmark
 */

#include <Arduino.h>

volatile uint32_t isr_count = 0;

ISR(TIMER1_COMPA_vect) {
    isr_count++;
}

void setup() {
    pinMode(13, OUTPUT);
    
    // Timer1 CTC mode, 1kHz
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);  // CTC, prescaler 64
    OCR1A = 250;
    TIMSK1 = (1 << OCIE1A);
}

void loop() {
    if (isr_count >= 1000) {
        isr_count = 0;
        digitalWrite(13, !digitalRead(13));
    }
}