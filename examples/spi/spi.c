/**
 * @file main.c
 * @brief SPI example - Communicate with SPI device
 *
 * Hardware: ATmega328P as SPI Master communicating with SPI device
 *   SS  -> PB2 (Arduino D10)
 *   MOSI -> PB3 (Arduino D11)
 *   MISO -> PB4 (Arduino D12)
 *   SCK  -> PB5 (Arduino D13)
 *
 * Expected behavior:
 *   - SPI Master, Mode 0, 4MHz @ 16MHz
 *   - Sends 0x55, reads response
 *   - Toggles SS per transaction (explicit CS control)
 *
 * Assembly verification:
 *   SPI_begin() -> SPCR = SPE|MSTR|SPR0 (Mode 0, f/4)
 *   SPI_select() -> PORTB &= ~(1<<PB2)
 *   SPI_transfer() -> SPDR=0x55, wait SPIF, return SPDR
 *   SPI_deselect() -> PORTB |= (1<<PB2)
 */

#include <microavr/microavr.h>

int main(void) {
    /* Initialize SPI: Master, Mode 0, f/4 (4MHz @ 16MHz) */
    SPI_begin();  // Default: Master, Mode 0, f/4

    while (1) {
        /* Transaction: CS LOW -> transfer -> CS HIGH */
        SPI_select();
        uint8_t response = SPI_transfer(0x55);  // Send 0x55, get response
        SPI_deselect();

        /* Example: use response */
        if (response == 0xAA) {
            GPIO_toggle(PB5);  // Toggle LED on PB5 if response matches
        }

        delay_ms(100);
    }

    return 0;
}