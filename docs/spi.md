# SPI Peripheral Documentation

## 1. What the Peripheral Does

The SPI (Serial Peripheral Interface) on the ATmega328P is a synchronous serial communication interface for short-distance communication between microcontrollers and peripheral devices (sensors, displays, memory, etc.).

**Key Features:**
- **Master/Slave** operation (Master is primary use case)
- **Full-duplex** communication (simultaneous TX/RX)
- **4 SPI Modes** (CPOL/CPHA combinations)
- **Configurable clock speed** (f_OSC/2 to f_OSC/128)
- **Fixed pin assignments** (cannot be remapped)

**Pin Assignments (Fixed):**
| Pin | Function | Arduino |
|-----|----------|---------|
| PB2 | SS (Slave Select) | D10 |
| PB3 | MOSI (Master Out Slave In) | D11 |
| PB4 | MISO (Master In Slave Out) | D12 |
| PB5 | SCK (Serial Clock) | D13 |

---

## 2. Physical/Electrical Intuition

```
                    ┌─────────────────────┐
                    │      ATmega328P     │
                    │      (Master)       │
                    │                     │
         SS ──────►│ PB2 (D10)           │
                    │                     │
        MOSI ────►│ PB3 (D11)           │
                    │                     │
        MISO ◄────│ PB4 (D12)           │
                    │                     │
        SCK  ────►│ PB5 (D13)           │
                    │                     │
         Device ◄──┘                     │
                    └─────────────────────┘
```

**SPI Modes (CPOL/CPHA):**
| Mode | CPOL | CPHA | Leading Edge | Trailing Edge |
|------|------|------|--------------|---------------|
| 0    | 0    | 0    | Rising       | Falling       |
| 1    | 0    | 1    | Falling      | Rising        |
| 2    | 1    | 0    | Falling      | Rising        |
| 3    | 1    | 1    | Rising       | Falling       |

**Clock Speed:**
- f_OSC/2 = 8 MHz (max)
- f_OSC/4 = 4 MHz
- f_OSC/8 = 2 MHz
- f_OSC/16 = 1 MHz
- f_OSC/32 = 500 kHz
- f_OSC/64 = 250 kHz
- f_OSC/128 = 125 kHz

**Double Speed (SPI2X):** Doubles effective clock (e.g., f_OSC/2 instead of f_OSC/4)

---

## 3. MicroAVR API

### Concise API (Primary)
```c
// Initialization
SPI_begin();                           // Default: Master, Mode 0, f/4
SPI_begin(SPI_MASTER, SPI_MODE0, SPI_CLK_DIV4);

// Chip Select (EXPLICIT - not automatic!)
SPI_select();     // Pull SS LOW
SPI_deselect();   // Pull SS HIGH

// Data Transfer (full-duplex)
uint8_t rx = SPI_transfer(0x55);  // Write 0x55, read response
SPI_write(0x55);                  // Write only (ignore RX)
uint8_t rx = SPI_read();          // Read only (sends 0xFF)

// Cleanup
SPI_end();  // Disable SPI
```

### Fluent API (Optional)
```c
SPI_mode(SPI_MODE0).speed(SPI_CLK_DIV4).begin();
SPI.select().transfer(0x55).deselect();
uint8_t val = SPI.transfer(0x00);
```

### Mode/Speed Constants
```c
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
```

---

## 4. Simple Example

### SPI Device Communication
```c
#include <microavr/microavr.h>

int main(void) {
    /* Initialize SPI: Master, Mode 0, f/4 (4MHz @ 16MHz) */
    SPI_begin(SPI_MASTER, SPI_MODE0, SPI_CLK_DIV4);

    while (1) {
        /* Transaction: CS LOW -> transfer -> CS HIGH */
        SPI_select();
        uint8_t response = SPI_transfer(0x55);  // Send 0x55, get response
        SPI_deselect();

        /* Use response */
        delay_ms(100);
    }
}
```

---

## 5. What the AVR Is Actually Doing

### SPI Initialization (Master, Mode 0, f/4)
```
SPI_begin(SPI_MASTER, SPI_MODE0, SPI_CLK_DIV4);
         │
         ▼
DDRB |= (1<<PB2)|(1<<PB3)|(1<<PB5);  // SS, MOSI, SCK output
DDRB &= ~(1<<PB4);                    // MISO input
PORTB |= (1<<PB2);                    // SS HIGH (inactive)
         │
         ▼
SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
         │              │         │
         │              │         └── SPR0=1, SPR1=0 → ÷4
         │              └── MSTR=1 → Master mode
         └── SPE=1 → SPI Enable
         │
         ▼
SPSR = 0;  // SPI2X=0 (normal speed)
```

