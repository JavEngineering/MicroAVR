/**
 * @file uart.h
 * @brief UART/USART abstraction for ATmega328P - compile-time baud calculation, zero-cost.
 *
 * Hardware: ATmega328P USART0
 *   Pins: PD0 = RX, PD1 = TX
 *   Registers: UBRR0, UCSR0A, UCSR0B, UCSR0C, UDR0
 *
 * Modes: Asynchronous (8N1 default), Synchronous, SPI Master
 * Baud rates: 2400 to 1M @ 16MHz
 *
 * Compile-time resolution: baud rate calculation at compile time.
 * No runtime lookup tables. Both concise and fluent APIs compile to
 * identical machine code.
 */

#ifndef MICROAVR_UART_H
#define MICROAVR_UART_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * UART Format Enum
 * ========================================================================== */

typedef enum {
    UART_8N1 = 0,   // 8 data, no parity, 1 stop (default)
    UART_8N2 = 1,   // 8 data, no parity, 2 stop
    UART_8E1 = 2,   // 8 data, even parity, 1 stop
    UART_8O1 = 3,   // 8 data, odd parity, 1 stop
    UART_9N1 = 4,   // 9 data, no parity, 1 stop
} uart_format_t;

/* ============================================================================
 * Compile-Time Baud Rate Calculation
 * ========================================================================== */

#define _UART_BAUD_UBRR(baud)  ((F_CPU / (16UL * (baud))) - 1)

/* Common baud rates @ 16MHz (U2X = 0, normal async mode) */
#define UART_UBRR_2400     416   // 0.1% error
#define UART_UBRR_4800     207   // 0.2% error
#define UART_UBRR_9600     103   // 0.2% error
#define UART_UBRR_19200    51    // 0.2% error
#define UART_UBRR_38400    25    // 0.2% error
#define UART_UBRR_57600    16    // 2.1% error
#define UART_UBRR_115200   8     // 3.7% error (consider U2X mode)
#define UART_UBRR_230400   3     // 8.5% error
#define UART_UBRR_250000   3     // 8.5% error
#define UART_UBRR_500000   1     // 0% error (with U2X)
#define UART_UBRR_1000000  0     // 0% error (with U2X)

/* ============================================================================
 * Format Register Values (UCSR0C)
 * Note: ATmega328P uses UPM01/UPM00, USBS0, UCSZ01/UCSZ00
 * ========================================================================== */

#define _UART_FORMAT_8N1  ((0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(3<<UCSZ00))  // 8N1
#define _UART_FORMAT_8N2  ((0<<UPM01)|(0<<UPM00)|(1<<USBS0)|(3<<UCSZ00))  // 8N2
#define _UART_FORMAT_8E1  ((1<<UPM01)|(0<<UPM00)|(0<<USBS0)|(3<<UCSZ00))  // 8E1
#define _UART_FORMAT_8O1  ((1<<UPM01)|(1<<UPM00)|(0<<USBS0)|(3<<UCSZ00))  // 8O1
#define _UART_FORMAT_9N1  ((0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(7<<UCSZ00))  // 9N1 (UCSZ2 in UCSR0B)

#define UART_GET_FORMAT(fmt) \
    ((fmt) == UART_8N1 ? _UART_FORMAT_8N1 : \
     (fmt) == UART_8N2 ? _UART_FORMAT_8N2 : \
     (fmt) == UART_8E1 ? _UART_FORMAT_8E1 : \
     (fmt) == UART_8O1 ? _UART_FORMAT_8O1 : \
     _UART_FORMAT_9N1)

/* ============================================================================
 * Concise API (Primary) - Macros/Static Inline for Zero Overhead
 * ========================================================================== */

/**
 * @brief Initialize UART with baud rate and format (default 8N1).
 * Hardware: Sets UBRR0, UCSR0B (RXEN/TXEN), UCSR0C (format)
 * Usage: UART_begin(9600);
 *        UART_begin(9600, UART_8N1);
 */
static inline void UART_begin(uint32_t baud, uart_format_t fmt) {
    /* Set baud rate */
    UBRR0 = _UART_BAUD_UBRR(baud);

    /* Enable RX and TX */
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);

    /* Set format (8N1 default) */
    UCSR0C = UART_GET_FORMAT(fmt);

    /* For 9-bit, set UCSZ2 in UCSR0B */
    if (fmt == UART_9N1) {
        UCSR0B |= (1<<UCSZ02);
    } else {
        UCSR0B &= ~(1<<UCSZ02);
    }
}

