# Using MicroAVR in Your PlatformIO Project

## Quick Start

### Step 1: Get MicroAVR

**Option A: Clone the repository**
```bash
git clone https://github.com/JavEngineering/MicroAVR.git
```

**Option B: Download ZIP from GitHub**

**Option C: Add as git submodule**
```bash
git submodule add https://github.com/JavEngineering/MicroAVR.git lib/MicroAVR
```

### Step 2: Configure platformio.ini

```ini
[env:nano]
platform = atmelavr
board = nanoatmega328new
framework =
build_flags =
    -std=c11
    -Os
    -Wall
    -Ilib/MicroAVR/include
```

### Step 3: Write Code

```c
#include <microavr/microavr.h>

int main(void) {
    GPIO_output(PB5);
    while (1) {
        GPIO_toggle(PB5);
        delay_ms(500);
    }
}
```

---

## Complete Example Project

### Directory Structure

```
my_project/
├── platformio.ini
├── src/
│   └── main.c
└── lib/
    └── MicroAVR/
        ├── include/
        │   └── microavr/
        │       ├── microavr.h
        │       ├── gpio.h
        │       └── ...
        └── ...
```

### platformio.ini

```ini
[env:nano]
platform = atmelavr
board = nanoatmega328new
framework =
board_build.mcu = atmega328p
board_build.f_cpu = 16000000L
build_flags =
    -std=c11
    -Os
    -Wall
    -Wextra
    -Wpedantic
    -ffunction-sections
    -fdata-sections
    -Ilib/MicroAVR/include
upload_port = COM4
upload_speed = 115200
build_unflags = -flto
```

### src/main.c

```c
/**
 * @file main.c
 * @brief MicroAVR blink example
 */

#include <microavr/microavr.h>

int main(void) {
    /* Configure LED pin as output */
    GPIO_output(PB5);

    /* Main loop */
    while (1) {
        GPIO_toggle(PB5);
        delay_ms(500);
    }

    return 0;
}
```

---

## Integration Methods

### Method 1: Git Submodule (Recommended)

1. Add submodule to your project:
```bash
git submodule add https://github.com/JavEngineering/MicroAVR.git lib/MicroAVR
```

2. Add to platformio.ini:
```ini
build_flags = -Ilib/MicroAVR/include
```

3. Initialize submodule (if cloning existing repo):
```bash
git submodule update --init
```

### Method 2: Clone & Reference

1. Clone MicroAVR:
```bash
git clone https://github.com/JavEngineering/MicroAVR.git
```

2. Copy or symlink to your project's `lib/` folder

3. Add to platformio.ini:
```ini
build_flags = -Ilib/MicroAVR/include
```

### Method 3: Copy Headers Only

1. Copy `include/microavr/` folder to your project's `include/` directory

2. Add to platformio.ini:
```ini
build_flags = -Iinclude
```

### Method 4: PlatformIO Registry (Future)

Once published to PlatformIO Registry:

```ini
lib_deps = JavEngineering/MicroAVR
```

---

## Build Flags Reference

### Required Flags

| Flag | Purpose |
|------|---------|
| `-std=c11` | C11 standard required |
| `-Os` | Optimize for size |
| `-I<path>` | Include path to MicroAVR headers |

### Recommended Flags

| Flag | Purpose |
|------|---------|
| `-Wall -Wextra` | Enable warnings |
| `-Wpedantic` | Strict C compliance |
| `-ffunction-sections` | Enable dead code elimination |
| `-fdata-sections` | Enable dead code elimination |

### Example Configuration

```ini
build_flags =
    -std=c11
    -Os
    -Wall
    -Wextra
    -Wpedantic
    -ffunction-sections
    -fdata-sections
    -Ilib/MicroAVR/include
```

---

## Hardware Requirements

- **MCU:** ATmega328P
- **Clock:** 16MHz (Arduino Nano default)
- **Board:** Arduino Nano (new bootloader) or compatible

---

## Troubleshooting

### Issue: "microavr/microavr.h: No such file"

Solution: Check your include path in `build_flags`:
```ini
build_flags = -Ilib/MicroAVR/include
```

### Issue: "undefined reference to `__mulsi3'`

Solution: Add `-lm` to build_flags:
```ini
build_flags = ... -lm
```

### Issue: Warnings about Timer0

This is expected. Timer0 is used by `delay_ms()`. See [docs/timer.md](docs/timer.md) for details.

### Issue: SPI_begin macro takes 0 arguments

Use `SPI_begin()` without arguments (uses defaults: Master, Mode 0, f/4):
```c
SPI_begin();  // Correct
```

Do not pass arguments to the macro:
```c
SPI_begin(SPI_MASTER, SPI_MODE0, SPI_CLK_DIV4);  // Wrong
```

---

## API Header Files

Include the umbrella header for all peripherals:
```c
#include <microavr/microavr.h>
```

Or include specific peripherals:
```c
#include <microavr/gpio.h>
#include <microavr/adc.h>
#include <microavr/uart.h>
// etc.
```

---

## Further Reading

- [README.md](README.md) - Project overview
- [docs/](docs/) - Detailed API documentation
- [examples/](examples/) - Working examples for each peripheral
