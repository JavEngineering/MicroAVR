/**
 * @file spi.h
 * @brief SPI abstraction for ATmega328P - compile-time config, zero-cost.
 *
 * Hardware: ATmega328P SPI
 *   Pins: PB2 = SS, PB3 = MOSI, PB4 = MISO, PB5 = SCK
 *   Registers: SPCR, SPSR, SPDR
 *
 * Modes: Master (primary), Slave
 * Clock: f_OSC/2 to f_OSC/128
 * SPI Modes: 0, 1, 2, 3 (CPOL/CPHA)
 *
 * Compile-time resolution: all config at compile time.
 * No runtime lookup tables. Both concise and fluent APIs compile to
 * identical machine code.
 *
 * Note: Chip select (SS) is NOT automatically managed.
 * User must call SPI_select() / SPI_deselect() explicitly.
 */

#ifndef MICROAVR_SPI_H
#define MICROAVR_SPI_H

#include <avr/io.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * SPI Enums
 * ========================================================================== */

typedef enum {
    SPI_MODE0 = 0,  // CPOL=0, CPHA=0
    SPI_MODE1 = 1,  // CPOL=0, CPHA=1
    SPI_MODE2 = 2,  // CPOL=1, CPHA=0
    SPI_MODE3 = 3,  // CPOL=1, CPHA=1
} spi_mode_t;

typedef enum {
    SPI_CLK_DIV2    = 0,   // f_OSC/2
    SPI_CLK_DIV4    = 1,   // f_OSC/4
    SPI_CLK_DIV8    = 2,   // f_OSC/8
    SPI_CLK_DIV16   = 3,   // f_OSC/16
    SPI_CLK_DIV32   = 4,   // f_OSC/32
    SPI_CLK_DIV64   = 5,   // f_OSC/64
    SPI_CLK_DIV128  = 6,   // f_OSC/128
} spi_clock_div_t;

typedef enum {
    SPI_MASTER = 0,
    SPI_SLAVE  = 1,
} spi_role_t;

/* ============================================================================
 * Compile-Time SPI Configuration Macros
 * ========================================================================== */

#define _SPI_MODE_BITS(mode) \
    ((mode) == SPI_MODE0 ? 0 : \
     (mode) == SPI_MODE1 ? (1<<CPHA) : \
     (mode) == SPI_MODE2 ? (1<<CPOL) : \
     (1<<CPOL)|(1<<CPHA))

#define _SPI_CLK_BITS(div) \
    ((div) == SPI_CLK_DIV2   ? 0 : \
     (div) == SPI_CLK_DIV4   ? (1<<SPR0) : \
     (div) == SPI_CLK_DIV8   ? (1<<SPR1) : \
     (div) == SPI_CLK_DIV16  ? (1<<SPR1)|(1<<SPR0) : \
     (div) == SPI_CLK_DIV32  ? (1<<SPI2X) : \
     (div) == SPI_CLK_DIV64  ? (1<<SPI2X)|(1<<SPR0) : \
     (1<<SPI2X)|(1<<SPR1))

/* ============================================================================
 * SPI Pin Definitions (Fixed hardware pins - use numeric values)
 * ========================================================================== */

#define SPI_SS_PIN    2   // PB2
#define SPI_MOSI_PIN  3   // PB3
#define SPI_MISO_PIN  4   // PB4
#define SPI_SCK_PIN   5   // PB5

/* ============================================================================
 * Concise API (Primary)
 * ========================================================================== */

/**
 * @brief Initialize SPI.
 * Hardware: Configures SPCR, SPSR, sets pin directions.
 * Usage: SPI_begin(SPI_MASTER, SPI_MODE0, SPI_CLK_DIV4);
 *        SPI_begin();  // Default: Master, Mode 0, f/4
 */
