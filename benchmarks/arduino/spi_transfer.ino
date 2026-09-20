/**
 * @file spi_transfer.ino
 * @brief Arduino-style SPI benchmark
 */

#include <SPI.h>

void setup() {
    SPI.begin();  // Default: Master, Mode 0, 4MHz
}

void loop() {
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(SS, LOW);
    SPI.transfer(0x55);
    digitalWrite(SS, HIGH);
    SPI.endTransaction();
    delay(100);
}