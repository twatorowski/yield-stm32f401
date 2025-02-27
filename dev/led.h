/**
 * @file led.h
 *
 * @date 2020-01-18
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief On-Board LED Driver
 */

#ifndef DEV_LED_H_
#define DEV_LED_H_

#include <stdint.h>
#include "dev/gpio_signals.h"
#include "util/bit.h"


/** Led colors */
typedef enum led_colors {
    /** mask for green led */
    LED_RED = BIT_VAL(0),
} led_colors_t;

/**
 * @brief initialize led module
 *
 * @return int status
 */
int Led_Init(void);

/**
 * @brief Enable/disable certain leds
 *
 * @param enable 1 - enable, 0 - disable
 * @param leds led mask
 *
 */
static inline void Led_SetState(int enable, led_colors_t leds)
{
    /* blue led logic */
    if (leds & LED_RED)
        GPIOSig_Set((gpio_signal_t)GPIO_SIGNAL_C11, !enable);
}

/**
 * @brief return the current state of the led
 *
 * @param leds led mask
 * @return int 1 - enabled, 0 - disabled
 */
static inline int Led_GetState(led_colors_t leds)
{
    /* get the state of the pin */
    if (leds & LED_RED)
        return !GPIOSig_Get((gpio_signal_t)GPIO_SIGNAL_C11);

    /* no bit mask set - return 0 by default */
    return 0;
}

#endif /* DEV_LED_H */
