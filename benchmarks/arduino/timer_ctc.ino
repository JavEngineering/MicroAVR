/**
 * @file timer_ctc.ino
 * @brief Arduino-style timer benchmark - Timer1 CTC
 */

#include <Arduino.h>

volatile uint32_t ms_ticks = 0;

ISR(TIMER1_COMPA_vect) {
    ms_ticks++;
}

void setup() {
    pinMode(13, OUTPUT);
    
    // Timer1 CTC mode
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);  // CTC, prescaler 64
    OCR1A = 250;  // 16MHz/64/250 = 1kHz
    TIMSK1 = (1 << OCIE1A);
}

void loop() {
    if (ms_ticks >= 1000) {
        ms_ticks = 0;
        digitalWrite(13, !digitalRead(13));
    }
}