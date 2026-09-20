# MicroAVR Makefile
# Target: ATmega328P @ 16MHz

MCU = atmega328p
F_CPU = 16000000UL
OPTIMIZATION = -Os

# Toolchain
CC = avr-gcc
CXX = avr-g++
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
AVRDUDE = avrdude

# Directories
INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build
EXAMPLES_DIR = examples

# Flags
CFLAGS = -std=c11 $(OPTIMIZATION) -Wall -Wextra -Wpedantic \
         -ffunction-sections -fdata-sections \
         -mmcu=$(MCU) -DF_CPU=$(F_CPU) \
         -I$(INCLUDE_DIR)

CXXFLAGS = -std=c++17 $(OPTIMIZATION) -Wall -Wextra -Wpedantic \
           -ffunction-sections -fdata-sections \
           -fno-rtti -fno-exceptions \
           -mmcu=$(MCU) -DF_CPU=$(F_CPU) \
           -I$(INCLUDE_DIR)

LDFLAGS = -Wl,--gc-sections -mmcu=$(MCU)

# Examples list
EXAMPLES = blink adc pwm timer uart spi i2c interrupt sleep wdt eeprom

# Default target
all: blink

# Build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Pattern rule for example builds
$(BUILD_DIR)/%.elf: $(EXAMPLES_DIR)/%/$(notdir %).c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$(notdir $<).o
	$(CC) $(LDFLAGS) $(BUILD_DIR)/$(notdir $<).o -o $@
	$(OBJCOPY) -O ihex -R .eeprom $@ $(BUILD_DIR)/$(notdir $*).hex
	$(SIZE) -C --mcu=$(MCU) $@

# Individual example targets
blink: $(BUILD_DIR)/blink.elf
	@echo "Build successful: $(BUILD_DIR)/blink.hex"

adc: $(BUILD_DIR)/adc.elf
	@echo "Build successful: $(BUILD_DIR)/adc.hex"

pwm: $(BUILD_DIR)/pwm.elf
	@echo "Build successful: $(BUILD_DIR)/pwm.hex"

timer: $(BUILD_DIR)/timer.elf
	@echo "Build successful: $(BUILD_DIR)/timer.hex"

uart: $(BUILD_DIR)/uart.elf
	@echo "Build successful: $(BUILD_DIR)/uart.hex"

spi: $(BUILD_DIR)/spi.elf
	@echo "Build successful: $(BUILD_DIR)/spi.hex"

i2c: $(BUILD_DIR)/i2c.elf
	@echo "Build successful: $(BUILD_DIR)/i2c.hex"

interrupt: $(BUILD_DIR)/interrupt.elf
	@echo "Build successful: $(BUILD_DIR)/interrupt.hex"

sleep: $(BUILD_DIR)/sleep.elf
	@echo "Build successful: $(BUILD_DIR)/sleep.hex"

wdt: $(BUILD_DIR)/wdt.elf
	@echo "Build successful: $(BUILD_DIR)/wdt.hex"

eeprom: $(BUILD_DIR)/eeprom.elf
	@echo "Build successful: $(BUILD_DIR)/eeprom.hex"

# Build all examples
examples: $(foreach ex,$(EXAMPLES),$(ex))

# Assembly inspection
asm: blink
	$(OBJDUMP) -S $(BUILD_DIR)/blink.elf > $(BUILD_DIR)/blink.lss
	@echo "Assembly listing: $(BUILD_DIR)/blink.lss"

# Flash to hardware (adjust port as needed)
flash: blink
	$(AVRDUDE) -p $(MCU) -c arduino -P COM4 -b 115200 -U flash:w:$(BUILD_DIR)/blink.hex:i

# Size report
size: blink
	$(SIZE) -C --mcu=$(MCU) $(BUILD_DIR)/blink.elf

# Clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all blink adc pwm timer uart spi i2c interrupt sleep wdt eeprom examples asm flash size clean