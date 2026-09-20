# UART/USART Peripheral Documentation

## 1. What the Peripheral Does

The ATmega328P contains one Universal Synchronous/Asynchronous Receiver/Transmitter (USART0) that can operate as:

- **UART** (Asynchronous) - Most common: serial communication with PC, GPS, Bluetooth, etc.
- **SPI Master** (Synchronous) - High-speed synchronous communication
- **LIN** - Local Interconnect Network (automotive)

This documentation covers the most common use case: **Asynchronous UART (8N1 default)**.

**Key Features:**
- Full-duplex operation (simultaneous TX/RX)
- Baud rates: 2400 to 1,000,000 bps @ 16MHz
- Configurable data bits (5-9), parity (none/even/odd), stop bits (1/2)
- Double-speed mode (U2X) for higher baud accuracy
- Interrupt-driven or polling operation

---

## 2. Physical/Electrical Intuition

```
                    ┌─────────────────────┐
                    │      ATmega328P     │
                    │                     │
         USB/TTL ──►│ PD0 (RX)            │
                    │                     │
                    │ PD1 (TX) ───► USB/TTL
                    │                     │
                    │   USART0            │
                    │  ┌─────────────┐    │
                    │  │ UBRR0       │    │  Baud rate
                    │  │ UCSR0A/B/C  │    │  Control/Status
                    │  │ UDR0        │    │  Data register
                    │  └─────────────┘    │
                    └─────────────────────┘
```

**Signal Levels:**
- **Idle/Mark**: HIGH (logic 1)
- **Start bit**: LOW (logic 0)
- **Data bits**: LSB first (typically 8 bits)
- **Parity bit**: Optional (even/odd)
- **Stop bit(s)**: HIGH (1 or 2 bits)

**Voltage Levels:** 0V to VCC (typically 5V or 3.3V). Use level shifters for RS-232 (±12V).

---

## 3. MicroAVR API

### Concise API (Primary)
```c
// Initialization (8N1 default)
UART_begin(9600);
UART_begin(9600, UART_8N1);
UART_begin(115200, UART_8E1);   // Even parity
UART_begin(115200, UART_8N2);   // 2 stop bits

// Transmission (blocking)
UART_write('A');                 // Single byte
UART_write_buf(buf, len);        // Buffer
UART_print("Hello");             // Null-terminated string
UART_println("Hello");           // String + \r\n

// Reception
uint8_t c = UART_read();         // Blocking read
bool avail = UART_available();   // Non-blocking check
uint8_t c = UART_read_nb();      // Non-blocking read (0xFF if empty)
UART_flush();                    // Wait for TX complete

// Cleanup
UART_end();                      // Disable RX/TX
```

### Fluent API (Optional)
```c
// PERIPHERAL -> TARGET -> ACTION
UART_baud(9600).begin();
UART.write('A');
UART.print("Hello");
uint8_t c = UART.read();
bool avail = UART.available();
```

### Format Constants
```c
typedef enum {
    UART_8N1 = 0,   // 8 data, no parity, 1 stop (default)
    UART_8N2 = 1,   // 8 data, no parity, 2 stop
    UART_8E1 = 2,   // 8 data, even parity, 1 stop
    UART_8O1 = 3,   // 8 data, odd parity, 1 stop
    UART_9N1 = 4,   // 9 data, no parity, 1 stop
} uart_format_t;
```

---

## 4. Simple Example

### Echo + Print (Blocking)
```c
#include <microavr/microavr.h>

int main(void) {
    UART_begin(9600);           // 9600 baud, 8N1

    UART_println("MicroAVR UART Ready");

    while (1) {
        uint8_t c = UART_read();    // Wait for character
        UART_write(c);              // Echo it back
        if (c == '\r') {
            UART_write('\n');       // Add LF on CR
        }
    }
}
```

### Non-Blocking Polling
```c
#include <microavr/microavr.h>

int main(void) {
    UART_begin(9600);

    while (1) {
        if (UART_available()) {
            uint8_t c = UART_read();
            UART_write(c);
        }
        /* Do other work here */
        GPIO_toggle(PB5);
        delay_ms(10);
    }
}
```

---

## 5. What the AVR Is Actually Doing

### Initialization (9600 baud, 8N1)
```
UART_begin(9600);
         │
         ▼
UBRR0 = (16000000 / (16 * 9600)) - 1 = 103
         │
         ▼
┌─────────────────────────────────────┐
│ UBRR0 = 103 (0x0067)                │
│ Baud = 16MHz / (16 * 104) = 9615 bps│
│ Error = +0.16% (within tolerance)   │
└─────────────────────────────────────┘
         │
         ▼
UCSR0B = (1<<RXEN0) | (1<<TXEN0);
         │
         ▼
┌─────────────────────────────────────┐
│ UCSR0B: RXEN0=1 (RX enable)         │
│         TXEN0=1 (TX enable)         │
└─────────────────────────────────────┘
         │
         ▼
UCSR0C = (3<<UCSZ00);  // 8N1: UPM=00, USBS=0, UCSZ=11
         │
         ▼
┌─────────────────────────────────────┐
│ UCSR0C: UPM01=0, UPM00=0 (no parity)│
│         USBS0=0 (1 stop bit)        │
│         UCSZ01=1, UCSZ00=1 (8 data) │
└─────────────────────────────────────┘
```

