# MicroAVR

Lightweight zero-cost abstraction library for ATmega328P AVR microcontrollers.

## Overview

MicroAVR sits between Arduino (high-level) and bare-metal C (low-level), providing readable, hardware-aware APIs that compile to identical machine code as register manipulation.

**Core philosophy:** *Remember the hardware. Remember the code.*

Every API operation makes it obvious what hardware/peripheral is being accessed and how it maps to AVR registers.

## Features

- **11 Peripherals:** GPIO, ADC, PWM, Timers, UART, SPI, I2C, Interrupts, Sleep, Power, Watchdog, EEPROM
- **Zero runtime overhead:** All functions are `static inline` — compiles to direct register operations
- **Compile-time resolution:** Pin/peripheral configuration resolved at compile time
- **Tiny footprint:** 156 bytes flash, 0 bytes RAM for basic blink example
- **Hardware transparency:** API calls map 1:1 to AVR instructions

## Quick Start

### Requirements

- [PlatformIO](https://platformio.org/) (`pip install platformio`)
- Arduino Nano (ATmega328P @ 16MHz) or compatible board

### Build & Flash

```bash
# Build blink example
pio run -e blink

# Flash to hardware
pio run -e blink -t upload

# Build all examples
pio run
```

### Basic Usage

```c
#include <microavr/microavr.h>

int main(void) {
    GPIO_output(PB5);           // Set PB5 as output (LED pin)

    while (1) {
        GPIO_toggle(PB5);       // Toggle LED
        delay_ms(500);          // Wait 500ms
    }
}
```

## Integration

For detailed instructions on using MicroAVR in your own PlatformIO project,
including complete example configurations and troubleshooting,
see [INTEGRATION.md](INTEGRATION.md).

## API Examples

```c
// GPIO
GPIO_output(PB5);
GPIO_high(PB5);
GPIO_toggle(PB5);

// ADC
uint16_t val = ADC_read(ADC0);

// PWM
PWM_enable(PD6);
PWM_duty(PD6, 128);

// UART
UART_begin(9600);
UART_print("Hello");

// Timers
Timer1_start();
Timer1_compareA(0x3FF);

// SPI
SPI_begin(SPI_MASTER, SPI_MODE0, SPI_DIV16);
uint8_t data = SPI_transfer(0x42);

// I2C
I2C_begin();
I2C_start(0x68);
I2C_write(0x00);

// Watchdog
WDT_enable(WDT_4S, WDT_INTERRUPT);

// EEPROM
EEPROM_write(0x00, 0x42);
uint8_t val = EEPROM_read(0x00);
```

## Project Structure

```
MicroAVR/
├── include/microavr/     # API headers (all code lives here)
│   ├── gpio.h
│   ├── adc.h
│   ├── pwm.h
│   ├── timer.h
│   ├── uart.h
│   ├── spi.h
│   ├── i2c.h
│   ├── interrupt.h
│   ├── sleep.h
│   ├── power.h
│   ├── wdt.h
│   ├── eeprom.h
│   └── microavr.h        # Umbrella header
├── src/                  # Build stubs (empty, for PlatformIO)
├── examples/             # Working examples for each peripheral
│   ├── blink/
│   ├── adc/
│   ├── pwm/
│   ├── timer/
│   ├── uart/
│   ├── spi/
│   ├── i2c/
│   ├── interrupt/
│   ├── sleep/
│   ├── wdt/
│   └── eeprom/
├── benchmarks/           # Arduino vs MicroAVR vs Bare AVR comparisons
├── docs/                 # Detailed API documentation
├── platformio.ini        # Build configuration
└── Makefile              # Alternative build system
```

## Hardware

- **Target:** ATmega328P @ 16MHz
- **Board:** Arduino Nano (new bootloader) or compatible
- **Toolchain:** AVR-GCC via PlatformIO

## Benchmarks

| Example | Flash | RAM |
|---------|-------|-----|
| Blink (MicroAVR) | 156 B | 0 B |
| Blink (Bare AVR) | 156 B | 0 B |
| Blink (Arduino) | 924 B | 0 B |

See `benchmarks/` for detailed comparisons across GPIO, ADC, PWM, Timer, UART, SPI, and I2C.

## Documentation

- [GPIO](docs/gpio.md)
- [ADC](docs/adc.md)
- [PWM](docs/pwm.md)
- [Timers](docs/timer.md)
- [UART](docs/uart.md)
- [SPI](docs/spi.md)
- [I2C](docs/i2c.md)
- [Interrupts](docs/interrupt.md)
- [Sleep](docs/sleep.md)
- [Watchdog](docs/wdt.md)
- [EEPROM](docs/eeprom.md)

## License

MIT License — see [LICENSE](LICENSE) for details.