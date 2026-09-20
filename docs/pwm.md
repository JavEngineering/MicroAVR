# PWM Peripheral Documentation

## 1. What the Peripheral Does

The PWM (Pulse Width Modulation) peripheral generates analog-like output by rapidly switching a digital pin between HIGH and LOW with a controllable duty cycle. The ATmega328P has 6 hardware PWM channels across 3 timers:

- **Timer0 (8-bit)**: OC0A → PD6 (Arduino D6), OC0B → PD5 (Arduino D5)
- **Timer1 (16-bit)**: OC1A → PB1 (Arduino D9), OC1B → PB2 (Arduino D10)
- **Timer2 (8-bit)**: OC2A → PB3 (Arduino D11), OC2B → PD3 (Arduino D3)

**Only these 6 pins support hardware PWM.** Other GPIO pins cannot generate hardware PWM signals.

---

## 2. Physical/Electrical Intuition

```
                    ┌─────────────────────┐
                    │      ATmega328P     │
                    │                     │
         LED ─────►│ PD6 (OC0A / D6)     │
                    │                     │
                    │ PD5 (OC0B / D5)     │◄──── LED
                    │                     │
                    │ PB1 (OC1A / D9)     │◄──── Motor/Servo
                    │ PB2 (OC1B / D10)    │
                    │                     │
                    │ PB3 (OC2A / D11)    │◄──── LED
                    │ PD3 (OC2B / D3)     │
                    └─────────────────────┘
```

**Duty Cycle**: Percentage of time the signal is HIGH in one period.
- 0% = Always LOW (0V)
- 50% = Square wave (equal HIGH/LOW time)
- 100% = Always HIGH (VCC)

**Frequency**: How many PWM cycles per second (Hz).
- Higher frequency = smoother analog approximation
- Too high = reduced resolution (fewer timer counts per period)
- Typical: 490Hz (Arduino default) to 10kHz+ for motor control

**Resolution**: Number of discrete duty cycle steps.
- 8-bit timers (Timer0, Timer2): 256 steps (0-255)
- 16-bit timer (Timer1): 65536 steps (0-65535)

---

## 3. MicroAVR API

### Concise API (Primary)
```c
// Enable/Disable
PWM_enable(PD6);        // Start PWM on pin
PWM_disable(PD6);       // Stop PWM, pin becomes normal I/O

// Duty cycle (0-255 for 8-bit, 0-65535 for 16-bit)
PWM_duty(PD6, 128);     // 50% duty cycle
PWM_duty(PB1, 32768);   // 50% on 16-bit Timer1

// Frequency (auto-calculates prescaler/TOP)
PWM_frequency(PD6, 1000);  // ~1kHz
PWM_frequency(PB1, 50);    // 50Hz for servos
```

### Fluent API (Optional)
```c
// PERIPHERAL -> TARGET -> ACTION
PWM_pin(PD6).enable();
PWM_pin(PD6).duty(128);
PWM_pin(PD6).frequency(1000);
PWM_pin(PD6).disable();
```

### Valid PWM Pins
```c
PD3  PD5  PD6  PB1  PB2  PB3
 │   │   │   │   │   │
 ▼   ▼   ▼   ▼   ▼   ▼
OC2B OC0B OC0A OC1A OC1B OC2A
(D3) (D5) (D6) (D9) (D10) (D11)
```

### Prescaler Constants
```c
PWM_PS_1      // ÷1
PWM_PS_8      // ÷8
PWM_PS_64     // ÷64 (default)
PWM_PS_256    // ÷256
PWM_PS_1024   // ÷1024
```

---

## 4. Simple Example

### LED Fade (PD6 / OC0A / Timer0 / Arduino D6)
```c
#include <microavr/microavr.h>

int main(void) {
    /* Enable PWM on PD6 (OC0A / Timer0) at ~1kHz */
    PWM_enable(PD6);
    PWM_frequency(PD6, 1000);

    while (1) {
        /* Fade in (0 to 255) */
        for (uint16_t i = 0; i < 255; i++) {
            PWM_duty(PD6, i);
            delay_ms(5);
        }

        /* Fade out (255 to 0) */
        for (uint16_t i = 255; i > 0; i--) {
            PWM_duty(PD6, i);
            delay_ms(5);
        }
    }
}
```