### Transmit (UART_write)
```
UART_write('A');
         │
         ▼
while (!(UCSR0A & (1<<UDRE0)));  // Wait for empty
         │
         ▼
┌─────────────────────────────────────┐
│ UDRE0 = 1 when transmit buffer empty│
│ Hardware shifts out: Start + 8 data │
│ + Stop bit(s)                       │
└─────────────────────────────────────┘
         │
         ▼
UDR0 = 'A';  // Load data
         │
         ▼
┌─────────────────────────────────────┐
│ Hardware transmits:                 │
│   Start (0) + 8 data + Stop (1)     │
│   @ 9600 baud = ~1.04ms per byte    │
└─────────────────────────────────────┘
```

### Receive (UART_read)
```
uint8_t c = UART_read();
         │
         ▼
while (!(UCSR0A & (1<<RXC0)));   // Wait for received
         │
         ▼
┌─────────────────────────────────────┐
│ RXC0 = 1 when receive buffer full   │
│ Hardware sampled: Start + 8 data +  │
│ Stop, checked for framing error     │
└─────────────────────────────────────┘
         │
         ▼
return UDR0;  // Read data (clears RXC0)
         │
         ▼
┌─────────────────────────────────────┐
│ Returns 8-bit received data         │
└─────────────────────────────────────┘
```

---

## 6. Register-Level Equivalent

| MicroAVR API | AVR Register Operation |
|--------------|------------------------|
| `UART_begin(9600)` | `UBRR0=103; UCSR0B=(1<<RXEN0)|(1<<TXEN0); UCSR0C=(3<<UCSZ00);` |
| `UART_end()` | `UCSR0B &= ~((1<<RXEN0)|(1<<TXEN0));` |
| `UART_write(c)` | `while(!(UCSR0A&(1<<UDRE0))); UDR0=c;` |
| `UART_read()` | `while(!(UCSR0A&(1<<RXC0))); return UDR0;` |
| `UART_available()` | `return (UCSR0A & (1<<RXC0));` |
| `UART_read_nb()` | `return (UCSR0A&(1<<RXC0)) ? UDR0 : 0xFF;` |
| `UART_flush()` | `while(!(UCSR0A&(1<<TXC0))); UCSR0A|=(1<<TXC0);` |
| `UART_print(s)` | `while(*s) UART_write(*s++);` |

**Manual Bare-Metal Equivalent:**
```c
UBRR0 = 103;                          // 9600 @ 16MHz
UCSR0B = (1<<RXEN0) | (1<<TXEN0);     // Enable RX/TX
UCSR0C = (3<<UCSZ00);                 // 8N1

// Write
while (!(UCSR0A & (1<<UDRE0)));
UDR0 = 'A';

// Read
while (!(UCSR0A & (1<<RXC0)));
char c = UDR0;
```

---

## 7. Relevant ATmega328P Registers

| Register | Address | Description |
|----------|---------|-------------|
| `UDR0`   | 0xC6    | USART I/O Data Register |
| `UCSR0A` | 0xC0    | USART Control and Status A |
| `UCSR0B` | 0xC1    | USART Control and Status B |
| `UCSR0C` | 0xC2    | USART Control and Status C |
| `UBRR0`  | 0xC4/0xC5 | USART Baud Rate Register (16-bit) |

**UCSR0A (Status):**
| Bit | Name | Description |
|-----|------|-------------|
| 7   | RXC0 | USART Receive Complete |
| 6   | TXC0 | USART Transmit Complete |
| 5   | UDRE0| USART Data Register Empty |
| 4   | FE0  | Frame Error |
| 3   | DOR0 | Data OverRun |
| 2   | UPE0 | Parity Error |
| 1   | U2X0 | Double Transmission Speed |
| 0   | MPCM0| Multi-processor Communication Mode |

**UCSR0B (Control):**
| Bit | Name | Description |
|-----|------|-------------|
| 7   | RXCIE0 | RX Complete Interrupt Enable |
| 6   | TXCIE0 | TX Complete Interrupt Enable |
| 5   | UDRIE0 | Data Register Empty Interrupt Enable |
| 4   | RXEN0  | Receiver Enable |
| 3   | TXEN0  | Transmitter Enable |
| 2   | UCSZ02 | Character Size Bit 2 |
| 1   | RXB80  | Receive Data Bit 8 |
| 0   | TXB80  | Transmit Data Bit 8 |

