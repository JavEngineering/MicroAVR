# Timer Peripheral Documentation

## 1. What the Peripheral Does

The ATmega328P has three timer/counter peripherals:

| Timer | Bits | Modes | Channels | Special Features |
|-------|------|-------|----------|------------------|
| **Timer0** | 8-bit | Normal, CTC, Fast PWM, Phase-correct | OC0A (PD6), OC0B (PD5) | Used by `delay_ms()` / `millis()` |
| **Timer1** | 16-bit | Normal, CTC, Fast PWM, Phase-correct, Phase/Freq correct | OC1A (PB1), OC1B (PB2) | Input Capture (ICP1), 16-bit precision |
| **Timer2** | 8-bit | Normal, CTC, Fast PWM, Phase-correct | OC2A (PB3), OC2B (PD3) | Async clock option (32.768kHz crystal) |

**Common Uses:**
- Precise timing / system tick (CTC mode)
- PWM generation (Fast PWM, Phase-correct)
- Input capture (Timer1 only)
- Time measurement / frequency counting

---

## 2. Physical/Electrical Intuition

```
                    ┌─────────────────────┐
                    │      ATmega328P     │
                    │                     │
         LED ─────►│ PB5 (LED)           │
                    │                     │
         ┌────────►│ Timer0 (8-bit)      │
         │         │  OC0A → PD6 (D6)    │◄── PWM/Output
         │         │  OC0B → PD5 (D5)    │◄── PWM/Output
         │         │  Used by delay_ms() │
         │         └─────────────────────┘
         │
         ├────────►│ Timer1 (16-bit)     │
         │         │  OC1A → PB1 (D9)    │◄── PWM/Servo
         │         │  OC1B → PB2 (D10)   │◄── PWM/Servo
         │         │  ICP1 → PB0 (D8)    │◄── Input Capture
         │         └─────────────────────┘
         │
         └────────►│ Timer2 (8-bit)      │
                   │  OC2A → PB3 (D11)   │◄── PWM
                   │  OC2B → PD3 (D3)    │◄── PWM
                   │  Async: TOSC1/2     │◄── 32kHz Crystal
                   └─────────────────────┘
```

**CTC Mode (Clear Timer on Compare Match):**
- Timer counts from 0 to OCRxA (TOP)
- When TCNTx == OCRxA: timer resets to 0
- Generates precise frequency: `f = F_CPU / (prescaler * (OCRxA + 1))`

**PWM Modes:** See PWM documentation (Phase 3)

---

## 3. MicroAVR API

### Concise API (Primary)
```c
// Timer control
Timer0_start();
Timer0_stop();
Timer0_reset();
Timer0_mode(TIMER_MODE_CTC);
Timer0_prescaler(TIMER_PS_64);

// Compare match (TOP in CTC mode)
Timer0_compareA(250);
Timer0_compareB(125);

// Interrupts
Timer0_compareAInterrupt();           // Enable interrupt bit
Timer0_compareAInterrupt(my_callback); // Enable + auto-generate ISR
Timer0_overflowInterrupt();

// Timer1 (16-bit) - additional features
Timer1_mode(TIMER_MODE_CTC);
Timer1_compareA(250);           // 16-bit value
Timer1_icr(40000);              // Input Capture Register (TOP in Fast PWM)
Timer1_compareAInterrupt();
Timer1_captureInterrupt();      // Input capture interrupt
```

### Fluent API (Optional)
```c
// PERIPHERAL -> TARGET -> ACTION
Timer0_mode(CTC).prescaler(64).compareA(250).start();
Timer1_compareAInterrupt(my_handler);
```

### ISR Generation Macro
```c
// Define callback
volatile uint32_t ms_ticks = 0;
void timer1_callback(void) { ms_ticks++; }

// Auto-generate ISR
TIMER_ISR(Timer1, TIMER1_COMPA_vect, timer1_callback);

// Expands to:
/*
static void _timer_Timer1_TIMER1_COMPA_vect_isr(void) { timer1_callback(); }
ISR(TIMER1_COMPA_vect) { _timer_Timer1_TIMER1_COMPA_vect_isr(); }
*/
```

