# Sleep Modes & Power Management Documentation

## 1. What the Peripheral Does

The ATmega328P supports multiple sleep modes to dramatically reduce power consumption when the MCU is idle. The sleep controller disables the CPU clock and optionally other peripherals while keeping certain wake-up sources active.

**Sleep Modes Available:**
| Mode | SMCR Value | Current @ 16MHz | Wake Sources |
|------|------------|-----------------|--------------|
| **Idle** | 0x00 | ~0.3 mA | Any interrupt |
| **ADC Noise Reduction** | 0x01 | ~0.2 mA | ADC, WDT, TWI, Ext INT |
| **Power-down** | 0x02 | ~0.1 µA | Ext INT, TWI, WDT, Pin Change |
| **Power-save** | 0x03 | ~0.5 µA | Timer2, Ext INT, TWI, WDT, Pin Change |
| **Standby** | 0x06 | ~0.5 µA | Ext INT, WDT, Pin Change |
| **Extended Standby** | 0x07 | ~0.5 µA | Timer2, Ext INT, WDT, Pin Change |

**Key Registers:**
- `SMCR` - Sleep Mode Control Register (SE, SM2:0)
- `PRR` - Power Reduction Register
- `MCUCR` - BODSE, BODS (Brown-out control)
- `WDTCSR` - Watchdog Timer (wake source)

---

## 2. Physical/Electrical Intuition

```
                    ┌─────────────────────┐
                    │      ATmega328P     │
                    │                     │
         Button ──►│ PD2 (INT0)          │──► Wake from Power-down
                    │                     │
         WDT  ────►│ Internal            │──► Periodic wake (~4s)
                    │                     │
         I2C  ◄───►│ PC4/PC5 (TWI)       │──► Wake from Power-down
                    │                     │
         PinChg ──►│ PB0-PD7 (PCINT)     │──► Wake from Power-down
                    │                     │
                    │  Sleep Controller   │
                    │  ┌─────────────┐    │
                    │  │ SMCR (SE)   │    │  ← Sleep Enable
                    │  │ SM2:0       │    │  ← Mode Select
                    │  └─────────────┘    │
                    │  ┌─────────────┐    │
                    │  │ PRR         │    │  ← Power Reduction
                    │  └─────────────┘    │
                    └─────────────────────┘
```

**Current Consumption (Typical @ 16MHz, 5V):**
| Mode | Current | Wakeup Time |
|------|---------|-------------|
| Active | ~10 mA | N/A |
| Idle | ~0.3 mA | 1 cycle |
| ADC NR | ~0.2 mA | 1 cycle |
| Power-down | ~0.1 µA | ~6 cycles |
| Power-save | ~0.5 µA | ~6 cycles |

---

## 3. MicroAVR API

### Sleep Mode API
```c
// Mode selection
Sleep_mode(SLEEP_PWR_DOWN);        // Deepest sleep
Sleep_mode(SLEEP_PWR_SAVE);        // Timer2 keeps running
Sleep_mode(SLEEP_IDLE);            // Lightest sleep

// Enable/disable (auto-manages PRR)
Sleep_enable();                     // Auto-disables unused peripherals via PRR
Sleep_disable();                    // Restores PRR = 0

// Enter sleep
Sleep_cpu();                        // sei(); sleep_cpu(); cli()
                                    // Auto-enables WDT interrupt if WDT enabled

// BOD control (call immediately before Sleep_cpu())
Sleep_bod_disable();                // Disables BOD during sleep
SLEEP_BOD_DISABLE();                // Macro version
```

### Power Reduction API
```c
// Individual control
Power_reduce_adc();
Power_reduce_spi();
Power_reduce_timer0();
Power_reduce_timer1();
Power_reduce_timer2();
Power_reduce_twi();
Power_reduce_uart();

// Enable back
Power_enable_adc();
Power_enable_spi();
// ...

// Bulk control (called automatically by Sleep_enable/disable)
Power_reduce_all_unused();          // Called by Sleep_enable()
Power_restore_all();                // Called by Sleep_disable()
```

### BOD Disable Macro
```c
SLEEP_BOD_DISABLE();                // Call immediately before Sleep_cpu()
```

---

## 4. Simple Example

