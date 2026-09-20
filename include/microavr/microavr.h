/**
 * @file microavr.h
 * @brief Umbrella header - includes all MicroAVR peripherals.
 *
 * Single include for users:
 *   #include <microavr/microavr.h>
 *
 * For finer control, include individual headers:
 *   #include <microavr/gpio.h>
 *   #include <microavr/delay.h>
 *   #include <microavr/adc.h>
 *   #include <microavr/pwm.h>
 *   #include <microavr/timer.h>
 *   #include <microavr/uart.h>
 *   #include <microavr/spi.h>
 *   #include <microavr/i2c.h>
 *   #include <microavr/interrupt.h>
 *   #include <microavr/sleep.h>
 *   #include <microavr/power.h>
 *   #include <microavr/wdt.h>
 *   ...
 */

#ifndef MICROAVR_H
#define MICROAVR_H

#include <microavr/gpio.h>
#include <microavr/delay.h>
#include <microavr/adc.h>
#include <microavr/pwm.h>
#include <microavr/timer.h>
#include <microavr/uart.h>
#include <microavr/spi.h>
#include <microavr/i2c.h>
#include <microavr/interrupt.h>
#include <microavr/sleep.h>
#include <microavr/power.h>
#include <microavr/wdt.h>
#include <microavr/eeprom.h>

/* Future peripherals - uncomment when implemented */
/* #include <microavr/interrupt.h> */

#endif /* MICROAVR_H */