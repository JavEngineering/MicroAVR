/**
 * @file i2c_read.c
 * @brief Bare AVR I2C benchmark
 */

#include <avr/io.h>
#include <util/delay.h>

static inline void i2c_begin(void) {
    TWSR = 0;
    TWBR = 72;  // 100kHz @ 16MHz
    TWCR = (1<<TWEN);
}

static inline void i2c_start(void) {
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}

static inline void i2c_stop(void) {
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

static inline void i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}

static inline uint8_t i2c_read_ack(void) {
    TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

static inline uint8_t i2c_read_nack(void) {
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

static inline uint8_t i2c_status(void) {
    return (TWSR & 0xF8);
}

static inline bool i2c_device_read(uint8_t addr, uint8_t reg, uint8_t* data, uint8_t len) {
    i2c_start();
    if ((i2c_status() != 0x08) && (i2c_status() != 0x10)) return false;

    i2c_write((addr << 1) | 0);
    if (i2c_status() != 0x18) { i2c_stop(); return false; }

    i2c_write(reg);
    if (i2c_status() != 0x28) { i2c_stop(); return false; }

    i2c_start();
    if (i2c_status() != 0x10) { i2c_stop(); return false; }

    i2c_write((addr << 1) | 1);
    if (i2c_status() != 0x40) { i2c_stop(); return false; }

    while (len--) {
        if (len == 0) *data++ = i2c_read_nack();
        else *data++ = i2c_read_ack();
    }

    i2c_stop();
    return true;
}

int main(void) {
    i2c_begin();
    uint8_t data[2];

    while (1) {
        i2c_device_read(0x68, 0x3B, data, 2);
        _delay_ms(100);
    }

    return 0;
}