/**
 * @file spi_transfer.c
 * @brief Bare AVR SPI benchmark
 */

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    /* Master, Mode 0, f/4 */
    DDRB |= (1<<PB2) | (1<<PB3) | (1<<PB5);  // SS, MOSI, SCK output
    DDRB &= ~(1<<PB4);                        // MISO input
    PORTB |= (1<<PB2);                        // SS HIGH

    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);  // Enable, Master, f/4
    SPSR = 0;

    while (1) {
        PORTB &= ~(1<<PB2);  // SS LOW
        SPDR = 0x55;
        while (!(SPSR & (1<<SPIF)));
        (void)SPDR;
        PORTB |= (1<<PB2);  // SS HIGH
        _delay_ms(100);
    }

    return 0;
}