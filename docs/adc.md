# ADC Peripheral Documentation

## 1. What the Peripheral Does

The ADC (Analog-to-Digital Converter) on the ATmega328P converts analog voltages (0 to VREF) into 10-bit digital values (0-1023). It features:

- **10-bit resolution** (1024 steps)
- **6 external channels** (ADC0-ADC5 on PC0-PC5 / Arduino A0-A5)
- **2 internal channels** (ADC6: temperature sensor, ADC7: 1.1V bandgap)
- **Selectable reference voltage** (AVCC, internal 1.1V, internal 2.56V)
- **Configurable prescaler** (2-128) for ADC clock (target 50-200 kHz)
- **Single-ended and differential** input modes (differential not exposed in Phase 2)
- **Free-running and single-conversion** modes

---

## 2. Physical/Electrical Intuition

```
                    ┌─────────────────────┐
                    │      ATmega328P     │
                    │                     │
         Pot  ─────►│ PC0 (ADC0)          │
         Wiper      │                     │
                    │ PC1 (ADC1)          │◄──── Sensor
                    │ PC2 (ADC2)          │
                    │ PC3 (ADC3)          │
                    │ PC4 (ADC4)          │
                    │ PC5 (ADC5)          │
                    │                     │
                    │ Internal:           │
                    │   ADC6: Temp Sensor │
                    │   ADC7: 1.1V Bandgap│
                    └─────────────────────┘
```

**Voltage Reference**: Determines the maximum input voltage that reads as 1023.
- **AVCC**: VCC (typically 5V or 3.3V) - requires decoupling capacitor on AREF pin
- **Internal 1.1V**: Stable bandgap reference, good for low-voltage sensing
- **Internal 2.56V**: Higher internal reference (availability varies)

**Prescaler**: ADC clock = F_CPU / prescaler. For 16MHz:
| Prescaler | ADC Clock | Conversion Time |
|-----------|-----------|-----------------|
| 2 | 8 MHz | Too fast (reduced accuracy) |
| 128 | 125 kHz | ~104 μs per conversion ✓ |

**Input Impedance**: ~100MΩ, but source impedance should be ≤10kΩ for accurate results.

---

## 3. MicroAVR API

### Concise API (Primary)
```c
// Initialization
ADC_begin();
ADC_end();                    // Disable to save power

// Configuration
ADC_reference(ADC_REF_AVCC);      // AVCC reference
ADC_reference(ADC_REF_INTERNAL_1V1); // 1.1V internal
ADC_reference(ADC_REF_INTERNAL_2V56); // 2.56V internal
ADC_prescaler(ADC_PS_128);        // Prescaler (2,4,8,16,32,64,128)

// Reading
uint16_t value = ADC_read(ADC0);  // 0-1023
```

### Fluent API (Optional)
```c
// PERIPHERAL -> TARGET -> ACTION
ADC_channel(ADC0).read();
```

### Channel Identifiers
```c
ADC0  ADC1  ADC2  ADC3  ADC4  ADC5  ADC6  ADC7
 │    │    │    │    │    │    │    │
 ▼    ▼    ▼    ▼    ▼    ▼    ▼    ▼
PC0  PC1  PC2  PC3  PC4  PC5  Temp 1.1V
(A0) (A1) (A2) (A3) (A4) (A5) Sensor Bandgap
```

### Reference & Prescaler Constants
```c
// Reference
ADC_REF_AVCC
ADC_REF_INTERNAL_1V1
ADC_REF_INTERNAL_2V56

// Prescaler
ADC_PS_2    ADC_PS_4    ADC_PS_8    ADC_PS_16
ADC_PS_32   ADC_PS_64   ADC_PS_128
```

---

## 4. Simple Example

### Potentiometer Reading (A0 / PC0 / ADC0)
```c
#include <microavr/microavr.h>

int main(void) {
    /* Initialize ADC */
    ADC_begin();
    ADC_reference(ADC_REF_AVCC);   // AVCC reference (VCC)
    ADC_prescaler(ADC_PS_128);     // 16MHz/128 = 125kHz ADC clock

    while (1) {
        /* Read potentiometer (0-1023) */
        uint16_t value = ADC_read(ADC0);

        /* Use value: map to PWM, UART output, etc. */
        delay_ms(100);
    }
}
```

### Temperature Sensor (Internal)
```c
#include <microavr/microavr.h>

int main(void) {
    ADC_begin();
    ADC_reference(ADC_REF_INTERNAL_1V1);  // 1.1V ref for temp sensor
    ADC_prescaler(ADC_PS_128);

    while (1) {
        uint16_t raw = ADC_read(ADC6);
        /* Convert to °C: (raw * 1.1 / 1024 - 0.5) * 100 */
        delay_ms(1000);
    }
}
```