### Servo Control (PB1 / OC1A / Timer1 / Arduino D9)
```c
#include <microavr/microavr.h>

int main(void) {
    /* Timer1 16-bit PWM at 50Hz for servo */
    PWM_enable(PB1);
    PWM_frequency(PB1, 50);  // 50Hz = 20ms period

    while (1) {
        /* 1ms pulse (0°) */
        PWM_duty(PB1, 3277);   // ~5% duty = 1ms @ 20ms
        delay_ms(1000);

        /* 1.5ms pulse (90°) */
        PWM_duty(PB1, 4915);   // ~7.5% duty = 1.5ms
        delay_ms(1000);

        /* 2ms pulse (180°) */
        PWM_duty(PB1, 6554);   // ~10% duty = 2ms
        delay_ms(1000);
    }
}
```

---

## 5. What the AVR Is Actually Doing

### Enable PWM (PD6 / Timer0)
```
PWM_enable(PD6);
         │
         ▼
DDRD |= (1 << DDD6);          // Set PD6 as output
         │
         ▼
TCCR0A = (1<<WGM01)|(1<<WGM00)|(1<<COM0A1);
         │                      │       │
         │                      │       └── Non-inverting PWM on OC0A
         │                      └── Fast PWM mode (TOP=0xFF)
         ▼
TCCR0B = (1<<CS01)|(1<<CS00);  // Prescaler ÷64
```

### Set Duty Cycle
```
PWM_duty(PD6, 128);
         │
         ▼
OCR0A = 128;
         │
         ▼
┌─────────────────────────────────────┐
│ Timer0 counts 0→255→0...            │
│ When TCNT0 == OCR0A: pin goes LOW  │
│ When TCNT0 == 0: pin goes HIGH     │
│ Duty = 128/256 = 50%               │
└─────────────────────────────────────┘
```

### Set Frequency (Auto-Calculation)
```
PWM_frequency(PD6, 1000);
         │
         ▼
f_PWM = F_CPU / (prescaler * (TOP + 1))
      = 16MHz / (64 * 256) = 976 Hz ≈ 1kHz
         │
         ▼
Prescaler = 64 (CS01|CS00), TOP = 255 (fixed for 8-bit)
```

For Timer1 (16-bit), both prescaler and TOP (ICR1) are adjusted:
```
f_PWM = F_CPU / (prescaler * (ICR1 + 1))
→ Adjusts ICR1 and prescaler for closest match
```

---

## 6. Register-Level Equivalent

| MicroAVR API | AVR Register Operation |
|--------------|------------------------|
| `PWM_enable(pin)` | DDRx \|= (1<<bit); TCCRx = Fast PWM config; TCCRx \|= COMx1 |
| `PWM_disable(pin)` | TCCRx &= ~COMx1 |
| `PWM_duty(pin, val)` | OCRx = val |
| `PWM_frequency(pin, hz)` | TCCRx = prescaler; ICR1 = top (Timer1) |

**Manual Bare-Metal Equivalent (PD6 / Timer0):**
```c
/* Init */
DDRD  |= (1 << DDD6);                    // Output
TCCR0A = (1<<WGM01)|(1<<WGM00)|(1<<COM0A1);  // Fast PWM, non-inverting
TCCR0B = (1<<CS01)|(1<<CS00);            // Prescaler ÷64

/* Set duty */
OCR0A = 128;  // 50%
```

---

## 7. Relevant ATmega328P Registers

| Register | Timer | Description |
|----------|-------|-------------|
| `TCCR0A` | 0 | Timer0 Control Register A |
| `TCCR0B` | 0 | Timer0 Control Register B |
| `OCR0A`  | 0 | Timer0 Output Compare A |
| `OCR0B`  | 0 | Timer0 Output Compare B |
| `TCCR1A` | 1 | Timer1 Control Register A |
| `TCCR1B` | 1 | Timer1 Control Register B |
| `OCR1A`  | 1 | Timer1 Output Compare A (16-bit) |
| `OCR1B`  | 1 | Timer1 Output Compare B (16-bit) |
| `ICR1`   | 1 | Timer1 Input Capture / TOP (16-bit) |
| `TCCR2A` | 2 | Timer2 Control Register A |
| `TCCR2B` | 2 | Timer2 Control Register B |
| `OCR2A`  | 2 | Timer2 Output Compare A |
| `OCR2B`  | 2 | Timer2 Output Compare B |