### Timer Modes
```c
typedef enum {
    TIMER_MODE_NORMAL        = 0,
    TIMER_MODE_CTC           = 1,
    TIMER_MODE_FAST_PWM      = 2,
    TIMER_MODE_PHASE_CORRECT = 3,
    TIMER_MODE_PHASE_FREQ    = 4,  // Timer1 only
} timer_mode_t;
```

### Prescaler Constants
```c
TIMER_PS_1      // ÷1
TIMER_PS_8      // ÷8
TIMER_PS_64     // ÷64
TIMER_PS_256    // ÷256
TIMER_PS_1024   // ÷1024
```

---

## 4. Simple Example

### 1ms System Tick + LED Blink (Timer1 CTC)
```c
#include <microavr/microavr.h>

volatile uint32_t ms_ticks = 0;

void timer1_callback(void) { ms_ticks++; }
TIMER_ISR(Timer1, TIMER1_COMPA_vect, timer1_callback);

int main(void) {
    GPIO_output(PB5);

    Timer1_mode(TIMER_MODE_CTC);
    Timer1_prescaler(TIMER_PS_64);
    Timer1_compareA(250);        // 16MHz/64/250 = 1000Hz = 1ms
    Timer1_compareAInterrupt();  // Enable interrupt
    Timer1_start();

    sei();  // Enable global interrupts

    while (1) {
        if (ms_ticks >= 1000) {
            ms_ticks = 0;
            GPIO_toggle(PB5);     // Blink every 1 second
        }
    }
}
```

### Precise 50Hz Servo Signal (Timer1 Fast PWM)
```c
#include <microavr/microavr.h>

int main(void) {
    Timer1_mode(TIMER_MODE_FAST_PWM);  // Fast PWM, TOP=ICR1
    Timer1_prescaler(TIMER_PS_64);
    Timer1_icr(40000);                 // 16MHz/64/40001 = 50Hz
    Timer1_compareA(3000);             // 1.5ms pulse (center)
    Timer1_start();
    // PB1 (OC1A) outputs 50Hz PWM
}
```

---

## 5. What the AVR Is Actually Doing

### Timer1 CTC Mode (1ms Tick)
```
Timer1_mode(CTC);
         │
         ▼
TCCR1B |= (1<<WGM12);         // WGM12=1 → CTC mode
         │
         ▼
┌─────────────────────────────────────┐
│ TCCR1B: WGM13=0, WGM12=1, WGM11=0  │
│ WGM13:10 = 0100 → CTC, TOP=OCR1A    │
└─────────────────────────────────────┘

Timer1_prescaler(64);
         │
         ▼
TCCR1B = (TCCR1B & 0xF8) | 0x03;    // CS11=1, CS10=1 → ÷64
         │
         ▼
┌─────────────────────────────────────┐
│ TCCR1B: CS12=0, CS11=1, CS10=1     │
│ Clock = 16MHz / 64 = 250kHz         │
└─────────────────────────────────────┘

Timer1_compareA(250);
         │
         ▼
OCR1A = 250;                    // TOP = 250
         │
         ▼
┌─────────────────────────────────────┐
│ Timer counts 0→250→0...             │
│ Period = 251 * (64/16MHz) = 1ms     │
│ f = 16MHz / (64 * 251) ≈ 1000Hz     │
└─────────────────────────────────────┘

Timer1_compareAInterrupt();
         │
         ▼
TIMSK1 |= (1<<OCIE1A);          // Enable COMPA interrupt
         │
         ▼
┌─────────────────────────────────────┐
│ When TCNT1 == OCR1A:                │
│   - TCNT1 resets to 0               │
│   - OCF1A flag set                  │
│   - If OCIE1A=1: ISR fires          │
└─────────────────────────────────────┘

Timer1_start();
         │
         ▼
TCCR1B |= (1<<CS11)|(1<<CS10);  // Start clock
```

---

## 6. Register-Level Equivalent

