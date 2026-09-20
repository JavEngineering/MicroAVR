#!/bin/bash
# run_benchmarks.sh - Build and compare all three blink variants
# Requires: avr-gcc, avr-objdump, avr-size, platformio (optional)

set -e

BUILD_DIR="build/benchmarks"
RESULTS_FILE="benchmarks/results.md"

echo "=== MicroAVR Benchmark Suite ==="
echo "Building three variants..."
echo ""

mkdir -p "$BUILD_DIR/arduino" "$BUILD_DIR/microavr" "$BUILD_DIR/bare_avr"

# Compile function
compile_variant() {
    local name=$1
    local src=$2
    local extra_flags=$3
    local out_dir="$BUILD_DIR/$name"

    echo "Compiling $name..."
    avr-gcc -std=c11 -Os -Wall -Wextra -Wpedantic \
        -ffunction-sections -fdata-sections \
        -mmcu=atmega328p -DF_CPU=16000000UL \
        -Iinclude \
        $extra_flags \
        -c "$src" -o "$out_dir/main.o"

    avr-gcc -Wl,--gc-sections -mmcu=atmega328p \
        "$out_dir/main.o" -o "$out_dir/blink.elf"

    avr-objcopy -O ihex -R .eeprom "$out_dir/blink.elf" "$out_dir/blink.hex"
}

# Extract size
get_size() {
    local elf=$1
    avr-size -C --mcu=atmega328p "$elf" 2>/dev/null | tail -1
}

# Compile all three
compile_variant "arduino" "benchmarks/arduino/blink_arduino.c" ""
compile_variant "microavr" "benchmarks/microavr/blink_microavr.c" ""
compile_variant "bare_avr" "benchmarks/bare_avr/blink_bare.c" ""

# Generate results
echo "=== Benchmark Results ===" > "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"
echo "| Variant | Flash (bytes) | SRAM (bytes) | .text size |" >> "$RESULTS_FILE"
echo "|---------|---------------|--------------|------------|" >> "$RESULTS_FILE"

for variant in arduino microavr bare_avr; do
    elf="$BUILD_DIR/$variant/blink.elf"
    size_output=$(get_size "$elf")
    flash=$(echo "$size_output" | awk '{print $1}')
    sram=$(echo "$size_output" | awk '{print $2}')
    text=$(echo "$size_output" | awk '{print $4}')
    echo "| $variant | $flash | $sram | $text |" >> "$RESULTS_FILE"
done

echo "" >> "$RESULTS_FILE"
echo "=== Assembly Comparison (GPIO toggle) ===" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

for variant in arduino microavr bare_avr; do
    echo "#### $variant" >> "$RESULTS_FILE"
    echo '```asm' >> "$RESULTS_FILE"
    avr-objdump -d "$BUILD_DIR/$variant/blink.elf" | grep -A 20 "<main>" | head -30 >> "$RESULTS_FILE"
    echo '```' >> "$RESULTS_FILE"
    echo "" >> "$RESULTS_FILE"
done

echo "Results written to $RESULTS_FILE"
cat "$RESULTS_FILE"