### Deep Sleep with Button + WDT Wake
```c
#include <microavr/microavr.h>

volatile bool wdt_wake = false;
volatile bool button_wake = false;

void wdt_callback(void) { wdt_wake = true; }
ISR(WDT_vect) { wdt_callback(); }

void int0_callback(void) { button_wake = true; }
INTERRUPT_ISR(INT0_vect, int0_callback);

int main(void) {
    GPIO_output(PB5);              // LED
    GPIO_input(PD2);               // Button
    GPIO_pullup(PD2);

    // Button on INT0
    Interrupt_configure(0, INT_FALLING);
    Interrupt_on(0, INT_FALLING, int0_callback);

    // WDT ~4s interrupt
    WDTCSR = (1<<WDCE)|(1<<WDE);
    WDTCSR = (1<<WDE)|(1<<WDIE)|(1<<WDP3)|(1<<WDP0);  // ~4s

    // Deep sleep config
    Sleep_mode(SLEEP_PWR_DOWN);
    Sleep_enable();

    Interrupt_enable_global();

    while (1) {
        Sleep_cpu();               // Sleeps until button or WDT

        if (wdt_wake) {
            wdt_wake = false;
            GPIO_toggle(PB5);      // Blink on WDT wake
        }
        if (button_wake) {
            button_wake = false;
            GPIO_toggle(PB5);      // Blink on button
        }
    }
}
```

---

## 5. What the AVR Is Actually Doing

### Sleep Enable
```
Sleep_enable();
         │
         ▼
PRR = 0xFF;                        // Disable all peripheral clocks
SMCR |= (1<<SE);                   // Set Sleep Enable bit
```

### Sleep Mode Selection
```
Sleep_mode(SLEEP_PWR_DOWN);
         │
         ▼
SMCR = (SMCR & ~0x07) | 0x02;      // SM2:0 = 010 (Power-down)
```

### BOD Disable Sequence (Timed!)
```
Sleep_bod_disable();
         │
         ▼
MCUCR = (1<<BODS)|(1<<BODSE);      // Must write BODS+BODSE together
MCUCR = (1<<BODS);                 // Then BODS alone within 4 cycles
         │
         ▼
┌─────────────────────────────────────┐
│ BOD disabled during sleep only      │
│ Re-enables automatically on wake    │
└─────────────────────────────────────┘
```

### Entering Sleep
```
Sleep_cpu();
         │
         ▼
sei();                             // Enable global interrupts
sleep_cpu();                       // AVR SLEEP instruction
cli();                             // Disable interrupts on wake
         │
         ▼
┌─────────────────────────────────────┐
│ CPU halted, clocks stopped          │
│ Wake on: INT0, PCINT, TWI, WDT      │
│ Resumes at next instruction         │
└─────────────────────────────────────┘
```

### WDT Auto-Wake (Built into Sleep_cpu)
```
Sleep_cpu() internally:
    if (WDTCSR & (1<<WDE)) {        // If WDT enabled
        WDTCSR |= (1<<WDIE);         // Enable WDT interrupt
    }
```

---

## 6. Register-Level Equivalent

| MicroAVR API | AVR Register Operation |
|--------------|------------------------|
| `Sleep_mode(SLEEP_PWR_DOWN)` | `SMCR = (SMCR & ~0x07) | 0x02;` |
| `Sleep_enable()` | `PRR=0xFF; SMCR|=1<<SE;` |
| `Sleep_disable()` | `SMCR&=~(1<<SE); PRR=0;` |
| `Sleep_cpu()` | `sei(); sleep_cpu(); cli();` |
| `Sleep_bod_disable()` | `MCUCR=(1<<BODS)|(1<<BODSE); MCUCR=(1<<BODS);` |
| `Power_reduce_adc()` | `PRR |= (1<<PRADC);` |
| `Power_enable_adc()` | `PRR &= ~(1<<PRADC);` |

---

## 7. Relevant ATmega328P Registers

| Register | Address | Description |
|----------|---------|-------------|
| `SMCR` | 0x53 | Sleep Mode Control Register |
| `PRR` | 0x64 | Power Reduction Register |
| `MCUCR` | 0x55 | MCU Control Register (BOD) |
| `WDTCSR` | 0x60 | Watchdog Timer Control |

**SMCR Bits:**
| Bit | Name | Description |
|-----|------|-------------|
| 2:0 | SM2:0 | Sleep Mode Select |
| 7 | SE | Sleep Enable |