### SPI Transfer (Full-Duplex)
```
uint8_t rx = SPI_transfer(0x55);
         │
         ▼
SPDR = 0x55;              // Load transmit data
         │
         ▼
while (!(SPSR & (1<<SPIF)));  // Wait for transfer complete
         │
         ▼
return SPDR;              // Return received byte
         │
         ▼
┌─────────────────────────────────────┐
│ Hardware shifts out 0x55 MSB-first  │
│ Simultaneously shifts in 8 bits     │
│ SPIF flag set when 8 bits done      │
└─────────────────────────────────────┘
```

### Chip Select Control (Explicit!)
```
SPI_select();      // PORTB &= ~(1<<PB2);  // SS LOW
// ... transfer ...
SPI_deselect();    // PORTB |= (1<<PB2);   // SS HIGH
```
**Note:** CS is NOT automatic. You control it explicitly for multi-byte transactions.

---

## 6. Register-Level Equivalent

| MicroAVR API | AVR Register Operation |
|--------------|------------------------|
| `SPI_begin()` | `SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR0); SPSR=0;` |
| `SPI_end()` | `SPCR &= ~(1<<SPE);` |
| `SPI_select()` | `PORTB &= ~(1<<PB2);` |
| `SPI_deselect()` | `PORTB |= (1<<PB2);` |
| `SPI_transfer(d)` | `SPDR=d; while(!(SPSR&(1<<SPIF))); return SPDR;` |
| `SPI_write(d)` | `SPDR=d; while(!(SPSR&(1<<SPIF))); (void)SPDR;` |
| `SPI_read()` | `SPDR=0xFF; while(!(SPSR&(1<<SPIF))); return SPDR;` |

**Manual Bare-Metal Equivalent:**
```c
/* Master, Mode 0, f/4 */
DDRB |= (1<<PB2)|(1<<PB3)|(1<<PB5);  // SS, MOSI, SCK output
DDRB &= ~(1<<PB4);                    // MISO input
PORTB |= (1<<PB2);                    // SS HIGH

SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);  // Enable, Master, f/4
SPSR = 0;

/* Transfer */
PORTB &= ~(1<<PB2);   // CS LOW
SPDR = 0x55;
while (!(SPSR & (1<<SPIF)));
uint8_t rx = SPDR;
PORTB |= (1<<PB2);    // CS HIGH
```

---

## 7. Relevant ATmega328P Registers

| Register | Address | Description |
|----------|---------|-------------|
| `SPCR` | 0x4C | SPI Control Register |
| `SPSR` | 0x4D | SPI Status Register |
| `SPDR` | 0x4E | SPI Data Register |

**SPCR Bits:**
| Bit | Name | Description |
|-----|------|-------------|
| 7 | SPIE | SPI Interrupt Enable |
| 6 | SPE | SPI Enable |
| 5 | DORD | Data Order (0=MSB first) |
| 4 | MSTR | Master/Slave Select |
| 3 | CPOL | Clock Polarity |
| 2 | CPHA | Clock Phase |
| 1:0 | SPR1:0 | Clock Rate Select |

**SPSR Bits:**
| Bit | Name | Description |
|-----|------|-------------|
| 7 | SPIF | SPI Interrupt Flag |
| 6 | WCOL | Write Collision Flag |
| 0 | SPI2X | Double SPI Speed |

---

## 8. Performance / Code Size Notes

### Flash Usage (ATmega328P @ 16MHz, -Os)
| Implementation | Flash (bytes) | SRAM (bytes) |
|----------------|---------------|--------------|
| MicroAVR (this lib) | ~156 | 0 |
| Bare AVR C | ~150 | 0 |
| Arduino `SPI` | ~1000+ | ~20+ |

### Transfer Speed
| Prescaler | Clock | Max Rate |
|-----------|-------|----------|
| DIV2 | 8 MHz | 1 MB/s |
| DIV4 | 4 MHz | 500 kB/s |
| DIV8 | 2 MHz | 250 kB/s |
| DIV16 | 1 MHz | 125 kB/s |
| DIV32 | 500 kHz | 62.5 kB/s |
| DIV64 | 250 kHz | 31.25 kB/s |
| DIV128 | 125 kHz | 15.6 kB/s |

