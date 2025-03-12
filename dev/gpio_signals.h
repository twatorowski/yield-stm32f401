/**
 * @file gpio_pins.h
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-06-12
 *
 * @brief Pin locations definitions done with gpio.h
 */

#ifndef DEV_GPIO_SIGNALS_H
#define DEV_GPIO_SIGNALS_H

#include "dev/gpio.h"

/** signal location */
typedef struct gpio_pinloc {
    /** gpio port */
    gpio_t *gpio;
    /** pin number */
    gpio_pin_t pin;
} gpio_signal_t;

/** @brief define for the signal location */
#define GPIO_SIGNAL(port, pin_num)              { .gpio = port, .pin = pin_num }

/* signal definitions */
#include "dev/gpio_signals_blackpill.h"
#include "dev/gpio_signals_stm32.h"


/**
 * @brief Configure given pin in gpio as output and set it's initial sta
 *
 * @param gpio gpio signal
 * @param otype output type
 * @param initial_value output initial value
 *
 * @return err_t status
 */
static inline ALWAYS_INLINE err_t GPIOSig_CfgOutput(gpio_signal_t gpio,
    gpio_otype_t otype, int initial_value)
{
    return GPIO_CfgOutput(gpio.gpio, gpio.pin, otype, initial_value);
}

/**
 * @brief configure the output type
 *
 * @param gpio gpio signal definition
 * @param otype output type
 *
 * @return err_t error code
 */
static inline ALWAYS_INLINE err_t GPIOSig_CfgOutputType(gpio_signal_t gpio,
    gpio_otype_t otype)
{
    return GPIO_CfgOutputType(gpio.gpio, gpio.pin, otype);
}

/**
 * @brief configure the output speed
 *
 * @param gpio gpio signal definition
 * @param speed speed
 *
 * @return err_t error code
 */
static inline ALWAYS_INLINE err_t GPIOSig_CfgOutputSpeed(gpio_signal_t gpio,
    gpio_ospeed_t speed)
{
    return GPIO_CfgOutputSpeed(gpio.gpio, gpio.pin, speed);
}

/**
 * @brief Configure given pin as input
 *
 * @param gpio gpio signal
 *
 * @return err_t status code
 */
static inline ALWAYS_INLINE err_t GPIOSig_CfgInput(gpio_signal_t gpio)
{
    return GPIO_CfgInput(gpio.gpio, gpio.pin);
}

/**
 * @brief Setup pull resistor type on given pin
 *
 * @param gpio gpio signal
 * @param pull pull up/dn configuration
 *
 * @return err_t status code
 */
static inline ALWAYS_INLINE err_t GPIOSig_CfgPull(gpio_signal_t gpio,
    gpio_pull_t pull)
{
    return GPIO_CfgPull(gpio.gpio, gpio.pin, pull);
}

/**
 * @brief configure given pin as analog input
 *
 * @param gpio gpio signal
 *
 * @return err_t status code
 */
static inline ALWAYS_INLINE err_t GPIOSig_CfgAnalog(gpio_signal_t gpio)
{
    return GPIO_CfgAnalog(gpio.gpio, gpio.pin);
}

/**
 * @brief configure given pin as alternate function
 *
 * @param gpio gpio signal
 * @param af alternate function
 *
 * @return err_t status code
 */
static inline ALWAYS_INLINE err_t GPIOSig_CfgAltFunction(gpio_signal_t gpio,
    gpio_af_t af)
{
    return GPIO_CfgAltFunction(gpio.gpio, gpio.pin, af);
}

/**
 * @brief Set pin state
 *
 * @param gpio gpio signal
 * @param value pin value
 */
static inline ALWAYS_INLINE void GPIOSig_Set(gpio_signal_t gpio, int value)
{
    GPIO_Set(gpio.gpio, gpio.pin, value);
}

/**
 * @brief Get pin state
 *
 * @param gpio gpio signal
 *
 * @return int pin state
 */
static inline ALWAYS_INLINE int GPIOSig_Get(gpio_signal_t gpio)
{
    return GPIO_Get(gpio.gpio, gpio.pin);
}

#endif /* DEV_GPIO_SIGNALS_H */
