/**
 * @file main.c
 * @brief I2C example - Read from I2C device (e.g., MPU6050, BMP280, etc.)
 *
 * Hardware: ATmega328P as I2C Master
 *   SDA -> PC4 (Arduino A4)
 *   SCL -> PC5 (Arduino A5)
 *
 * Expected behavior:
 *   - Initializes I2C at 100kHz
 *   - Reads from device at address 0x68 (e.g., MPU6050)
 *   - Reads register 0x3B (accel X high byte)
 *
 * Assembly verification:
 *   I2C_begin() -> TWBR=72, TWEN=1
 *   I2C_device_read(0x68, 0x3B, buf, 2) -> START, SLA+W, reg, REP_START, SLA+R, read, STOP
 */

#include <microavr/microavr.h>

int main(void) {
    /* Initialize I2C at 100kHz */
    I2C_begin();

    /* Example: Read 2 bytes from register 0x3B of device at 0x68 (MPU6050) */
    uint8_t data[2];

    while (1) {
        if (I2C_device_read(0x68, 0x3B, data, 2)) {
            /* Successfully read 2 bytes from register 0x3B */
            /* data[0] = high byte, data[1] = low byte */
            GPIO_toggle(PB5);  // Toggle LED on successful read
        } else {
            /* Read failed - device not responding */
        }

        delay_ms(100);
    }

    return 0;
}