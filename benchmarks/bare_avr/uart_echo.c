/**
 * @file uart_echo.c
 * @brief Bare AVR UART benchmark
 */

#include <avr/io.h>
#include <util/delay.h>

static inline void uart_begin(uint32_t baud) {
    UBRR0 = (F_CPU / (16UL * baud)) - 1;
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
    UCSR0C = (3<<UCSZ0);  // 8N1
}

static inline void uart_write(uint8_t data) {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

static inline uint8_t uart_read(void) {
    while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
}

static inline void uart_print(const char* str) {
    while (*str) uart_write(*str++);
}

static inline void uart_println(const char* str) {
    uart_print(str);
    uart_write('\r');
    uart_write('\n');
}

int main(void) {
    uart_begin(9600);
    uart_println("Bare AVR UART Ready");

    while (1) {
        uint8_t c = uart_read();
        uart_write(c);
        if (c == '\r') uart_write('\n');
    }

    return 0;
}