**UCSR0C (Control):**
| Bit | Name | Description |
|-----|------|-------------|
| 7:6 | UMSEL01:0 | USART Mode Select |
| 5:4 | UPM01:0   | Parity Mode |
| 3   | USBS0     | Stop Bit Select |
| 2:1 | UCSZ01:0  | Character Size |
| 0   | UCPOL0    | Clock Polarity (sync mode) |

---

## 8. Performance / Code Size Notes

### Flash Usage (ATmega328P @ 16MHz, -Os)
| Implementation | Flash (bytes) | SRAM (bytes) |
|----------------|---------------|--------------|
| MicroAVR (this lib) | ~156 | 0 |
| Bare AVR C | ~150 | 0 |
| Arduino `Serial` | ~1500+ | ~64+ (ring buffer) |

### Baud Rate Accuracy (16MHz, Normal Mode)
| Baud Rate | UBRR0 | Actual Baud | Error |
|-----------|-------|-------------|-------|
| 2400 | 416 | 2400 | 0.0% |
| 9600 | 103 | 9615 | +0.16% |
| 19200 | 51 | 19231 | +0.16% |
| 38400 | 25 | 38462 | +0.16% |
| 57600 | 16 | 56818 | -1.36% |
| 115200 | 8 | 111111 | -3.55% |
| 115200* | 16 (U2X) | 115385 | +0.16% |

*Use U2X mode (UCSR0A |= (1<<U2X0)) for better high-baud accuracy.*

### Assembly (UART_write)
```asm
; UART_write('A')
wait_udre:
    lds r24, UCSR0A
    sbrs r24, UDRE0
    rjmp wait_udre
ldi r24, 0x41      ; 'A' = 0x41
sts UDR0, r24
```

---

## 9. Common Mistakes

| Mistake | Problem | Fix |
|---------|---------|-----|
| Wrong baud rate | Garbled communication | Use `UART_begin(9600)` with correct `F_CPU` |
| Forgetting `UART_begin()` | No communication | Call `UART_begin()` before any I/O |
| Crossed RX/TX wires | No data received | RX ↔ TX, TX ↔ RX (cross) |
| Wrong voltage levels | No communication / damage | Use 5V/3.3V logic, level shift for RS-232 |
| High baud without U2X | High error rate | Enable U2X: `UCSR0A |= (1<<U2X0);` |
| Blocking read in ISR | Deadlock | Use `UART_read_nb()` or ring buffer |

---

## 10. Bare-Metal Progression

### Step 1: MicroAVR API (Learning)
```c
UART_begin(9600);
UART_println("Hello");
char c = UART_read();
```

### Step 2: Understand the Mapping
```
9600 baud @ 16MHz → UBRR0 = (16000000/(16*9600))-1 = 103
UCSR0B: RXEN=1, TXEN=1
UCSR0C: UCSZ=11 (8-bit), UPM=00 (no parity), USBS=0 (1 stop)
```

### Step 3: See Register Equivalent
```c
UBRR0 = 103;
UCSR0B = (1<<RXEN0)|(1<<TXEN0);
UCSR0C = (3<<UCSZ00);
```

### Step 4: Modify Registers Directly
```c
// Enable double speed for 115200
UCSR0A |= (1<<U2X0);

// 9-bit mode
UCSR0B |= (1<<UCSZ02);
UCSR0C = (7<<UCSZ00);
```

### Step 5: Write Bare-Metal AVR C
```c
#include <avr/io.h>

int main(void) {
    UBRR0 = 103;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    UCSR0C = (3<<UCSZ00);
    while (1) {
        while (!(UCSR0A & (1<<RXC0)));
        char c = UDR0;
        while (!(UCSR0A & (1<<UDRE0)));
        UDR0 = c;
    }
}
```

---

## Assembly Verification (Build Output)

Run `pio run -e uart_example` and inspect `.pio/build/uart_example/firmware.elf`:

```asm
; UART_begin(9600)
ldi r24, 0x67
sts UBRR0, r24
ldi r24, 0x00
sts UBRR0+1, r24
ldi r24, 0x18
sts UCSR0B, r24      ; RXEN|TXEN
ldi r24, 0x06
sts UCSR0C, r24      ; 8N1

; UART_write('A')
wait_udre:
lds r24, UCSR0A
sbrs r24, UDRE0
rjmp wait_udre
ldi r24, 0x41
sts UDR0, r24

; UART_read()
wait_rxc:
lds r24, UCSR0A
sbrs r24, RXC0
rjmp wait_rxc
lds r24, UDR0
```

**Key observations:**
- All operations inlined, no function calls
- Baud rate computed at compile time (constant 103)
- Blocking loops compile to 3-4 instructions each
- Zero overhead vs bare-metal