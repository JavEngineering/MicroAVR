/**
 * @file main.c
 * @brief UART example - Echo + print demo
 *
 * Hardware: ATmega328P with USB-UART on PD0/PD1 (RX/TX)
 *
 * Expected behavior:
 *   - Prints "MicroAVR UART Ready" on startup
 *   - Echoes received characters
 *   - Adds newline on carriage return
 *
 * Assembly verification:
 *   UART_begin(9600) -> UBRR0=103, UCSR0B=RXEN|TXEN, UCSR0C=8N1
 *   UART_write() -> waits for UDRE0, writes UDR0
 *   UART_read() -> waits for RXC0, reads UDR0
 */

#include <microavr/microavr.h>

int main(void) {
    /* Initialize UART at 9600 baud, 8N1 */
    UART_begin(9600);

    /* Print welcome message */
    UART_println("MicroAVR UART Ready");

    while (1) {
        /* Read character (blocking) */
        uint8_t c = UART_read();

        /* Echo it back */
        UART_write(c);

        /* Add newline on carriage return */
        if (c == '\r') {
            UART_write('\n');
        }
    }

    return 0;
}