/**
 * @file benchmark.h
 * @brief Common benchmark utilities.
 *
 * Provides timing macros and result structures for
 * comparing Arduino / MicroAVR / Bare AVR implementations.
 */

#ifndef MICROAVR_BENCHMARK_H
#define MICROAVR_BENCHMARK_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/* Result structure */
typedef struct {
    const char *name;
    uint16_t flash_bytes;
    uint16_t sram_bytes;
    uint32_t cycles_per_iteration;
} benchmark_result_t;

#endif /* MICROAVR_BENCHMARK_H */