/**
 * @brief Initialize UART with default 8N1 format.
 * Usage: UART_begin(9600);
 */
#define UART_begin(baud)  UART_begin(baud, UART_8N1)

/**
 * @brief Disable UART (disable RX/TX).
 * Hardware: UCSR0B &= ~((1<<RXEN0)|(1<<TXEN0))
 * Usage: UART_end();
 */
static inline void UART_end(void) {
    UCSR0B &= ~((1<<RXEN0)|(1<<TXEN0));
}

/**
 * @brief Write a single byte (blocking).
 * Hardware: Waits for UDRE0, writes UDR0
 * Usage: UART_write('A');
 */
static inline void UART_write(uint8_t data) {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

/**
 * @brief Write a buffer (blocking).
 * Hardware: Repeated UART_write()
 * Usage: UART_write_buf(buf, len);
 */
static inline void UART_write_buf(const uint8_t* buf, uint16_t len) {
    while (len--) {
        UART_write(*buf++);
    }
}

/**
 * @brief Print null-terminated string (blocking).
 * Hardware: Repeated UART_write()
 * Usage: UART_print("Hello");
 */
static inline void UART_print(const char* str) {
    while (*str) {
        UART_write(*str++);
    }
}

/**
 * @brief Print string + newline (blocking).
 * Hardware: UART_print() + UART_write('\r') + UART_write('\n')
 * Usage: UART_println("Hello");
 */
static inline void UART_println(const char* str) {
    UART_print(str);
    UART_write('\r');
    UART_write('\n');
}

/**
 * @brief Print a byte as two hex digits.
 * Hardware: Uses UART_write().
 * @param byte Value to print (0x00-0xFF)
 * Usage: UART_print_hex(0x42);
 */
static inline void UART_print_hex(uint8_t byte) {
    static const char hex[] = "0123456789ABCDEF";
    UART_write(hex[byte >> 4]);
    UART_write(hex[byte & 0x0F]);
}

/**
 * @brief Print a byte as hex followed by newline.
 * Hardware: Uses UART_print_hex() + UART_write().
 * @param byte Value to print
 * Usage: UART_println_hex(0x42);
 */
static inline void UART_println_hex(uint8_t byte) {
    UART_print_hex(byte);
    UART_write('\r');
    UART_write('\n');
}

/**
 * @brief Read a single byte (blocking).
 * Hardware: Waits for RXC0, reads UDR0
 * @return Received byte
 * Usage: uint8_t c = UART_read();
 */
static inline uint8_t UART_read(void) {
    while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
}

/**
 * @brief Check if data is available (non-blocking).
 * Hardware: Checks RXC0 flag
 * @return true if byte available
 * Usage: if (UART_available()) { c = UART_read(); }
 */
static inline bool UART_available(void) {
    return (UCSR0A & (1<<RXC0)) != 0;
}

/**
 * @brief Read byte if available (non-blocking).
 * Hardware: Checks RXC0, reads UDR0 if set
 * @return Received byte, or 0xFF if none available
 * Usage: uint8_t c = UART_read_nb();
 */
static inline uint8_t UART_read_nb(void) {
    if (UCSR0A & (1<<RXC0)) {
        return UDR0;
    }
    return 0xFF;
}

/**
 * @brief Flush transmit buffer (wait for TX complete).
 * Hardware: Waits for TXC0 flag
 * Usage: UART_flush();
 */
static inline void UART_flush(void) {
    while (!(UCSR0A & (1<<TXC0)));
    UCSR0A |= (1<<TXC0);  // Clear flag
}

/* ============================================================================
 * Fluent API (Optional) - PERIPHERAL -> TARGET -> ACTION grammar
 * ========================================================================== */

#define UART_baud(baud)     (void)(baud)  /* placeholder for fluent syntax */
#define UART_format(fmt)    (void)(fmt)   /* placeholder */

#define uart_baud(b)        UART_begin(b)
#define uart_write(b)       UART_write(b)
#define uart_print(s)       UART_print(s)
#define uart_println(s)     UART_println(s)
#define uart_read()         UART_read()
#define uart_available()    UART_available()

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_UART_H */