**PRR Bits:**
| Bit | Name | Description |
|-----|------|-------------|
| 7 | PRTWI | TWI Power Reduction |
| 6 | PRTIM2 | Timer2 Power Reduction |
| 5 | PRTIM0 | Timer0 Power Reduction |
| 4 | PRUSART0 | USART0 Power Reduction |
| 3 | PRTIM1 | Timer1 Power Reduction |
| 2 | PRSPI | SPI Power Reduction |
| 1 | PRADC | ADC Power Reduction |

---

## 8. Performance / Code Size Notes

### Flash Usage (ATmega328P @ 16MHz, -Os)
| Implementation | Flash (bytes) | SRAM (bytes) |
|----------------|---------------|--------------|
| MicroAVR (this lib) | ~156 | 0 |
| Bare AVR C | ~150 | 0 |

### Power Consumption (Typical @ 5V, 16MHz)
| Mode | Current | Wakeup Time |
|------|---------|-------------|
| Active | 10 mA | N/A |
| Idle | 0.3 mA | 1 cycle |
| Power-down | 0.1 µA | ~6 cycles |
| Power-save (Timer2 @ 32kHz) | 0.5 µA | ~6 cycles |

### Assembly (Sleep_cpu)
```asm
; Sleep_cpu()
sei
sleep
cli
```

---

## 9. Common Mistakes

| Mistake | Problem | Fix |
|---------|---------|-----|
| Missing `sei()` before sleep | Never wakes up | `Sleep_cpu()` handles this |
| BOD disable too early | BOD not disabled | Call `Sleep_bod_disable()` immediately before `Sleep_cpu()` |
| Forgetting `Sleep_enable()` | Sleep mode not entered | Call `Sleep_enable()` before `Sleep_cpu()` |
| Missing `sei()` in main | Interrupts never fire | Call `Interrupt_enable_global()` |
| WDT not configured | No periodic wake | Configure `WDTCSR` before sleep |
| PRR not restored | Peripherals dead after wake | `Sleep_disable()` restores PRR=0 |
| BOD disable sequence wrong | BOD not disabled | Must write BODS+BODSE together, then BODS alone |

---

## 10. Bare-Metal Progression

### Step 1: MicroAVR API (Learning)
```c
Sleep_mode(SLEEP_PWR_DOWN);
Sleep_enable();
Sleep_cpu();
```

### Step 2: Understand the Mapping
```
Sleep_enable() → PRR=0xFF, SMCR|=SE
Sleep_mode(PWR_DOWN) → SMCR[2:0]=010
Sleep_cpu() → sei(); sleep; cli()
```

### Step 3: See Register Equivalent
```c
PRR = 0xFF;
SMCR = (1<<SE) | 0x02;
sei(); sleep_cpu(); cli();
```

### Step 4: Modify Registers Directly
```c
PRR |= (1<<PRADC);          // Keep ADC on during sleep
SMCR = (1<<SE) | 0x03;      // Power-save mode
```

### Step 5: Write Bare-Metal AVR C
```c
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

int main(void) {
    // Button on INT0
    DDRD &= ~(1<<2);
    PORTD |= (1<<2);
    EICRA = (1<<ISC01);      // Falling edge
    EIMSK = (1<<INT0);

    // WDT ~4s
    WDTCSR = (1<<WDCE)|(1<<WDE);
    WDTCSR = (1<<WDE)|(1<<WDIE)|(1<<WDP3)|(1<<WDP0);

    // Sleep setup
    PRR = 0xFF;
    SMCR = (1<<SE) | (1<<SM1);  // Power-down

    sei();
    while (1) {
        sleep_cpu();  // Wake on INT0 or WDT
    }
}
```

---

## Assembly Verification (Build Output)

Run `pio run -e sleep_example` and inspect `.pio/build/sleep_example/firmware.elf`:

```asm
; Sleep_enable()
ldi r24, 0xFF
sts PRR, r24
sbi SMCR, 7          ; SE = 1

; Sleep_mode(PWR_DOWN)
in r24, SMCR
andi r24, 0xF8
ori r24, 0x02
sts SMCR, r24

; Sleep_bod_disable()
ldi r24, 0x60
sts MCUCR, r24
ldi r24, 0x40
sts MCUCR, r24

; Sleep_cpu()
sei
sleep
cli
```

**Key observations:**
- All operations inlined, no function calls
- BOD disable sequence uses exact 2-write sequence
- `sleep_cpu()` expands to 3 instructions
- Zero overhead vs bare-metal