---

## 5. What the AVR Is Actually Doing

### Initialization
```
ADC_begin();
         │
         ▼
ADCSRA |= (1 << ADEN);
         │
         ▼
┌─────────────────────────────────────┐
│ ADCSRA (ADC Control & Status Reg A) │
│ Bit 7 (ADEN) = 1 → ADC ENABLED      │
└─────────────────────────────────────┘

ADC_reference(ADC_REF_AVCC);
         │
         ▼
ADMUX = (ADMUX & 0x3F) | 0x40;
         │
         ▼
┌─────────────────────────────────────┐
│ ADMUX (ADC Multiplexer Selection)   │
│ REFS1:0 = 01 → AVCC reference       │
└─────────────────────────────────────┘

ADC_prescaler(ADC_PS_128);
         │
         ▼
ADCSRA = (ADCSRA & 0xF8) | 0x07;
         │
         ▼
┌─────────────────────────────────────┐
│ ADCSRA                              │
│ ADPS2:0 = 111 → ÷128 prescaler      │
│ ADC Clock = 16MHz/128 = 125 kHz     │
└─────────────────────────────────────┘
```

### Single Conversion
```
uint16_t value = ADC_read(ADC0);
                    │
                    ▼
ADMUX = (ADMUX & 0xF0) | 0x00;
                    │
                    ▼
┌─────────────────────────────────────┐
│ ADMUX MUX3:0 = 0000 → ADC0 (PC0)    │
└─────────────────────────────────────┘
                    │
                    ▼
ADCSRA |= (1 << ADSC);
                    │
                    ▼
┌─────────────────────────────────────┐
│ ADCSRA Bit 6 (ADSC) = 1 → START     │
│ Hardware begins conversion          │
└─────────────────────────────────────┘
                    │
                    ▼
while (ADCSRA & (1 << ADSC));
                    │
                    ▼
┌─────────────────────────────────────┐
│ Wait for ADSC to clear (conversion  │
│ complete - auto-cleared by hardware)│
└─────────────────────────────────────┘
                    │
                    ▼
return (ADCH << 8) | ADCL;
                    │
                    ▼
┌─────────────────────────────────────┐
│ Read ADCL FIRST, then ADCH          │
│ (Reading ADCL locks ADCH)           │
│ Result: 10-bit value 0-1023         │
└─────────────────────────────────────┘
```

---

## 6. Register-Level Equivalent

| MicroAVR API | AVR Register Operation |
|--------------|------------------------|
| `ADC_begin()` | `ADCSRA |= (1 << ADEN);` |
| `ADC_end()` | `ADCSRA &= ~(1 << ADEN);` |
| `ADC_reference(ref)` | `ADMUX = (ADMUX & 0x3F) \| ref;` |
| `ADC_prescaler(ps)` | `ADCSRA = (ADCSRA & 0xF8) \| ps;` |
| `ADC_read(ADC0)` | See conversion sequence above |

**Manual Bare-Metal Equivalent:**
```c
/* Init */
ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  // Enable, ÷128
ADMUX  = (1 << REFS0);                                              // AVCC ref

/* Read ADC0 */
ADMUX = (ADMUX & 0xF0) | 0x00;  // Select ADC0
ADCSRA |= (1 << ADSC);          // Start
while (ADCSRA & (1 << ADSC));   // Wait
uint16_t result = ADCL | (ADCH << 8);  // Read (ADCL first!)
```

---

## 7. Relevant ATmega328P Registers

| Register | Address | Description |
|----------|---------|-------------|
| `ADMUX`  | 0x7C (0x27) | ADC Multiplexer Selection |
| `ADCSRA` | 0x7A (0x26) | ADC Control and Status Register A |
| `ADCSRB` | 0x7B (0x28) | ADC Control and Status Register B |
| `ADCL`   | 0x78 (0x24) | ADC Data Register Low Byte |
| `ADCH`   | 0x79 (0x25) | ADC Data Register High Byte |

**ADMUX Bits:**
| Bit | Name | Description |
|-----|------|-------------|
| 7:6 | REFS1:0 | Reference Selection |
| 5   | ADLAR   | Left Adjust Result |
| 3:0 | MUX3:0  | Channel Selection |

**ADCSRA Bits:**
| Bit | Name | Description |
|-----|------|-------------|
| 7   | ADEN  | ADC Enable |
| 6   | ADSC  | Start Conversion |
| 5   | ADATE | Auto Trigger Enable |
| 4   | ADIF  | Interrupt Flag |
| 3   | ADIE  | Interrupt Enable |
| 2:0 | ADPS2:0 | Prescaler Select |

