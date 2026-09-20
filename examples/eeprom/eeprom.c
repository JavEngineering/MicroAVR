/**
 * @file main.c
 * @brief EEPROM example - read/write demonstration
 *
 * Hardware: ATmega328P with LED on PB5
 *
 * Expected behavior:
 *   - Writes 0x42 to EEPROM address 0x00
 *   - Reads it back and toggles LED
 *   - Demonstrates update (skip if same)
 *   - Block write/read
 *
 * Assembly verification:
 *   EEPROM_write() -> EEMPE + EEPE timed sequence
 *   EEPROM_read() -> EERE bit set
 */

#include <microavr/microavr.h>

/* Constants */
#define ADDR_TEST      0x00
#define ADDR_BLOCK     0x10
#define ADDR_UPDATED   0x20
#define MAGIC_VALUE    0x42

int main(void) {
    /* Configure LED */
    GPIO_output(PB5);
    GPIO_low(PB5);

    /* Initialize serial for debug output */
    UART_begin(9600);
    UART_println("EEPROM Test");

    /* Read current value at address 0x00 */
    uint8_t val = EEPROM_read(ADDR_TEST);
    UART_print("Initial value: 0x");
    UART_println_hex(val);

    /* Write magic value */
    if (val != MAGIC_VALUE) {
        UART_print("Writing 0x42...");
        EEPROM_write(ADDR_TEST, MAGIC_VALUE);
        UART_println("done");
    }

    /* Read it back */
    val = EEPROM_read(ADDR_TEST);
    UART_print("Read back: 0x");
    UART_println_hex(val);

    /* Toggle LED if correct */
    if (val == MAGIC_VALUE) {
        GPIO_toggle(PB5);
    }

    /* Demonstrate update (skip if same) */
    UART_print("Update with same value (should skip)...");
    EEPROM_update(ADDR_TEST, MAGIC_VALUE);  // No write
    UART_println("done");

    /* Update with different value */
    UART_print("Update with new value (0x43)...");
    EEPROM_update(ADDR_TEST, 0x43);  // Writes
    UART_println("done");

    val = EEPROM_read(ADDR_TEST);
    UART_print("After update: 0x");
    UART_println_hex(val);

    /* Block write */
    uint8_t block_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    UART_print("Block write to 0x10...");
    EEPROM_write_block(ADDR_BLOCK, block_data, sizeof(block_data));
    UART_println("done");

    /* Block read */
    uint8_t block_read[5];
    EEPROM_read_block(ADDR_BLOCK, block_read, sizeof(block_read));
    UART_print("Block read: ");
    for (uint8_t i = 0; i < sizeof(block_read); i++) {
        UART_print("0x");
        UART_print_hex(block_read[i]);
        if (i < sizeof(block_read) - 1) {
            UART_print(" ");
        }
    }
    UART_println("");

    /* Erase all and verify */
    UART_print("Erase all...");
    EEPROM_erase_all();
    UART_println("done");

    val = EEPROM_read(ADDR_TEST);
    UART_print("After erase: 0x");
    UART_println_hex(val);

    /* Restore magic value */
    EEPROM_write(ADDR_TEST, MAGIC_VALUE);

    UART_println("EEPROM test complete");

    while (1) {
        delay_ms(1000);
    }

    return 0;
}