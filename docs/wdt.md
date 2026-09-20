# Watchdog Timer (WDT)

## Overview

The ATmega328P includes an independent on-chip watchdog timer (WDT) that operates from a separate ~128 kHz oscillator. It provides safety against software hangups by generating a system reset or interrupt if not periodically serviced.

## Hardware

| Feature | Details |
|---------|---------|
| Clock | Independent ~128 kHz oscillator |
| Timeouts | 16ms, 32ms, 64ms, 128ms, 256ms, 512ms, 1s, 2s, 4s, 8s |
| Modes | Interrupt, System Reset, Interrupt + Reset |
| Register | WDTCSR (Watchdog Timer Control Register) |
| Config sequence | Timed: WDCE + WDE must be set together, then new config within 4 cycles |

## Quick Start

```c
#include <microavr/microavr.h>

int main(void) {
    GPIO_output(PB5);

    // Enable WDT: 4s timeout, interrupt mode
    WDT_enable(WDT_4S, WDT_INTERRUPT);
    Interrupt_enable_global();

    while (1) {
        GPIO_toggle(PB5);
        WDT_reset();  // "Kick the dog" - must call before timeout
        delay_ms(500);
    }
}

// Auto-generated WDT ISR
void wdt_callback(void) { periodic_task(); }
WDT_ISR(wdt_callback);
```

## API Reference

### Enums

#### `wdt_timeout_t`
| Value | Timeout |
|-------|---------|
| `WDT_16MS` | 16 ms |
| `WDT_32MS` | 32 ms |
| `WDT_64MS` | 64 ms |
| `WDT_128MS` | 128 ms |
| `WDT_256MS` | 256 ms |
| `WDT_512MS` | 512 ms |
| `WDT_1S` | 1 s |
| `WDT_2S` | 2 s |
| `WDT_4S` | 4 s |
| `WDT_8S` | 8 s |

#### `wdt_mode_t`
| Value | Behavior |
|-------|----------|
| `WDT_RESET` | System reset only |
| `WDT_INTERRUPT` | Interrupt only |
| `WDT_INT_RESET` | Interrupt first, then reset on next timeout |

### Functions

#### `WDT_enable(timeout, mode)`
Enable watchdog with specified timeout and mode.

```c
WDT_enable(WDT_4S, WDT_INTERRUPT);
```

**Assembly:** Timed WDTCSR sequence (WDCE + WDE, then config)

#### `WDT_disable()`
Disable watchdog timer.

```c
WDT_disable();
```

**Assembly:** Timed sequence to clear WDE

#### `WDT_reset()`
Reset (kick) the watchdog timer. Must be called before timeout expires.

```c
WDT_reset();  // WDR instruction
```

**Assembly:** `wdr` instruction

#### `WDT_isr_enable()` / `WDT_isr_disable()`
Enable/disable watchdog interrupt (WDIE bit).

```c
WDT_isr_enable();
```

#### `WDT_was_reset()`
Check if last reset was caused by watchdog.

```c
if (WDT_was_reset()) {
    // WDT caused last reset
    WDT_clear_reset_flag();
}
```

#### `WDT_clear_reset_flag()`
Clear the watchdog reset flag (WDRF in MCUSR).

### ISR Macro

#### `WDT_ISR(callback)`
Auto-generate WDT interrupt service routine with user callback.

```c
void my_callback(void) {
    // Handle watchdog interrupt
}

WDT_ISR(my_callback);
```

## Configuration Sequence

The ATmega328P requires a specific timed sequence to modify WDT configuration:

```
Step 1: Set WDCE + WDE simultaneously
        WDTCSR = (1<<WDCE) | (1<<WDE);
Step 2: Write new config within 4 clock cycles
        WDTCSR = (new_config);
```

This is implemented atomically in `WDT_enable()` with interrupts disabled:

```c
static inline void WDT_enable(wdt_timeout_t timeout, wdt_mode_t mode) {
    uint8_t config = _WDT_TIMEOUT_BITS(timeout) | _WDT_MODE_BITS(mode);
    uint8_t _sreg = SREG;
    cli();
    WDTCSR = (1<<WDCE) | (1<<WDE);
    WDTCSR = (config);
    SREG = _sreg;
}
```

## Integration with Sleep (Phase 9)

When using sleep mode with WDT enabled:

```c
// Enable WDT interrupt mode
WDT_enable(WDT_4S, WDT_INTERRUPT);

// Enter idle sleep - will wake on WDT interrupt
Sleep_mode();  // Auto-enables WDIE if WDE=1
```

`Sleep_cpu()` detects if WDT is enabled and automatically enables the watchdog interrupt for wakeup.

## Examples

See `examples/wdt/main.c` for a complete working example.

## Common Patterns

### Periodic Task
```c
WDT_enable(WDT_1S, WDT_INTERRUPT);

while (1) {
    WDT_reset();
    delay_ms(100);
}
```

### Safety Reset Detection
```c
if (WDT_was_reset()) {
    // Recover from WDT reset
    WDT_clear_reset_flag();
    // Initialize peripherals
}
```

### Multiple Timeouts
```c
// Start with long timeout
WDT_enable(WDT_8S, WDT_INT_RESET);

// Reduce timeout during critical operations
WDT_enable(WDT_256MS, WDT_INT_RESET);

// Restore longer timeout
WDT_enable(WDT_4S, WDT_INTERRUPT);
```

## Notes

- WDT oscillator is independent of system clock
- Default timeout after reset is ~16ms with WDE set
- `WDT_disable()` must use the same timed sequence as `WDT_enable()`
- The WDT reset flag (WDRF) survives only one reset unless cleared