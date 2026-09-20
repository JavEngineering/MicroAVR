/**
 * @file i2c_read.ino
 * @brief Arduino-style I2C benchmark - Wire library
 */

#include <Wire.h>

void setup() {
    Wire.begin();
    Wire.setClock(100000);
}

void loop() {
    uint8_t data[2];
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);  // Repeated start
    Wire.requestFrom(0x68, 2);
    if (Wire.available() >= 2) {
        data[0] = Wire.read();
        data[1] = Wire.read();
    }
    delay(100);
}