### Assembly (SPI_transfer)
```asm
; SPI_transfer(0x55)
ldi r24, 0x55
sts SPDR, r24
wait_spi:
lds r24, SPSR
sbrs r24, SPIF
rjmp wait_spi
lds r24, SPDR
```

---

## 9. Common Mistakes

| Mistake | Problem | Fix |
|---------|---------|-----|
| Forgetting `SPI_select()` | Device ignores transaction | Call `SPI_select()` before transfer |
| Not `SPI_deselect()` | Bus contention | Always pair select/deselect |
| Wrong SPI mode | Communication failure | Match device datasheet (CPOL/CPHA) |
| Too fast clock | Device can't keep up | Use slower divider (DIV16-DIV128) |
| MISO not input | No data received | `DDRB &= ~(1<<PB4);` for Master |
| SS not HIGH initially | Device always selected | `PORTB |= (1<<PB2);` at init |

---

## 10. Bare-Metal Progression

### Step 1: MicroAVR API (Learning)
```c
SPI_begin(SPI_MASTER, SPI_MODE0, SPI_CLK_DIV4);
SPI_select();
uint8_t rx = SPI_transfer(0x55);
SPI_deselect();
```

### Step 2: Understand the Mapping
```
SPI → Master Mode 0, f/4
→ SPCR: SPE=1, MSTR=1, SPR0=1
→ SPSR: SPI2X=0
→ Pins: PB2=SS(out), PB3=MOSI(out), PB4=MISO(in), PB5=SCK(out)
```

### Step 3: See Register Equivalent
```c
SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
SPDR = 0x55;
while(!(SPSR&(1<<SPIF)));
uint8_t rx = SPDR;
```

### Step 4: Modify Registers Directly
```c
SPCR = (SPCR & 0xF0) | (1<<SPR1);  // Change to f/16
SPSR |= (1<<SPI2X);                // Enable double speed
```

### Step 5: Write Bare-Metal AVR C
```c
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    DDRB |= (1<<2)|(1<<3)|(1<<5);  // SS, MOSI, SCK out
    DDRB &= ~(1<<4);                // MISO in
    PORTB |= (1<<2);                // SS HIGH
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);  // Master, f/4

    while (1) {
        PORTB &= ~(1<<2);       // CS LOW
        SPDR = 0x55;
        while (!(SPSR & (1<<SPIF)));
        uint8_t rx = SPDR;
        PORTB |= (1<<2);        // CS HIGH
        _delay_ms(100);
    }
}
```

---

## Assembly Verification (Build Output)

Run `pio run -e spi_example` and inspect `.pio/build/spi_example/firmware.elf`:

```asm
; SPI_begin()
sbi 0x17, 2      ; DDRB |= (1<<2)  [SS]
sbi 0x17, 3      ; DDRB |= (1<<3)  [MOSI]
sbi 0x17, 5      ; DDRB |= (1<<5)  [SCK]
cbi 0x17, 4      ; DDRB &= ~(1<<4) [MISO]
sbi 0x18, 2      ; PORTB |= (1<<2) [SS HIGH]

ldi r24, 0x53    ; SPCR = SPE|MSTR|SPR0
sts 0x2C, r24

ldi r24, 0x00    ; SPSR = 0
sts 0x2D, r24

; SPI_select()
cbi 0x18, 2      ; PORTB &= ~(1<<2) [SS LOW]

; SPI_transfer(0x55)
ldi r24, 0x55
sts 0x2E, r24    ; SPDR = 0x55
wait:
lds r24, 0x2D
sbrs r24, 7
rjmp wait
lds r24, 0x2E    ; return SPDR

; SPI_deselect()
sbi 0x18, 2      ; PORTB |= (1<<2) [SS HIGH]
```

**Key observations:**
- All operations inlined, no function calls
- Pin directions set in ~5 instructions
- Transfer compiles to ~4 instructions + wait loop
- Zero overhead vs bare-metal

---

## Next Steps

Ready for **Phase 7: I2C/TWI** when you are. The architecture supports:
- `I2C.begin()`, `I2C.end()`
- `I2C.device(addr).write()`, `I2C.device(addr).read()`
- Register mapping: TWBR, TWSR, TWDR, TWCR
- Master mode with 100kHz/400kHz