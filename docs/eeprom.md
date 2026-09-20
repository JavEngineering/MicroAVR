# EEPROM Access

## Overview

The ATmega328P includes 1KB of internal EEPROM (Electrically Erasable Programmable Read-Only Memory) for non-volatile data storage. EEPROM retains data across power cycles and supports byte-level read/write operations.

## Hardware

| Feature | Details |
|---------|---------|
| Size | 1KB (0x0000–0x03FF) |
| Registers | EEARH/L (address), EEDR (data), EECR (control) |
| Write modes | Byte write, Erase+Write |
| Timed sequence | EEMPE must be set, then EEPE within 4 cycles |
| Interrupt | EE_READY_vect (when write completes) |
| Endurance | 100,000 write/erase cycles |

## Quick Start

```c
#include <microavr/microavr.h>

int main(void) {
    UART_begin(9600);

    // Write a byte
    EEPROM_write(0x00, 0x42);

    // Read it back
    uint8_t val = EEPROM_read(0x00);
    UART_print("Value: 0x");
    UART_println_hex(val);

    // Update only if different (saves wear)
    EEPROM_update(0x00, 0x43);

    return 0;
}
```

## API Reference

### Functions

#### `EEPROM_read(addr)`
Read a byte from EEPROM address.

```c
uint8_t val = EEPROM_read(0x00);
```

**Assembly:** EERE bit set, EEDR read

#### `EEPROM_write(addr, data)`
Write a byte to EEPROM address.

```c
EEPROM_write(0x00, 0x42);
```

**Assembly:** Timed sequence EEMPE + EEPE

#### `EEPROM_update(addr, data)`
Update byte only if different (saves EEPROM wear).

```c
EEPROM_update(0x00, 0x43);  // Writes only if different
```

#### `EEPROM_write_block(addr, data, len)`
Write a block of data to EEPROM.

```c
uint8_t buf[] = {0x01, 0x02, 0x03};
EEPROM_write_block(0x10, buf, sizeof(buf));
```

#### `EEPROM_read_block(addr, data, len)`
Read a block of data from EEPROM.

```c
uint8_t buf[3];
EEPROM_read_block(0x10, buf, sizeof(buf));
```

#### `EEPROM_erase_all()`
Erase entire EEPROM (fill with 0xFF).

```c
EEPROM_erase_all();
```

#### `EEPROM_ready()` / `EEPROM_wait()`
Check/wait for write completion.

```c
if (EEPROM_ready()) {
    // Ready for new write
}
EEPROM_wait();  // Block until ready
```

#### `EEPROM_enable_interrupt()` / `EEPROM_disable_interrupt()`
Enable/disable EEPROM ready interrupt.

```c
EEPROM_enable_interrupt();
```

### ISR Macro

#### `EEPROM_ISR(callback)`
Auto-generate EEPROM ready ISR with user callback.

```c
void eeprom_complete(void) {
    // Write complete
}

EEPROM_ISR(eeprom_complete);
```

## Write Sequence

The ATmega328P requires a specific timed sequence to write EEPROM:

```
Step 1: Set EEMPE (master program enable)
        EECR |= (1<<EEMPE);
Step 2: Set EEPE within 4 cycles (starts write)
        EECR |= (1<<EEPE);
```

This is implemented atomically in `EEPROM_write()` with interrupts disabled:

```c
static inline void EEPROM_write(uint16_t addr, uint8_t data) {
    uint8_t _sreg = SREG;
    cli();
    while (!(EECR & (1<<EEPE)));
    EEAR = addr;
    EEDR = data;
    EECR |= (1<<EEMPE);
    EECR |= (1<<EEPE);
    SREG = _sreg;
}
```

## Wear Leveling

EEPROM has limited endurance (100,000 write/erase cycles). Use `EEPROM_update()` to minimize writes:

```c
// Bad: Writes every time (100K cycles)
EEPROM_write(0x00, counter);

// Good: Only writes if value changed
EEPROM_update(0x00, counter);
```

## Examples

See `examples/eeprom/main.c` for a complete working example.

## Common Patterns

### Configuration Storage
```c
// Save configuration
EEPROM_write(0x00, config.mode);
EEPROM_write(0x01, config.speed);

// Load configuration
config.mode = EEPROM_read(0x00);
config.speed = EEPROM_read(0x01);
```

### Data Logging
```c
uint16_t addr = 0;
void log_data(uint8_t value) {
    EEPROM_write(addr++, value);
    if (addr > EEPROM_END) addr = 0;
}
```

### Structure Storage
```c
typedef struct {
    uint16_t magic;
    uint8_t version;
    uint8_t data[10];
} Config;

Config cfg = {0x1234, 1, {0}};
EEPROM_write_block(0x00, (uint8_t*)&cfg, sizeof(cfg));
```

## Notes

- EEPROM address range: 0x0000–0x03FF (1KB)
- Default value after erase: 0xFF
- Write time: ~3.4ms per byte
- Block operations are simple loops (no page write on ATmega328P)
- `EEPROM_update()` compares before writing to reduce wear