| MicroAVR API | AVR Register Operation |
|--------------|------------------------|
| `Timer0_start()` | `TCCR0B \|= CS0 bits` |
| `Timer0_stop()` | `TCCR0B &= ~0x07` |
| `Timer0_reset()` | `TCNT0 = 0` |
| `Timer0_mode(CTC)` | `TCCR0A |= (1<<WGM01)` |
| `Timer0_prescaler(64)` | `TCCR0B = (TCCR0B & 0xF8) \| 0x03` |
| `Timer0_compareA(v)` | `OCR0A = v` |
| `Timer0_compareAInterrupt()` | `TIMSK0 \|= (1<<OCIE0A)` |
| `Timer0_overflowInterrupt()` | `TIMSK0 \|= (1<<TOIE0)` |
| `Timer1_mode(CTC)` | `TCCR1B \|= (1<<WGM12)` |
| `Timer1_prescaler(64)` | `TCCR1B = (TCCR1B & 0xF8) \| 0x03` |
| `Timer1_compareA(v)` | `OCR1A = v` |
| `Timer1_icr(v)` | `ICR1 = v` |
| `Timer1_compareAInterrupt()` | `TIMSK1 \|= (1<<OCIE1A)` |
| `Timer1_captureInterrupt()` | `TIMSK1 \|= (1<<ICIE1)` |

**Manual Bare-Metal Equivalent (Timer1 CTC):**
```c
TCCR1A = 0;
TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);  // CTC, ÷64
OCR1A = 250;                                   // TOP = 250
TIMSK1 = (1<<OCIE1A);                          // Enable interrupt
sei();                                         // Global interrupts

ISR(TIMER1_COMPA_vect) {
    ms_ticks++;
}
```

---

## 7. Relevant ATmega328P Registers

| Register | Timer | Description |
|----------|-------|-------------|
| `TCCR0A`, `TCCR0B` | 0 | Timer0 Control Registers |
| `TCNT0` | 0 | Timer0 Counter |
| `OCR0A`, `OCR0B` | 0 | Timer0 Output Compare |
| `TIMSK0`, `TIFR0` | 0 | Timer0 Interrupt Mask/Flag |
| `TCCR1A`, `TCCR1B`, `TCCR1C` | 1 | Timer1 Control Registers |
| `TCNT1` | 1 | Timer1 Counter (16-bit) |
| `OCR1A`, `OCR1B` | 1 | Timer1 Output Compare (16-bit) |
| `ICR1` | 1 | Timer1 Input Capture / TOP (16-bit) |
| `TIMSK1`, `TIFR1` | 1 | Timer1 Interrupt Mask/Flag |
| `TCCR2A`, `TCCR2B` | 2 | Timer2 Control Registers |
| `TCNT2` | 2 | Timer2 Counter |
| `OCR2A`, `OCR2B` | 2 | Timer2 Output Compare |
| `TIMSK2`, `TIFR2` | 2 | Timer2 Interrupt Mask/Flag |

**Key Bits:**
| Bit | Register | Function |
|-----|----------|----------|
| `WGMx2:0` | TCCRxA/B | Waveform Generation Mode |
| `CSx2:0` | TCCRxB | Clock Select (Prescaler) |
| `COMx1:0` | TCCRxA | Compare Output Mode |
| `OCIEx` | TIMSKx | Output Compare Interrupt Enable |
| `TOIEx` | TIMSKx | Timer Overflow Interrupt Enable |
| `ICIEx` | TIMSK1 | Input Capture Interrupt Enable (Timer1) |

---

## 8. Performance / Code Size Notes

### Flash Usage (ATmega328P @ 16MHz, -Os)
| Implementation | Flash (bytes) | SRAM (bytes) |
|----------------|---------------|--------------|
| MicroAVR (this lib) | ~156 | 0 |
| Bare AVR C | ~150 | 0 |
| Arduino (Timer1 + ISR) | ~800+ | ~50+ |

### CTC Frequency Examples (16MHz)
| Timer | Prescaler | OCRxA | Frequency |
|-------|-----------|-------|-----------|
| Timer0/2 | 64 | 249 | 1 kHz |
| Timer0/2 | 64 | 124 | 2 kHz |
| Timer1 | 64 | 249 | 1 kHz |
| Timer1 | 8 | 1999 | 1 kHz |
| Timer1 | 64 | 24999 | 10 Hz |