**Key Bits:**
| Bit | Register | Function |
|-----|----------|----------|
| COMx1:0 | TCCRxA | Compare Output Mode (10 = non-inverting PWM) |
| WGMx2:0 | TCCRxA/B | Waveform Generation Mode |
| CSx2:0 | TCCRxB | Clock Select (Prescaler) |

---

## 8. Performance / Code Size Notes

### Flash Usage (ATmega328P @ 16MHz, -Os)
| Implementation | Flash (bytes) | SRAM (bytes) |
|----------------|---------------|--------------|
| MicroAVR (this lib) | ~200-250 | 0 |
| Bare AVR C | ~180-220 | 0 |
| Arduino `analogWrite()` | ~800+ | ~50+ |

### Frequency Accuracy (16MHz, Fast PWM)
| Target | Actual (Timer0/2) | Actual (Timer1) |
|--------|-------------------|-----------------|
| 1 kHz | 976 Hz | 1000 Hz |
| 50 Hz | 61 Hz | 50 Hz |
| 10 kHz | 9.8 kHz | 10 kHz |

*Timer1 achieves exact frequency via ICR1 adjustment; Timer0/2 limited by fixed TOP=255.*

### Assembly (PWM_duty(PD6, 128))
```asm
; OCR0A = 128
ldi r24, 128
sts OCR0A, r24
```

---

## 9. Common Mistakes

| Mistake | Problem | Fix |
|---------|---------|-----|
| Using non-PWM pin (e.g., PB0) | No hardware PWM output | Use only PD3, PD5, PD6, PB1, PB2, PB3 |
| Duty > 255 on 8-bit timer | Value truncated | Use 0-255 for Timer0/2 |
| Expecting exact frequency on Timer0/2 | Fixed TOP=255 limits resolution | Use Timer1 for precise frequencies |
| Forgetting PWM_enable() | Pin stays as GPIO | Call PWM_enable() first |
| Frequency too high for 8-bit | Reduced resolution, glitches | Use Timer1 or lower frequency |

---

## 10. Bare-Metal Progression

### Step 1: MicroAVR API (Learning)
```c
PWM_enable(PD6);
PWM_frequency(PD6, 1000);
PWM_duty(PD6, 128);
```

### Step 2: Understand the Mapping
```
PD6 → OC0A → Timer0 (8-bit)
→ TCCR0A, TCCR0B, OCR0A
→ COM0A1 = PWM output enable
→ WGM01:0 = 011 (Fast PWM)
→ CS01:0 = 011 (Prescaler ÷64)
```

### Step 3: See Register Equivalent
```c
DDRD  |= (1<<DDD6);
TCCR0A = (1<<WGM01)|(1<<WGM00)|(1<<COM0A1);
TCCR0B = (1<<CS01)|(1<<CS00);
OCR0A = 128;
```

### Step 4: Modify Registers Directly
```c
OCR0A = 200;  // Change duty directly
TCCR0B = (TCCR0B & 0xF8) | 0x03;  // Change prescaler
```

### Step 5: Write Bare-Metal AVR C
```c
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    DDRD |= (1<<DDD6);
    TCCR0A = (1<<WGM01)|(1<<WGM00)|(1<<COM0A1);
    TCCR0B = (1<<CS01)|(1<<CS00);
    while (1) {
        for (uint8_t i=0; i<255; i++) {
            OCR0A = i;
            _delay_ms(5);
        }
        for (uint8_t i=255; i>0; i--) {
            OCR0A = i;
            _delay_ms(5);
        }
    }
}
```

---

## Assembly Verification (Build Output)

Run `pio run -e pwm_example` and inspect `.pio/build/pwm_example/firmware.elf`:

```asm
; PWM_enable(PD6)
lds r24, DDRD
ori r24, 0x40
sts DDRD, r24
; TCCR0A = Fast PWM + COM0A1
ldi r24, 0x83
sts TCCR0A, r24
; TCCR0B = prescaler 64
ldi r24, 0x03
sts TCCR0B, r24

; PWM_frequency(PD6, 1000) - sets prescaler to 64
ldi r24, 0x03
sts TCCR0B, r24

; PWM_duty(PD6, 128) - in loop
ldi r24, 128
sts OCR0A, r24
```

**Key observations:**
- All operations inlined, no function calls
- PWM_enable sets DDR, TCCR0A/B in ~4 instructions
- PWM_duty compiles to single `sts OCR0A, r24`
- Timer1 16-bit uses ICR1 for frequency precision