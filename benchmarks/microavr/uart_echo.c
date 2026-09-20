/**
 * @file uart_echo.c
 * @brief MicroAVR UART benchmark
 */

#include <microavr/microavr.h>

int main(void) {
    UART_begin(9600);
    UART_println("MicroAVR UART Ready");

    while (1) {
        uint8_t c = UART_read();
        UART_write(c);
        if (c == '\r') UART_write('\n');
    }

    return 0;
}