---

## 8. Performance / Code Size Notes

### Flash Usage (ATmega328P @ 16MHz, -Os)
| Implementation | Flash (bytes) | SRAM (bytes) |
|----------------|---------------|--------------|
| MicroAVR (this lib) | ~220 | 0 |
| Bare AVR C | ~200 | 0 |
| Arduino `analogRead()` | ~1000+ | ~100+ |

### Conversion Timing
| Prescaler | ADC Clock | Cycles/Conversion | Time @ 16MHz |
|-----------|-----------|-------------------|--------------|
| 128 | 125 kHz | 13 cycles | ~104 μs |
| 64 | 250 kHz | 13 cycles | ~52 μs |
| 32 | 500 kHz | 13 cycles | ~26 μs |

### Code Generation (ADC_read(ADC0))
```asm
; Select channel ADC0 (MUX=0)
lds  r24, ADMUX
andi r24, 0xF0
sts  ADMUX, r24

; Start conversion
lds  r24, ADCSRA
ori  r24, 0x40
sts  ADCSRA, r24

; Wait for ADSC clear
wait_loop:
lds  r24, ADCSRA
sbrc r24, 6
rjmp wait_loop

; Read result (ADCL first!)
lds  r24, ADCL
lds  r25, ADCH
; Result in r25:r24 (10-bit)
```

---

## 9. Common Mistakes

| Mistake | Problem | Fix |
|---------|---------|-----|
| Forgetting `ADC_begin()` | ADC not enabled, reads return 0 | Call `ADC_begin()` first |
| Wrong prescaler | ADC clock too fast/slow | Use 128 for 16MHz (125kHz) |
| Reading ADCH before ADCL | High byte not locked | Always read ADCL first |
| No decoupling on AREF | Noisy readings with AVCC ref | Add 0.1μF cap on AREF pin |
| Source impedance >10kΩ | Inaccurate readings | Buffer with op-amp |
| Changing channel mid-conversion | Wrong channel sampled | Select channel before ADSC |

---

## 10. Bare-Metal Progression

### Step 1: MicroAVR API (Learning)
```c
ADC_begin();
ADC_reference(ADC_REF_AVCC);
ADC_prescaler(ADC_PS_128);
uint16_t value = ADC_read(ADC0);
```

### Step 2: Understand the Mapping
```
ADC0 → MUX=0x00 → PC0 pin
ADC1 → MUX=0x01 → PC1 pin
...
ADMUX[3:0] = channel
ADMUX[7:6] = reference
ADCSRA[2:0] = prescaler
```

### Step 3: See Register Equivalent
```c
ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
ADMUX  = (1<<REFS0);
```

### Step 4: Modify Registers Directly
```c
ADMUX = (ADMUX & 0xF0) | 0x00;  // Select ADC0
ADCSRA |= (1<<ADSC);            // Start
while (ADCSRA & (1<<ADSC));     // Wait
uint16_t val = ADCL | (ADCH<<8); // Read
```

### Step 5: Write Bare-Metal AVR C
```c
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    ADMUX  = (1<<REFS0);
    while (1) {
        ADMUX = (ADMUX & 0xF0) | 0x00;
        ADCSRA |= (1<<ADSC);
        while (ADCSRA & (1<<ADSC));
        uint16_t val = ADCL | (ADCH<<8);
        _delay_ms(100);
    }
}
```

---

## Assembly Verification (Build Output)

Run `pio run -e adc_example` and inspect `.pio/build/adc_example/firmware.elf`:

```asm
; ADC_begin() - Enable ADC
lds r24, ADCSRA
ori r24, 0x80
sts ADCSRA, r24

; ADC_reference(AVCC) - REFS0=1
lds r24, ADMUX
andi r24, 0x3F
ori r24, 0x40
sts ADMUX, r24

; ADC_prescaler(128) - ADPS2:0=111
lds r24, ADCSRA
andi r24, 0xF8
ori r24, 0x07
sts ADCSRA, r24

; ADC_read(ADC0) - Select channel, start, wait, read
lds r24, ADMUX
andi r24, 0xF0
sts ADMUX, r24
lds r24, ADCSRA
ori r24, 0x40
sts ADCSRA, r24
wait:
lds r24, ADCSRA
sbrc r24, 6
rjmp wait
lds r24, ADCL
lds r25, ADCH
```

**Key observations:**
- All operations inlined, no function calls
- Channel selection, start, wait, read all in main loop
- Efficient register manipulation matching bare-metal code