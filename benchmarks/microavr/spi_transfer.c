/**
 * @file spi_transfer.c
 * @brief MicroAVR SPI benchmark
 */

#include <microavr/microavr.h>

int main(void) {
    SPI_begin(SPI_MASTER, SPI_MODE0, SPI_CLK_DIV4);

    while (1) {
        SPI_select();
        SPI_transfer(0x55);
        SPI_deselect();
        delay_ms(100);
    }

    return 0;
}