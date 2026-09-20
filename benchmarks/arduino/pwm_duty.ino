/**
 * @file pwm_duty.ino
 * @brief Arduino-style PWM benchmark - analogWrite()
 */

void setup() {
    pinMode(6, OUTPUT);  // PD6 / OC0A
}

void loop() {
    for (int i = 0; i < 255; i++) {
        analogWrite(6, i);
        delay(5);
    }
    for (int i = 255; i > 0; i--) {
        analogWrite(6, i);
        delay(5);
    }
}