static inline void SPI_begin(spi_role_t role, spi_mode_t mode, spi_clock_div_t div) {
    /* Set pin directions for Master: SS, MOSI, SCK output; MISO input */
    if (role == SPI_MASTER) {
        DDRB |= (1<<SPI_SS_PIN) | (1<<SPI_MOSI_PIN) | (1<<SPI_SCK_PIN);  // SS, MOSI, SCK output
        DDRB &= ~(1<<SPI_MISO_PIN);                                        // MISO input
        PORTB |= (1<<SPI_SS_PIN);                                          // SS HIGH (inactive)
    } else {
        DDRB |= (1<<SPI_MISO_PIN);                                         // MISO output (slave)
        DDRB &= ~((1<<SPI_SS_PIN)|(1<<SPI_MOSI_PIN)|(1<<SPI_SCK_PIN));    // SS, MOSI, SCK input
    }

    /* Configure SPCR: SPE=1, MSTR, mode, clock */
    uint8_t spcr = (1<<SPE);
    if (role == SPI_MASTER) spcr |= (1<<MSTR);
    spcr |= _SPI_MODE_BITS(mode);
    spcr |= _SPI_CLK_BITS(div);
    SPCR = spcr;

    /* Clear SPI2X for normal speed (handled in _SPI_CLK_BITS) */
    SPSR = (SPSR & ~(1<<SPI2X)) | ((div >= SPI_CLK_DIV32) ? (1<<SPI2X) : 0);
}

/**
 * @brief Initialize SPI with defaults (Master, Mode 0, f/4).
 * Usage: SPI_begin();
 */
#define SPI_begin()  SPI_begin(SPI_MASTER, SPI_MODE0, SPI_CLK_DIV4)

/**
 * @brief Disable SPI.
 * Hardware: SPE=0
 * Usage: SPI_end();
 */
static inline void SPI_end(void) {
    SPCR &= ~(1<<SPE);
}

/**
 * @brief Select device (pull SS LOW).
 * Hardware: PORTB &= ~(1<<PB2)
 * Usage: SPI_select();
 */
static inline void SPI_select(void) {
    PORTB &= ~(1<<SPI_SS_PIN);  // SS LOW
}

/**
 * @brief Deselect device (pull SS HIGH).
 * Hardware: PORTB |= (1<<PB2)
 * Usage: SPI_deselect();
 */
static inline void SPI_deselect(void) {
    PORTB |= (1<<SPI_SS_PIN);  // SS HIGH
}

/**
 * @brief Full-duplex transfer: write byte, return received byte.
 * Hardware: SPDR = data; wait for SPIF; return SPDR
 * @return Received byte
 * Usage: uint8_t rx = SPI_transfer(0x55);
 */
static inline uint8_t SPI_transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1<<SPIF)));
    return SPDR;
}

/**
 * @brief Write byte (ignore received).
 * Hardware: SPDR = data; wait for SPIF
 * Usage: SPI_write(0x55);
 */
static inline void SPI_write(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1<<SPIF)));
    (void)SPDR;  // Clear SPIF by reading
}

/**
 * @brief Read byte (send 0xFF).
 * Hardware: SPDR = 0xFF; wait for SPIF; return SPDR
 * @return Received byte
 * Usage: uint8_t rx = SPI_read();
 */
static inline uint8_t SPI_read(void) {
    SPDR = 0xFF;
    while (!(SPSR & (1<<SPIF)));
    return SPDR;
}

/* ============================================================================
 * Fluent API (Optional) - PERIPHERAL -> TARGET -> ACTION grammar
 * ========================================================================== */

#define SPI_mode(m)     (void)(m)  /* placeholder */
#define SPI_speed(d)    (void)(d)  /* placeholder */

#define spi_mode(m)     SPI_mode(m)
#define spi_speed(d)    SPI_speed(d)
#define spi_transfer(d) SPI_transfer(d)
#define spi_write(d)    SPI_write(d)
#define spi_read()      SPI_read()
#define spi_select()    SPI_select()
#define spi_deselect()  SPI_deselect()

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROAVR_SPI_H */