### Assembly (Timer1 CTC Setup)
```asm
; Timer1_mode(CTC)
ldi r24, 0x08
sts TCCR1B, r24

; Timer1_prescaler(64)
ldi r24, 0x03
sts TCCR1B, r24

; Timer1_compareA(250)
ldi r24, 0xFA
ldi r25, 0x00
sts OCR1A, r24
sts OCR1A+1, r25

; Timer1_compareAInterrupt()
ldi r24, 0x10
sts TIMSK1, r24

; Timer1_start()
ldi r24, 0x0B
sts TCCR1B, r24

; ISR
ISR(TIMER1_COMPA_vect):
    lds r24, ms_ticks
    lds r25, ms_ticks+1
    lds r26, ms_ticks+2
    lds r27, ms_ticks+3
    adiw r24, 1
    adc r26, r1
    adc r27, r1
    sts ms_ticks, r24
    sts ms_ticks+1, r25
    sts ms_ticks+2, r26
    sts ms_ticks+3, r27
    reti
```

---

## 9. Common Mistakes

| Mistake | Problem | Fix |
|---------|---------|-----|
| Forgetting `sei()` | Interrupts never fire | Call `sei()` after timer setup |
| Wrong prescaler | Wrong frequency | Use `TIMER_PS_64` for 1kHz @ 16MHz |
| CTC: using `compareB` as TOP | `compareA` is TOP in CTC | Use `compareA` for TOP, `compareB` for independent match |
| Timer0 + `delay_ms()` | Wrong delay times | Don't modify Timer0 if using `delay_ms()` |
| Forgetting `sei()` | ISRs never run | Call `sei()` after all init |
| Not declaring callback `static` | ISR optimization issues | Use `static void callback(void)` |

---

## 10. Bare-Metal Progression

### Step 1: MicroAVR API (Learning)
```c
Timer1_mode(TIMER_MODE_CTC);
Timer1_prescaler(TIMER_PS_64);
Timer1_compareA(250);
Timer1_compareAInterrupt();
Timer1_start();
```

### Step 2: Understand the Mapping
```
Timer1 → 16-bit timer/counter
→ TCNT1 counts up
→ OCR1A = TOP in CTC
→ TCCR1B: WGM12=1 (CTC), CS11:0=011 (÷64)
→ TIMSK1: OCIE1A=1 (interrupt enable)
```

### Step 3: See Register Equivalent
```c
TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);
OCR1A = 250;
TIMSK1 = (1<<OCIE1A);
```

### Step 4: Modify Registers Directly
```c
OCR1A = 500;           // Change frequency
TCCR1B = (TCCR1B & 0xF8) | 0x04;  // Change prescaler to 256
```

### Step 5: Write Bare-Metal AVR C
```c
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t ms_ticks = 0;

ISR(TIMER1_COMPA_vect) {
    ms_ticks++;
}

int main(void) {
    DDRB |= (1<<5);           // LED output
    TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);  // CTC, ÷64
    OCR1A = 250;              // 1ms @ 16MHz
    TIMSK1 = (1<<OCIE1A);     // Enable interrupt
    sei();
    while (1) {
        if (ms_ticks >= 1000) {
            ms_ticks = 0;
            PORTB ^= (1<<5);
        }
    }
}
```

---

## Assembly Verification (Build Output)

Run `pio run -e timer_example` and inspect `.pio/build/timer_example/firmware.elf`:

```asm
; Timer1_mode(CTC)
ldi r24, 0x08
sts TCCR1B, r24

; Timer1_prescaler(64)
ldi r24, 0x03
sts TCCR1B, r24

; Timer1_compareA(250)
ldi r24, 0xFA
ldi r25, 0x00
sts OCR1A, r24
sts OCR1A+1, r25

; Timer1_compareAInterrupt()
ldi r24, 0x10
sts TIMSK1, r24

; Timer1_start()
ldi r24, 0x0B
sts TCCR1B, r24

; ISR(TIMER1_COMPA_vect) - auto-generated by TIMER_ISR macro
...
```

**Key observations:**
- All operations inlined, no function calls
- Timer setup in ~5 instructions
- ISR auto-generated by `TIMER_ISR` macro
- Zero overhead vs bare-metal