/**
 * @file i2c_read.c
 * @brief MicroAVR I2C benchmark
 */

#include <microavr/microavr.h>

int main(void) {
    I2C_begin();

    uint8_t data[2];

    while (1) {
        I2C_device_read(0x68, 0x3B, data, 2);
        delay_ms(100);
    }

    return 0;
}